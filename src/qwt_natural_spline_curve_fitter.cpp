/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_natural_spline_curve_fitter.h"
#include <qpolygon.h>
#include <qdebug.h>

static QVector<double> qwtCofficients( const QPolygonF &points )
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

    const QVector<double> s = qwtCofficients( points );

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

class QwtNaturalSplineCurveFitter::PrivateData
{
public:
    PrivateData():
        splineSize( 250 )
    {
    }

    int splineSize;
};

//! Constructor
QwtNaturalSplineCurveFitter::QwtNaturalSplineCurveFitter( int splineSize )
{
    d_data = new PrivateData;
    setSplineSize( splineSize );
}

//! Destructor
QwtNaturalSplineCurveFitter::~QwtNaturalSplineCurveFitter()
{
    delete d_data;
}

/*!
   Assign a spline size ( has to be at least 10 points )

   \param splineSize Spline size
   \sa splineSize()
*/
void QwtNaturalSplineCurveFitter::setSplineSize( int splineSize )
{
    d_data->splineSize = qMax( splineSize, 10 );
}

/*!
  \return Spline size
  \sa setSplineSize()
*/
int QwtNaturalSplineCurveFitter::splineSize() const
{
    return d_data->splineSize;
}

/*!
  Find a curve which has the best fit to a series of data points

  \param points Series of data points
  \return Curve points
*/
QPolygonF QwtNaturalSplineCurveFitter::fitCurve( const QPolygonF &points ) const
{
    return qwtFitNatural( points, d_data->splineSize );
}
