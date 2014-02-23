/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_bezier_curve_fitter.h"
#include "qwt_math.h"
#include "qwt_interval.h"

static inline double qwtLineLength( const QPointF &p_start, const QPointF &p_end )
{
    const double dx = p_start.x() - p_end.x();
    const double dy = p_start.y() - p_end.y();

    return qSqrt( dx * dx + dy * dy );
}

static inline QwtInterval qwtBezierInterval( const QPointF &p0, const QPointF &p1,
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

static inline double qwtBezierValue( const QPointF &p1, const QPointF &p2,
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

    QwtInterval intv = qwtBezierInterval( p[0], p[0], p[1], p[2] );

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
            intv = qwtBezierInterval( p[j - 1], p[j], p[j + 1], p[j2] );
        }

        const double y = qwtBezierValue( points[j], points[j + 1], intv, x );

        fittedPoints += QPointF( x, y );
    }

    return fittedPoints;
}

class QwtBezierSplineCurveFitter::PrivateData
{
public:
    PrivateData():
        splineSize( 250 )
    {
    }

    int splineSize;
};

//! Constructor
QwtBezierSplineCurveFitter::QwtBezierSplineCurveFitter( int splineSize )
{
    d_data = new PrivateData;
    setSplineSize( splineSize );
}

//! Destructor
QwtBezierSplineCurveFitter::~QwtBezierSplineCurveFitter()
{
    delete d_data;
}

/*!
   Assign a spline size ( has to be at least 10 points )

   \param splineSize Spline size
   \sa splineSize()
*/
void QwtBezierSplineCurveFitter::setSplineSize( int splineSize )
{
    d_data->splineSize = qMax( splineSize, 10 );
}

/*!
  \return Spline size
  \sa setSplineSize()
*/
int QwtBezierSplineCurveFitter::splineSize() const
{
    return d_data->splineSize;
}

/*!
  Find a curve which has the best fit to a series of data points

  \param points Series of data points
  \return Curve points
*/
QPolygonF QwtBezierSplineCurveFitter::fitCurve( const QPolygonF &points ) const
{
    return qwtFitBezier( points, d_data->splineSize );
}
