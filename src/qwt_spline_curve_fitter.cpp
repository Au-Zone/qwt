/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_curve_fitter.h"
#include "qwt_math.h"
#include "qwt_interval.h"

// - bezier

static inline double qwtLineLength( const QPointF &p_start, const QPointF &p_end )
{
    const double dx = p_start.x() - p_end.x();
    const double dy = p_start.y() - p_end.y();

    return qSqrt( dx * dx + dy * dy );
}

static inline QwtInterval qwtBezierIntervalX( const QPointF &p0, const QPointF &p1,
        const QPointF &p2, const QPointF &p3 )
{
    const double d02 = qwtLineLength( p0, p2 );
    const double d13 = qwtLineLength( p1, p3 );
    const double d12_2 = 0.5 * qwtLineLength( p1, p2 );

    const bool b1 = ( d02 / 6.0 ) < d12_2;
    const bool b2 = ( d13 / 6.0 ) < d12_2;

    double s1, s2;

    if( b1 && b2 )
    {
        s1 = ( p0 != p1 ) ? 1.0 / 6.0 : 1.0 / 3.0;
        s2 = ( p2 != p3 ) ? 1.0 / 6.0 : 1.0 / 3.0;
    }
    else if( !b1 && b2 )
    {
        s1 = d12_2 / d02;
        s2 = s1;
    }
    else if ( b1 && !b2 )
    {
        s1 = d12_2 / d13;
        s2 = s1;
    }
    else
    {
        s1 = d12_2 / d02;
        s2 = d12_2 / d13;
    }

    const double x1 = p1.x() + ( p2.x() - p0.x() ) * s1;
    const double x2 = p2.x() + ( p1.x() - p3.x() ) * s2;

    return QwtInterval( 3.0 * x1, 3.0 * x2 );
}

static inline QwtInterval qwtBezierIntervalY( const QPointF &p0, const QPointF &p1,
        const QPointF &p2, const QPointF &p3 )
{
    const double d02 = qwtLineLength( p0, p2 );
    const double d13 = qwtLineLength( p1, p3 );
    const double d12_2 = 0.5 * qwtLineLength( p1, p2 );

    const bool b1 = ( d02 / 6.0 ) < d12_2;
    const bool b2 = ( d13 / 6.0 ) < d12_2;

    double s1, s2;

    if( b1 && b2 )
    {
        s1 = ( p0 != p1 ) ? 1.0 / 6.0 : 1.0 / 3.0;
        s2 = ( p2 != p3 ) ? 1.0 / 6.0 : 1.0 / 3.0;
    }
    else if( !b1 && b2 )
    {
        s1 = d12_2 / d02;
        s2 = s1;
    }
    else if ( b1 && !b2 )
    {
        s1 = d12_2 / d13;
        s2 = s1;
    }
    else
    {
        s1 = d12_2 / d02;
        s2 = d12_2 / d13;
    }

    const double y1 = p1.y() + ( p2.y() - p0.y() ) * s1;
    const double y2 = p2.y() + ( p1.y() - p3.y() ) * s2;

    return QwtInterval( 3.0 * y1, 3.0 * y2 );
}

static inline double qwtBezierValueX( const QPointF &p1, const QPointF &p2,
    const QwtInterval &interval, double x )
{
    const double s1 = ( x - p1.x() ) / ( p2.x() - p1.x() );
    const double s2  = 1.0 - s1;

    const double a1 = s1 * interval.minValue();
    const double a2 = s1 * s1 * interval.maxValue();
    const double a3 = s1 * s1 * s1 * p2.x();

    return ( ( s2 * p1.x() + a1 ) * s2 + a2 ) * s2 + a3;
}

static inline double qwtBezierValueY( const QPointF &p1, const QPointF &p2,
    const QwtInterval &interval, double x )
{
    const double s1 = ( x - p1.x() ) / ( p2.x() - p1.x() );
    const double s2  = 1.0 - s1;

    const double a1 = s1 * interval.minValue();
    const double a2 = s1 * s1 * interval.maxValue();
    const double a3 = s1 * s1 * s1 * p2.y();

    return ( ( s2 * p1.y() + a1 ) * s2 + a2 ) * s2 + a3;
}

static QPolygonF qwtFitBezier( const QPolygonF &points, int numPoints )
{
    const int psize = points.size();
    if ( psize <= 2 )
        return points;

    const QPointF *p = points.constData();

    // --- 

    const double x1 = points.first().x();
    const double x2 = points.last().x();
    const double delta = ( x2 - x1 ) / ( numPoints - 1 );

    QwtInterval intvX = qwtBezierIntervalX( p[0], p[0], p[1], p[2] );
    QwtInterval intvY = qwtBezierIntervalY( p[0], p[0], p[1], p[2] );

    QPolygonF fittedPoints;
    for ( int i = 0, j = 0; i < numPoints; i++ )
    {
        double x = x1 + i * delta;
        if ( x > x2 )
            x = x2;

        if ( x > p[j + 1].x() )
        {
            while ( x > p[j + 1].x() )
                j++;

            const int j2 = qMin( psize - 1, j + 2 );
            intvX = qwtBezierIntervalX( p[j - 1], p[j], p[j + 1], p[j2] );
            intvY = qwtBezierIntervalY( p[j - 1], p[j], p[j + 1], p[j2] );
        }

        const double x_ = qwtBezierValueX( points[j], points[j + 1], intvX, x );
        const double y = qwtBezierValueY( points[j], points[j + 1], intvY, x );

        fittedPoints += QPointF( x_, y );
    }

    return fittedPoints;
}

// - natural

static QVector<double> qwtCofficientNatural( const QPolygonF &points )
{
    const int size = points.size();

    QVector<double> aa0( size - 1 );
    QVector<double> bb0( size - 1 );

    double dx1 = points[1].x() - points[0].x();
    double dy1 = ( points[1].y() - points[0].y() ) / dx1;

    for ( int i = 1; i < size - 1; i++ )
    {
        const double dx2 = points[i+1].x() - points[i].x();
        const double dy2 = ( points[i+1].y() - points[i].y() ) / dx2;

        aa0[i] = 2.0 * ( dx1 + dx2 );
        bb0[i] = 6.0 * ( dy1 - dy2 );

        dy1 = dy2;
        dx1 = dx2;
    }

    // L-U Factorization
    QVector<double> cc0( size - 1 );
    for ( int i = 1; i < size - 2; i++ )
    {
        const double dx = points[i+1].x() - points[i].x();

        cc0[i] = dx / aa0[i];
        aa0[i+1] -= dx * cc0[i];
    }

    // forward elimination
    QVector<double> s( size );
    s[1] = bb0[1];
    for ( int i = 2; i < size - 1; i++ )
    {
        s[i] = bb0[i] - cc0[i-1] * s[i-1];
    }

    // backward elimination
    s[size - 2] = - s[size - 2] / aa0[size - 2];
    for ( int i = size - 3; i > 0; i-- )
    {
        const double dx = points[i+1].x() - points[i].x();
        s[i] = - ( s[i] + dx * s[i+1] ) / aa0[i];
    }
    s[size - 1] = s[0] = 0.0;

    return s;
}

static QPolygonF qwtFitNatural( const QPolygonF &points, int numPoints )
{
    const int size = points.size();
    if ( size <= 2 )
        return points;

    const QVector<double> s = qwtCofficientNatural( points );

    const double x1 = points.first().x();
    const double x2 = points.last().x();
    const double delta = ( x2 - x1 ) / ( numPoints - 1 );

    const QPointF *p = points.constData();

    double ai, bi, ci;
    QPointF pi;

    QPolygonF fittedPoints;
    for ( int i = 0, j = 0; i < numPoints; i++ )
    {
        double x = x1 + i * delta;
        if ( x > x2 )
            x = x2;

        if ( i == 0 || x > p[j + 1].x() )
        {
            while ( x > p[j + 1].x() )
                j++;

            const double dx = p[j + 1].x() - p[j].x();
            const double dy = p[j + 1].y() - p[j].y();

            ai = ( s[j+1] - s[j] ) / ( 6.0 * dx );
            bi = 0.5 * s[j];
            ci = dy / dx - ( s[j+1] + 2.0 * s[j] ) * dx / 6.0;
            pi = p[j];
        }

        const double dx = x - pi.x();
        const double y = ( ( ( ai * dx ) + bi ) * dx + ci ) * dx + pi.y();

        fittedPoints += QPointF( x, y );
    }

    return fittedPoints;
}

// - parametric

static QVector<double> qwtCofficientsParametric( const QVector<double> &xValues, const QVector<double> &yValues )
{
    const int size = xValues.size();

    QVector<double> aa0( size - 1 );
    QVector<double> cc0( size - 1 );
    QVector<double> dd0( size - 1 );

    double dx1 = xValues[size - 1] - xValues[size - 2];
    double dy1 = ( yValues[0] - yValues[size - 2] ) / dx1;

    for ( int i = 0; i < size - 1; i++ )
    {
        const double dx2 = xValues[i+1] - xValues[i];
        const double dy2 = ( yValues[i+1] - yValues[i] ) / dx2;

        aa0[i] = 2.0 * ( dx1 + dx2 );
        cc0[i] = dx2;
        dd0[i] = 6.0 * ( dy1 - dy2 );

        dy1 = dy2;
        dx1 = dx2;
    }

    // L-U Factorization
    aa0[0] = qSqrt( aa0[0] );
    cc0[0] = ( xValues[size - 1] - xValues[size - 2] ) / aa0[0];
    double sum0 = 0;

    QVector<double> bb0( size - 1 );
    for ( int i = 0; i < size - 3; i++ )
    {
        const double dx = xValues[i+1] - xValues[i];

        bb0[i] = dx / aa0[i];
        if ( i > 0 )
            cc0[i] = - cc0[i-1] * bb0[i-1] / aa0[i];
        aa0[i+1] = qSqrt( aa0[i+1] - qwtSqr( bb0[i] ) );
        sum0 += qwtSqr( cc0[i] );
    }

    const double dxx = xValues[size - 2] - xValues[size - 3];
    bb0[size - 3] = ( dxx - cc0[size - 4] * bb0[size - 4] ) / aa0[size - 3];
    aa0[size - 2] = qSqrt( aa0[size - 2] - qwtSqr( bb0[size - 3] ) - sum0 );


    QVector<double> s( size );
    s[0] = dd0[0] / aa0[0];

    double sum1 = 0;
    for ( int i = 1; i < size - 2; i++ )
    {
        s[i] = ( dd0[i] - bb0[i-1] * s[i-1] ) / aa0[i];
        sum1 += cc0[i-1] * s[i-1];
    }

    s[size - 2] = ( dd0[size - 2] - bb0[size - 3] * s[size - 3] - sum1 ) / aa0[size - 2];
    s[size - 2] = - s[size - 2] / aa0[size - 2];
    s[size - 3] = -( s[size - 3] + bb0[size - 3] * s[size - 2] ) / aa0[size - 3];
    for ( int i = size - 4; i >= 0; i-- )
        s[i] = - ( s[i] + bb0[i] * s[i+1] + cc0[i] * s[size - 2] ) / aa0[i];

    s[size-1] = s[0];

    return s;
}

static QPolygonF qwtFitParametric( const QPolygonF &points, int numPoints ) 
{
    const int size = points.size();
    if ( size <= 2 )
        return points;

    QVector<double> xValues( size );
    QVector<double> yValuesX( size );
    QVector<double> yValuesY( size );

    const QPointF *p = points.data();

    double param = 0.0;
    for ( int i = 0; i < size; i++ )
    {
        const double x = p[i].x();
        const double y = p[i].y();

        if ( i > 0 )
        {
            const double d1 = x - yValuesX[i-1];
            const double d2 = y - yValuesY[i-1];

            const double delta = qSqrt( d1 * d1 + d2 * d2 );
            param += qMax( delta, 1.0 );
        }
        
        xValues[i] = param;
        yValuesX[i] = x;
        yValuesY[i] = y;
    }

    const QVector<double> sX = qwtCofficientsParametric( xValues, yValuesX );
    const QVector<double> sY = qwtCofficientsParametric( xValues, yValuesY );

    QVector<double> aaX( size );
    QVector<double> bbX( size );
    QVector<double> ccX( size );

    QVector<double> aaY( size );
    QVector<double> bbY( size );
    QVector<double> ccY( size );

    for ( int i = 0; i < size - 1; i++ )
    {
        const double dx = xValues[i+1] - xValues[i];

        aaX[i] = ( sX[i+1] - sX[i] ) / ( 6.0 * dx );
        bbX[i] = 0.5 * sX[i];
        ccX[i] = ( yValuesX[i+1] - yValuesX[i] )
            / dx - ( sX[i+1] + 2.0 * sX[i] ) * dx / 6.0;
    
        aaY[i] = ( sY[i+1] - sY[i] ) / ( 6.0 * dx );
        bbY[i] = 0.5 * sY[i];
        ccY[i] = ( yValuesY[i+1] - yValuesY[i] )
            / dx - ( sY[i+1] + 2.0 * sY[i] ) * dx / 6.0;
    }

    const double x2 = xValues.last();
    const double deltaX = x2 / ( numPoints - 1 );

    double ax, bx, cx, ay, by, cy;

    QPolygonF fittedPoints;
    for ( int i = 0, j = 0; i < numPoints; i++ )
    {
        double x = i * deltaX;
        if ( x > x2 )
            x = x2;

        if ( i == 0 || x > xValues[j + 1] )
        {
            while ( x > xValues[j + 1] )
                j++;

            ax = aaX[j];
            bx = bbX[j];
            cx = ccX[j];

            ay = aaY[j];
            by = bbY[j];
            cy = ccY[j];
        }

        const double dx = x - xValues[j];

        double px = ( ( ( ax * dx ) + bx ) * dx + cx ) * dx + yValuesX[j];
        double py = ( ( ( ay * dx ) + by ) * dx + cy ) * dx + yValuesY[j];

        fittedPoints += QPointF( px, py );
    }

    return fittedPoints;
}

// ---
class QwtSplineCurveFitter::PrivateData
{
public:
    PrivateData():
        splineSize( 250 )
    {
    }

    QwtSplineCurveFitter::FitMode fitMode;
    int splineSize;
};

//! Constructor
QwtSplineCurveFitter::QwtSplineCurveFitter( int splineSize, FitMode fitMode )
{
    d_data = new PrivateData;

	d_data->fitMode = fitMode;
	setSplineSize( splineSize );
}

//! Destructor
QwtSplineCurveFitter::~QwtSplineCurveFitter()
{
    delete d_data;
}

/*!
  Select the algorithm used for building the spline

  \param mode Mode representing a spline algorithm
  \sa fitMode()
*/
void QwtSplineCurveFitter::setFitMode( FitMode mode )
{
    d_data->fitMode = mode;
}

/*!
  \return Mode representing a spline algorithm
  \sa setFitMode()
*/
QwtSplineCurveFitter::FitMode QwtSplineCurveFitter::fitMode() const
{
    return d_data->fitMode;
}

/*!
   Assign a spline size ( has to be at least 10 points )

   \param splineSize Spline size
   \sa splineSize()
*/
void QwtSplineCurveFitter::setSplineSize( int splineSize )
{
    d_data->splineSize = qMax( splineSize, 10 );
}

/*!
  \return Spline size
  \sa setSplineSize()
*/
int QwtSplineCurveFitter::splineSize() const
{
    return d_data->splineSize;
}

/*!
  Find a curve which has the best fit to a series of data points

  \param points Series of data points
  \return Curve points
*/
QPolygonF QwtSplineCurveFitter::fitCurve( const QPolygonF &points ) const
{
    const int size = points.size();
    if ( size <= 2 )
        return points;

	QPolygonF fittedPoints;

	switch( d_data->fitMode )
	{
      	case BezierSpline:
		{
			fittedPoints = qwtFitBezier( points, d_data->splineSize );
			break;
		}
        case NaturalSpline:
		{
			fittedPoints = qwtFitNatural( points, d_data->splineSize );
			break;
		}
        case ParametricSpline:
		{
			fittedPoints = qwtFitParametric( points, d_data->splineSize );
			break;
		}
		default:
		{
			fittedPoints = points;
		}
	}

	return fittedPoints;
}
