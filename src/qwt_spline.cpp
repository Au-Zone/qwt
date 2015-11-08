/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline.h"
#include "qwt_spline_parametrization.h"
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

namespace QwtSplineC1P
{
    struct param
    {
        param( const QwtSplineParametrization *p ):
            parameter( p )
        {
        }

        inline double operator()( const QPointF &p1, const QPointF &p2 ) const
        {
            return parameter->valueIncrement( p1, p2 );
        }

        const QwtSplineParametrization *parameter;
    };

    struct paramY
    {
        inline double operator()( const QPointF &p1, const QPointF &p2 ) const
        {
            return QwtSplineParametrization::valueIncrementY( p1, p2 );
        }
    };

    struct paramUniform
    {
        inline double operator()( const QPointF &p1, const QPointF &p2 ) const
        {
            return QwtSplineParametrization::valueIncrementUniform( p1, p2 );
        }
    };

    struct paramCentripetal
    {
        inline double operator()( const QPointF &p1, const QPointF &p2 ) const
        {
            return QwtSplineParametrization::valueIncrementCentripetal( p1, p2 );
        }
    };

    struct paramChordal
    {
        inline double operator()( const QPointF &p1, const QPointF &p2 ) const
        {
            return QwtSplineParametrization::valueIncrementChordal( p1, p2 );
        }
    };

    struct paramManhattan
    {
        inline double operator()( const QPointF &p1, const QPointF &p2 ) const
        {
            return QwtSplineParametrization::valueIncrementManhattan( p1, p2 );
        }
    };

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

        inline void end()
        {
            path.closeSubpath();
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

        inline void end()
        {
        } 

        QVector<QLineF> controlPoints;

    private:
        QLineF* d_cp;
    };
}

template< class SplineStore >
static inline SplineStore qwtSplineC1PathParamX(
    const QwtSplineC1 *spline, const QPolygonF &points )
{
    const int n = points.size();

    const QVector<double> m = spline->slopes( points );
    if ( m.size() != n )
        return SplineStore();

    const QPointF *pd = points.constData();
    const double *md = m.constData();

    SplineStore store;
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

template< class SplineStore >
static inline SplineStore qwtSplineC1PathParamY(
    const QwtSplineC1 *spline, const QPolygonF &points )
{
    const int n = points.size();

    QPolygonF pointsFlipped( n );
    for ( int i = 0; i < n; i++ )
    {
        pointsFlipped[i].setX( points[i].y() );
        pointsFlipped[i].setY( points[i].x() );
    }

    const QVector<double> m = spline->slopes( pointsFlipped );
    if ( m.size() != n )
        return SplineStore();

    const QPointF *pd = pointsFlipped.constData();
    const double *md = m.constData();

    SplineStore store;
    store.init( m.size() - 1 );
    store.start( pd[0].y(), pd[0].x() );

    QVector<QLineF> lines( n );
    for ( int i = 0; i < n - 1; i++ )
    {
        const double dx3 = ( pd[i+1].x() - pd[i].x() ) / 3.0;

        store.addCubic( pd[i].y() + md[i] * dx3, pd[i].x() + dx3, 
            pd[i+1].y() - md[i+1] * dx3, pd[i+1].x() - dx3, 
            pd[i+1].y(), pd[i+1].x() );
    }

    return store;
}

template< class SplineStore, class Param >
static inline SplineStore qwtSplineC1PathParametric( 
    const QwtSplineC1 *spline, const QPolygonF &points, Param param )
{
    const bool isClosing = ( spline->boundaryType() == QwtSpline::ClosedPolygon );
    const int n = points.size();

    QPolygonF pointsX, pointsY;
    pointsX.resize( isClosing ? n + 1 : n );
    pointsY.resize( isClosing ? n + 1 : n );

    QPointF *px = pointsX.data();
    QPointF *py = pointsY.data();
    const QPointF *p = points.constData();

    double t = 0.0;

    px[0].rx() = py[0].rx() = t;
    px[0].ry() = p[0].x();
    py[0].ry() = p[0].y();

    for ( int i = 1; i < n; i++ )
    {
        t += param( points[i-1], points[i] );

        px[i].rx() = py[i].rx() = t;
        px[i].ry() = p[i].x();
        py[i].ry() = p[i].y();
    }

    if ( isClosing )
    {
        t += param( points[n-1], points[0] );

        px[n].rx() = py[n].rx() = t;
        px[n].ry() = p[0].x();
        py[n].ry() = p[0].y();
    }

    const QVector<double> mx = spline->slopes( pointsX );
    const QVector<double> my = spline->slopes( pointsY );

    pointsY.clear(); // we don't need it anymore

    SplineStore store;
    store.init( isClosing ? n : n - 1 );
    store.start( points[0].x(), points[0].y() );

    for ( int i = 0; i < n - 1; i++ )
    {
#if 0
        const double t3 = param( points[i], points[i+1] ) / 3.0;
#else
        const double t3 = ( px[i+1].x() - px[i].x() ) / 3.0;
#endif

        const double cx1 = points[i].x() + mx[i] * t3;
        const double cy1 = points[i].y() + my[i] * t3;

        const double cx2 = points[i+1].x() - mx[i+1] * t3;
        const double cy2 = points[i+1].y() - my[i+1] * t3;

        store.addCubic( cx1, cy1, cx2, cy2, points[i+1].x(), points[i+1].y() );
    }

    if ( isClosing )
    {
        const double t3 = param( points[n-1], points[0] ) / 3.0;

        const double cx1 = points[n-1].x() + mx[n-1] * t3;
        const double cy1 = points[n-1].y() + my[n-1] * t3;

        const double cx2 = points[0].x() - mx[0] * t3;
        const double cy2 = points[0].y() - my[0] * t3;

        store.addCubic( cx1, cy1, cx2, cy2, points[0].x(), points[0].y() );
        store.end();
    }

    return store;
}

template< QwtSplinePolynomial toPolynomial( const QPointF &, double, const QPointF &, double ) >
static QPolygonF qwtPolygonParametric( double distance,
    const QPolygonF &points, const QVector<double> values, bool withNodes ) 
{
    QPolygonF fittedPoints;

    const QPointF *p = points.constData();
    const double *v = values.constData();
    
    fittedPoints += p[0];
    double t = distance;
    
    const int n = points.size();

    for ( int i = 0; i < n - 1; i++ )
    {
        const QPointF &p1 = p[i];
        const QPointF &p2 = p[i+1];

        const QwtSplinePolynomial polynomial = toPolynomial( p1, v[i], p2, v[i+1] );
            
        const double l = p2.x() - p1.x();
        
        while ( t < l )
        {
            fittedPoints += QPointF( p1.x() + t, p1.y() + polynomial.valueAt( t ) );
            t += distance;
        }   
        
        if ( withNodes )
        {
            if ( qFuzzyCompare( fittedPoints.last().x(), p2.x() ) )
                fittedPoints.last() = p2;
            else
                fittedPoints += p2;
        }       
        else
        {
            t -= l;
        }   
    }   
    
    return fittedPoints;
}

class QwtSpline::PrivateData
{
public:
    PrivateData():
        boundaryType( QwtSpline::ConditionalBoundaries )
    {
        parametrization = new QwtSplineParametrization( 
            QwtSplineParametrization::ParameterChordal );

        // parabolic runout at both ends

        boundaryConditions[0].type = QwtSpline::Clamped3;
        boundaryConditions[0].value = 0.0;

        boundaryConditions[1].type = QwtSpline::Clamped3;
        boundaryConditions[1].value = 0.0;
    }

    ~PrivateData()
    {
        delete parametrization;
    }

    QwtSplineParametrization *parametrization;
    QwtSpline::BoundaryType boundaryType;

    struct
    {
        QwtSpline::BoundaryCondition type;
        double value;

    } boundaryConditions[2];
};

/*!
  \brief Constructor

  The default setting is a non closing spline with chordal parametrization

  \sa setParametrization(), setClosing()
 */
QwtSpline::QwtSpline()
{
    d_data = new PrivateData;
}

//! Destructor
QwtSpline::~QwtSpline()
{
    delete d_data;
}

/*!
  The locality of an spline interpolation identifies how many adjacent
  polynoms are affected, when changing the position of one point.

  A locality of 'n' means, that changing the coordinates of a point
  has an effect on 'n' leading and 'n' following polynoms.
  Those polynoms can be calculated from a local subpolygon.

  A value of 0 means, that the interpolation is not local and any modification
  of the polygon requires to recalculate all polynoms ( f.e cubic splines ). 

  \return Order of locality
 */
uint QwtSpline::locality() const
{
    return 0;
}

void QwtSpline::setParametrization( int type )
{
    if ( d_data->parametrization->type() != type )
    {
        delete d_data->parametrization;
        d_data->parametrization = new QwtSplineParametrization( type );
    }
}

void QwtSpline::setParametrization( QwtSplineParametrization *parametrization )
{
    if ( ( parametrization != NULL ) && ( d_data->parametrization != parametrization ) )
    {
        delete d_data->parametrization;
        d_data->parametrization = parametrization;
    }
}   

const QwtSplineParametrization *QwtSpline::parametrization() const
{
    return d_data->parametrization;
}

void QwtSpline::setBoundaryType( BoundaryType boundaryType )
{
    d_data->boundaryType = boundaryType;;
}

QwtSpline::BoundaryType QwtSpline::boundaryType() const
{
    return d_data->boundaryType;
}

void QwtSpline::setBoundaryCondition( BoundaryPosition position,
    BoundaryCondition condition )
{
    if ( ( position == QwtSpline::AtBeginning ) || ( position == QwtSpline::AtEnd ) )
        d_data->boundaryConditions[position].type = condition;
}

QwtSpline::BoundaryCondition QwtSpline::boundaryCondition(
    BoundaryPosition position) const
{
    if ( ( position == QwtSpline::AtBeginning ) || ( position == QwtSpline::AtEnd ) )
        return d_data->boundaryConditions[position].type;

    return d_data->boundaryConditions[0].type; // should never happen
}

void QwtSpline::setBoundaryValue( BoundaryPosition position, double value )
{
    if ( ( position == QwtSpline::AtBeginning ) || ( position == QwtSpline::AtEnd ) )
        d_data->boundaryConditions[position].value = value;
}

double QwtSpline::boundaryValue( BoundaryPosition position ) const
{
    if ( ( position == QwtSpline::AtBeginning ) || ( position == QwtSpline::AtEnd ) )
        return d_data->boundaryConditions[position].value;

    return d_data->boundaryConditions[0].value; // should never happen
}

/*! \fn QVector<QLineF> bezierControlLines( const QPolygonF &points ) const

  \brief Interpolate a curve with Bezier curves

  Interpolates a polygon piecewise with cubic Bezier curves
  and returns the 2 control points of each curve as QLineF.

  \param points Control points
  \return Control points of the interpolating Bezier curves
 */

/*!
  \brief Interpolate a curve with Bezier curves

  Interpolates a polygon piecewise with cubic Bezier curves
  and returns them as QPainterPath.

  The implementation calculates the Bezier control lines first
  and converts them into painter path elements in an additional loop.
  
  \param points Control points
  \return Painter path, that can be rendered by QPainter

  \note Derived spline classes might overload painterPath() to avoid
        the extra loops for converting results into a QPainterPath

  \sa bezierControlLines()
 */
QPainterPath QwtSpline::painterPath( const QPolygonF &points ) const
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

    const QVector<QLineF> controlLines = bezierControlLines( points );
    if ( controlLines.size() < n - 1 )
        return path;

    const QPointF *p = points.constData();
    const QLineF *l = controlLines.constData();

    path.moveTo( p[0] );
    for ( int i = 0; i < n - 1; i++ )
        path.cubicTo( l[i].p1(), l[i].p2(), p[i+1] );

    if ( ( boundaryType() == QwtSpline::ClosedPolygon )
        && ( controlLines.size() >= n ) )
    {
        path.cubicTo( l[n-1].p1(), l[n-1].p2(), p[0] );
        path.closeSubpath();
    }

    return path;
}

/*!
  \brief Find an interpolated polygon with "equidistant" points

  When withNodes is disabled all points of the resulting polygon
  will be equidistant according to the parametrization. 

  When withNodes is enabled the resulting polygon will also include
  the control points and the interpolated points are always aligned to
  the control point before ( points[i] + i * distance ).

  The implementation calculates bezier curves first and calculates
  the interpolated points in a second run.

  \param points Control nodes of the spline
  \param distance Distance between 2 points according 
                  to the parametrization
  \param withNodes When true, also add the control 
                   nodes ( even if not being equidistant )

  \sa bezierControlLines()
 */
QPolygonF QwtSpline::equidistantPolygon( const QPolygonF &points, 
    double distance, bool withNodes ) const
{
    if ( distance <= 0.0 )
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

    const QVector<QLineF> controlLines = bezierControlLines( points );

    if ( controlLines.size() < n - 1 )
        return path;

    path += points.first();
    double t = distance;

    const QPointF *p = points.constData();
    const QLineF *cl = controlLines.constData();

    for ( int i = 0; i < n - 1; i++ )
    {
        const double l = d_data->parametrization->valueIncrement( p[i], p[i+1] );

        while ( t < l )
        {
            path += qwtBezierPoint( p[i], cl[i].p1(),
                cl[i].p2(), p[i+1], t / l );

            t += distance;
        }

        if ( withNodes )
        {
            if ( qFuzzyCompare( path.last().x(), p[i+1].x() ) )
                path.last() = p[i+1];
            else
                path += p[i+1];

            t = distance;
        }
        else
        {
            t -= l;
        }
    }

    if ( ( boundaryType() == QwtSpline::ClosedPolygon )
        && ( controlLines.size() >= n ) )
    {
        const double l = d_data->parametrization->valueIncrement( p[n-1], p[0] );

        while ( t < l )
        {
            path += qwtBezierPoint( p[n-1], cl[n-1].p1(),
                cl[n-1].p2(), p[0], t / l );

            t += distance;
        }

        if ( qFuzzyCompare( path.last().x(), p[0].x() ) )
            path.last() = p[0];
        else 
            path += p[0];
    }

    return path;
}

//! Constructor
QwtSplineG1::QwtSplineG1()
{
}

//! Destructor
QwtSplineG1::~QwtSplineG1()
{
}

QwtSplineC1::QwtSplineC1()
{
    setParametrization( QwtSplineParametrization::ParameterX );
}

//! Destructor
QwtSplineC1::~QwtSplineC1()
{
}

/*!
  \brief Calculate an interpolated painter path

  Interpolates a polygon piecewise into cubic Bezier curves
  and returns them as QPainterPath.

  The implementation calculates the slopes at the control points
  and converts them into painter path elements in an additional loop.
  
  \param points Control points
  \return QPainterPath Painter path, that can be rendered by QPainter

  \note Derived spline classes might overload painterPath() to avoid
        the extra loops for converting results into a QPainterPath

  \sa slopesParametric()
 */
QPainterPath QwtSplineC1::painterPath( const QPolygonF &points ) const
{
    const int n = points.size();
    if ( n <= 2 )
        return QwtSpline::painterPath( points );

    using namespace QwtSplineC1P;

    PathStore store;
    switch( parametrization()->type() )
    {
        case QwtSplineParametrization::ParameterX:
        {
            store = qwtSplineC1PathParamX<PathStore>( this, points );
            break;
        }
        case QwtSplineParametrization::ParameterY:
        {
            store = qwtSplineC1PathParamY<PathStore>( this, points );
            break;
        }
        case QwtSplineParametrization::ParameterUniform:
        {
            store = qwtSplineC1PathParametric<PathStore>(
                this, points, paramUniform() );
            break;
        }
        case QwtSplineParametrization::ParameterCentripetal:
        {
            store = qwtSplineC1PathParametric<PathStore>(
                this, points, paramCentripetal() );
            break;
        }
        case QwtSplineParametrization::ParameterChordal:
        {
            store = qwtSplineC1PathParametric<PathStore>(
                this, points, paramChordal() );
            break;
        }
        default:
        {
            store = qwtSplineC1PathParametric<PathStore>(
                this, points, param( parametrization() ) );
        }
    }

    return store.path;
}

/*! 
  \brief Interpolate a curve with Bezier curves
    
  Interpolates a polygon piecewise with cubic Bezier curves
  and returns the 2 control points of each curve as QLineF.

  \param points Control points
  \return Control points of the interpolating Bezier curves
 */
QVector<QLineF> QwtSplineC1::bezierControlLines( const QPolygonF &points ) const
{
    using namespace QwtSplineC1P;

    const int n = points.size();
    if ( n <= 2 )
        return QVector<QLineF>();

    ControlPointsStore store;
    switch( parametrization()->type() )
    {
        case QwtSplineParametrization::ParameterX:
        {
            store = qwtSplineC1PathParamX<ControlPointsStore>( this, points );
            break;
        }
        case QwtSplineParametrization::ParameterY:
        {
            store = qwtSplineC1PathParamY<ControlPointsStore>( this, points );
            break;
        }
        case QwtSplineParametrization::ParameterUniform:
        {
            store = qwtSplineC1PathParametric<ControlPointsStore>(
                this, points, paramUniform() );
            break;
        }
        case QwtSplineParametrization::ParameterCentripetal:
        {
            store = qwtSplineC1PathParametric<ControlPointsStore>(
                this, points, paramCentripetal() );
            break;
        }
        case QwtSplineParametrization::ParameterChordal:
        {
            store = qwtSplineC1PathParametric<ControlPointsStore>(
                this, points, paramChordal() );
            break;
        }
        default:
        {
            store = qwtSplineC1PathParametric<ControlPointsStore>(
                this, points, param( parametrization() ) );
        }
    }

    return store.controlPoints;
}

QPolygonF QwtSplineC1::equidistantPolygon( const QPolygonF &points,
    double distance, bool withNodes ) const
{
    if ( parametrization()->type() == QwtSplineParametrization::ParameterX )
    {
        if ( points.size() > 2 )
        {
            const QVector<double> m = slopes( points );
            if ( m.size() != points.size() )
                return QPolygonF();

            return qwtPolygonParametric<QwtSplinePolynomial::fromSlopes>(
                distance, points, m, withNodes );
        }
    }

    return QwtSplineG1::equidistantPolygon( points, distance, withNodes );
}

QVector<QwtSplinePolynomial> QwtSplineC1::polynomials(
    const QPolygonF &points ) const
{
    QVector<QwtSplinePolynomial> polynomials;

    const QVector<double> m = slopes( points );
    if ( m.size() < 2 )
        return polynomials;

    for ( int i = 1; i < m.size(); i++ )
    {
        polynomials += QwtSplinePolynomial::fromSlopes( 
            points[i-1], m[i-1], points[i], m[i] );
    }

    return polynomials;
}

QwtSplineC2::QwtSplineC2()
{
}

//! Destructor
QwtSplineC2::~QwtSplineC2()
{
}

/*!
  \brief Interpolate a curve with Bezier curves

  Interpolates a polygon piecewise with cubic Bezier curves
  and returns them as QPainterPath.

  \param points Control points
  \return Painter path, that can be rendered by QPainter
 */
QPainterPath QwtSplineC2::painterPath( const QPolygonF &points ) const
{
    // could be implemented from curvaturesX without the extra
    // loop for calculating the slopes vector. TODO ...

    return QwtSplineC1::painterPath( points );
}

/*! 
  \brief Interpolate a curve with Bezier curves
    
  Interpolates a polygon piecewise with cubic Bezier curves
  and returns the 2 control points of each curve as QLineF.

  \param points Control points
  \return Control points of the interpolating Bezier curves
 */
QVector<QLineF> QwtSplineC2::bezierControlLines( const QPolygonF &points ) const
{
    // could be implemented from curvaturesX without the extra
    // loop for calculating the slopes vector. TODO ...

    return QwtSplineC1::bezierControlLines( points );
}

QPolygonF QwtSplineC2::equidistantPolygon( const QPolygonF &points,
    double distance, bool withNodes ) const
{
    if ( parametrization()->type() == QwtSplineParametrization::ParameterX )
    {
        if ( points.size() > 2 )
        {
            const QVector<double> cv = curvatures( points );
            if ( cv.size() != points.size() )
                return QPolygonF();

            return qwtPolygonParametric<QwtSplinePolynomial::fromCurvatures>( 
                distance, points, cv, withNodes );
        }
    }

    return QwtSplineC1::equidistantPolygon( points, distance, withNodes );
}

QVector<double> QwtSplineC2::slopes( const QPolygonF &points ) const
{
    const QVector<double> curvatures = this->curvatures( points );
    if ( curvatures.size() < 2 )
        return QVector<double>();
    
    QVector<double> slopes( curvatures.size() );

    const double *cv = curvatures.constData();
    double *m = slopes.data();

    const int n = points.size();
    const QPointF *p = points.constData();

    QwtSplinePolynomial polynomial;

    for ( int i = 0; i < n - 1; i++ )
    {
        polynomial = QwtSplinePolynomial::fromCurvatures( p[i], cv[i], p[i+1], cv[i+1] );
        m[i] = polynomial.c1;
    }

    m[n-1] = polynomial.slopeAt( p[n-1].x() - p[n-2].x() );

    return slopes;
}

QVector<QwtSplinePolynomial> QwtSplineC2::polynomials( const QPolygonF &points ) const
{
    QVector<QwtSplinePolynomial> polynomials;
    
    const QVector<double> curvatures = this->curvatures( points );
    if ( curvatures.size() < 2 )
        return polynomials;

    const QPointF *p = points.constData();
    const double *cv = curvatures.constData();
    const int n = curvatures.size();
    
    for ( int i = 1; i < n; i++ )
    {   
        polynomials += QwtSplinePolynomial::fromCurvatures(
            p[i-1], cv[i-1], p[i], cv[i] );
    }
    
    return polynomials;
}
