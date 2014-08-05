/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline.h"
#include "qwt_math.h"

static inline QPointF qwtBezierPoint( const QPointF &p1,
    const QPointF &cp1, const QPointF &cp2, const QPointF &p2, double t )
{
    const double d1 = 3.0 * t;
    const double d2 = 3.0 * t * t;
    const double d3 = t * t * t;
    const double s  = 1.0 - t;

    const double x = (( s * p1.x() + d1 * cp1.x() ) * s + d2 * cp2.x() ) * s + d3 * p2.x();
    const double y = (( s * p1.y() + d1 * cp1.y() ) * s + d2 * cp2.y() ) * s + d3 * p2.y();

    return QPointF( x, y );
}

#if 0

static inline void qwtToCurvatures(
    const QPointF &p1, double m1, const QPointF &p2, double m2,
    double &cv1, double &cv2 )
{
    const double dx = p2.x() - p1.x();
    const double dy = p2.y() - p1.y();

    const double v = 3 * dy / dx - m1 - m2;
    const double k = 2.0 / dx;

    cv1 = k * ( v - m1 );
    cv2 = k * ( m2 - v );
}

#endif

namespace QwtSplineC1P
{
    class PathStore
    {
    public:
        inline void init( int size )
        {
            Q_UNUSED(size);
        }

        inline void start( double x1, double y1 )
        {
            path.moveTo( x1, y1 );
        }

        inline void addCubic( double cx1, double cy1,
            double cx2, double cy2, double x2, double y2 )
        {
            path.cubicTo( cx1, cy1, cx2, cy2, x2, y2 );
        }

        QPainterPath path;
    };

    class ControlPointsStore
    {
    public:
        inline ControlPointsStore():
            d_cp( NULL )
        {
        }

        inline void init( int size )
        {
            controlPoints.resize( size );
            d_cp = controlPoints.data();
        }   

        inline void start( double x1, double y1 )
        {   
            Q_UNUSED( x1 );
            Q_UNUSED( y1 );
        }
        
        inline void addCubic( double cx1, double cy1,
            double cx2, double cy2, double x2, double y2 )
        {
            Q_UNUSED( x2 );
            Q_UNUSED( y2 );

            QLineF &l = *d_cp++;
            l.setLine( cx1, cy1, cx2, cy2 );
        }

        QVector<QLineF> controlPoints;

    private:
        QLineF* d_cp;
    };
}

template< class SplineStore >
static inline SplineStore qwtSplinePathX(
    const QwtSplineC1 *spline, const QPolygonF &points )
{
    SplineStore store;

    const int n = points.size();

    const QVector<double> m = spline->slopesX( points );
    if ( m.size() != n )
        return store;

    const QPointF *pd = points.constData();
    const double *md = m.constData();

    store.init( m.size() - 1 );
    store.start( pd[0].x(), pd[0].y() );

    for ( int i = 0; i < n - 1; i++ )
    {
        const double dx3 = ( pd[i+1].x() - pd[i].x() ) / 3.0;

        store.addCubic( pd[i].x() + dx3, pd[i].y() + md[i] * dx3,
            pd[i+1].x() - dx3, pd[i+1].y() - md[i+1] * dx3,
            pd[i+1].x(), pd[i+1].y() );
    }

    return store;
}

template< class SplineStore, class Param >
static inline SplineStore qwtSplinePathParam( 
    const QwtSplineC1 *spline, const QPolygonF &points, Param param )
{
    const int n = points.size();

    QPolygonF px, py;

    px += QPointF( 0.0, points[0].x() );
    py += QPointF( 0.0, points[0].y() );

    double t = 0.0;
    for ( int i = 1; i < n; i++ )
    {
        t += param( points[i-1], points[i] );

        px += QPointF( t, points[i].x() );
        py += QPointF( t, points[i].y() );
    }

    const QVector<double> mx = spline->slopesX( px );
    const QVector<double> my = spline->slopesX( py );

    SplineStore store;
    store.init( n - 1 );
    store.start( points[0].x(), points[0].y() );

    for ( int i = 1; i < n; i++ )
    {
        const double t3 = param( points[i-1], points[i] ) / 3.0;

        const double cx1 = points[i-1].x() + mx[i-1] * t3;
        const double cy1 = points[i-1].y() + my[i-1] * t3;

        const double cx2 = points[i].x() - mx[i] * t3;
        const double cy2 = points[i].y() - my[i] * t3;

        store.addCubic( cx1, cy1, cx2, cy2, points[i].x(), points[i].y() );
    }

    return store;
}

QwtSplineParameter::QwtSplineParameter( int type ):
    d_type( type )
{
}

QwtSplineParameter::~QwtSplineParameter()
{
}
 
double QwtSplineParameter::value( const QPointF &p1, const QPointF &p2 ) const
{
    switch( d_type )
    {
        case ParameterX:
		{
            return valueX( p1, p2 );
		}
        case ParameterCentripetral:
		{
		    const double dx = p1.x() - p2.x();
    		const double dy = p1.y() - p2.y();

            return ::pow( dx * dx + dy * dy, 0.25 );
		}
        case ParameterChordal:
		{
            return valueChordal( p1, p2 );
		}
        case ParameterManhattan:
		{
            return valueManhattan( p1, p2 );
		}
        case ParameterUniform:
        default:
		{
            return valueUniform( p1, p2 );
		}
    }
}

int QwtSplineParameter::type() const
{
    return d_type;
}

QwtSpline::QwtSpline():
    d_parameter( new QwtSplineParameter( QwtSplineParameter::ParameterX ) )
{
}

QwtSpline::~QwtSpline()
{
    delete d_parameter;
}

void QwtSpline::setParametrization( int type )
{
    delete d_parameter;
    d_parameter = new QwtSplineParameter( type );
}

void QwtSpline::setParametrization( QwtSplineParameter *parameter )
{
    if ( d_parameter != parameter )
    {
        delete d_parameter;
        parameter = d_parameter;
    }
}   

const QwtSplineParameter *QwtSpline::parametrization() const
{
    return d_parameter;
}

QPainterPath QwtSpline::pathP( const QPolygonF &points ) const
{
    const int n = points.size();

    QPainterPath path;
    if ( n == 0 )
        return path;

    if ( n == 1 )
    {
        path.moveTo( points[0] );
        return path;
    }

    if ( n == 2 )
    {
        path.addPolygon( points );
        return path;
    }

    const QVector<QLineF> controlPoints = bezierControlPointsP( points );
    if ( controlPoints.size() == n - 1 )
    {
        const QPointF *p = points.constData();
        const QLineF *l = controlPoints.constData();

        path.moveTo( p[0] );
        for ( int i = 0; i < n - 1; i++ )
            path.cubicTo( l[i].p1(), l[i].p2(), p[i+1] );
    }

    return path;
}

QPolygonF QwtSpline::polygonP( const QPolygonF &points, 
    double delta, bool withNodes ) const
{
    if ( delta <= 0.0 )
        return QPolygonF();

    const int n = points.size();
    if ( n <= 1 )
        return points;

    if ( n == 2 )
    {
        // TODO
        return points;
    }

    QPolygonF path;

    const QVector<QLineF> controlPoints = bezierControlPointsP( points );
    if ( controlPoints.size() != n - 1 )
        return path;

    path += points.first();
    double t = delta;

    for ( int i = 0; i < n - 1; i++ )
    {
#if 1
        const double l = d_parameter->value( points[i], points[i+1] );
#endif

        while ( t < l )
        {
            path += qwtBezierPoint( points[i], controlPoints[i].p1(),
                controlPoints[i].p2(), points[i+1], t / l );

            t += delta;
        }

        if ( withNodes )
        {
            if ( qFuzzyCompare( path.last().x(), points[i+1].x() ) )
                path.last() = points[i+1];
            else
                path += points[i+1];

            t = delta;
        }
        else
        {
            t -= l;
        }
    }

    return path;
}

QwtSplineG1::QwtSplineG1()
{
}

QwtSplineG1::~QwtSplineG1()
{
}

class QwtSplineC1::PrivateData
{
public:
    PrivateData()
    {
        endCondition.type = QwtSplineC1::ParabolicRunout;
        endCondition.value[0] = endCondition.value[1] = 0.0;
    }

    void setEndCondition( QwtSplineC1::EndpointCondition condition, 
		double valueStart, double valueEnd )
    {
        endCondition.type = condition;
        endCondition.value[0] = valueStart;
        endCondition.value[1] = valueEnd;
    }

    struct
    {
        QwtSplineC1::EndpointCondition type;
        double value[2];

    } endCondition;
};

QwtSplineC1::QwtSplineC1()
{
	d_data = new PrivateData;
}

QwtSplineC1::~QwtSplineC1()
{
	delete d_data;
}

void QwtSplineC1::setEndConditions( EndpointCondition condition )
{
	if ( condition >= Natural )
    	d_data->setEndCondition( condition, 0.0, 0.0 );
	else
    	d_data->setEndCondition( condition, clampedBegin(), clampedEnd()  );
}

QwtSplineC1::EndpointCondition QwtSplineC1::endCondition() const
{
	return d_data->endCondition.type;
}

void QwtSplineC1::setClamped( double valueBegin, double valueEnd )
{
    d_data->setEndCondition( endCondition(), valueBegin, valueEnd );
}

double QwtSplineC1::clampedBegin() const
{
	return d_data->endCondition.value[0];
}

double QwtSplineC1::clampedEnd() const
{
	return d_data->endCondition.value[1];
}

QPainterPath QwtSplineC1::pathP( const QPolygonF &points ) const
{
    const int n = points.size();
    if ( n <= 2 )
        return QwtSpline::pathP( points );

    using namespace QwtSplineC1P;

    PathStore store;
    switch( parametrization()->type() )
    {
        case QwtSplineParameter::ParameterX:
        {
            store = qwtSplinePathX<PathStore>( this, points );
            break;
        }
        case QwtSplineParameter::ParameterUniform:
        {
            store = qwtSplinePathParam<PathStore>( this, points, 
                QwtSplineParameter::paramUniform() );
            break;
        }
        case QwtSplineParameter::ParameterChordal:
        {
            store = qwtSplinePathParam<PathStore>( this, points, 
                QwtSplineParameter::paramChordal() );
            break;
        }
        default:
        {
            store = qwtSplinePathParam<PathStore>( this, points, 
                QwtSplineParameter::param( parametrization() ) );
        }
    }

    return store.path;
}

QVector<QLineF> QwtSplineC1::bezierControlPointsP( const QPolygonF &points ) const
{
    using namespace QwtSplineC1P;

    const int n = points.size();
    if ( n <= 2 )
        return QVector<QLineF>();

    ControlPointsStore store;
    switch( parametrization()->type() )
    {
        case QwtSplineParameter::ParameterX:
        {
            store = qwtSplinePathX<ControlPointsStore>( this, points );
            break;
        }
        case QwtSplineParameter::ParameterUniform:
        {
            store = qwtSplinePathParam<ControlPointsStore>( this, points,
                QwtSplineParameter::paramUniform() );
            break;
        }
        case QwtSplineParameter::ParameterChordal:
        {
            store = qwtSplinePathParam<ControlPointsStore>( this, points,
                QwtSplineParameter::paramChordal() );
            break;
        }
        default:
        {
            store = qwtSplinePathParam<ControlPointsStore>( this, points,
                QwtSplineParameter::param( parametrization() ) );
        }
    }

    return store.controlPoints;
}

QPolygonF QwtSplineC1::polygonX( int numPoints, const QPolygonF &points ) const
{
    if ( points.size() <= 2 )
        return points;

    QPolygonF fittedPoints;

    const QVector<double> m = slopesX( points );
    if ( m.size() != points.size() )
        return fittedPoints;

    const QPointF *p = points.constData();
    const double *s = m.constData();

    const double x1 = points.first().x();
    const double x2 = points.last().x();

    const double delta = ( x2 - x1 ) / ( numPoints - 1 );

    double x0, y0;
    QwtSplinePolynom polynom;

    for ( int i = 0, j = 0; i < numPoints; i++ )
    {
        double x = x1 + i * delta;
        if ( x > x2 )
            x = x2;

        if ( i == 0 || x > p[j + 1].x() )
        {
            while ( x > p[j + 1].x() )
                j++;

            polynom = QwtSplinePolynom::fromSlopes( p[j], s[j], p[j + 1], s[j + 1] );

            x0 = p[j].x();
            y0 = p[j].y();
        }

        const double y = y0 + polynom.value( x - x0 );
        fittedPoints += QPointF( x, y );
    }

    return fittedPoints;
}   

QVector<QwtSplinePolynom> QwtSplineC1::polynomsX( const QPolygonF &points ) const
{
    QVector<QwtSplinePolynom> polynoms;

    const QVector<double> m = slopesX( points );
    if ( m.size() < 2 )
        return polynoms;

    for ( int i = 1; i < m.size(); i++ )
    {
        polynoms += QwtSplinePolynom::fromSlopes( 
            points[i-1], m[i-1], points[i], m[i] );
    }

    return polynoms;
}

QwtSplineC2::QwtSplineC2()
{
}

QwtSplineC2::~QwtSplineC2()
{
}
