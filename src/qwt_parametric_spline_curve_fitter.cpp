/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_parametric_spline_curve_fitter.h"
#include "qwt_math.h"
#include <qpolygon.h>

static QVector<double> qwtCofficients( const QVector<double> &xValues, const QVector<double> &yValues )
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

    const QVector<double> sX = qwtCofficients( xValues, yValuesX );
    const QVector<double> sY = qwtCofficients( xValues, yValuesY );

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

    const double deltaX = xValues.last() / ( numPoints - 1 );

    double ax, bx, cx, ay, by, cy;

    QPolygonF fittedPoints;
    for ( int i = 0, j = 0; i < numPoints; i++ )
    {
        const double x = i * deltaX;

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

class QwtParametricSplineCurveFitter::PrivateData
{
public:
    PrivateData():
        splineSize( 250 )
    {
    }

    int splineSize;
};

//! Constructor
QwtParametricSplineCurveFitter::QwtParametricSplineCurveFitter( int splineSize )
{
    d_data = new PrivateData;
    setSplineSize( splineSize );
}

//! Destructor
QwtParametricSplineCurveFitter::~QwtParametricSplineCurveFitter()
{
    delete d_data;
}

/*!
   Assign a spline size ( has to be at least 10 points )

   \param splineSize Spline size
   \sa splineSize()
*/
void QwtParametricSplineCurveFitter::setSplineSize( int splineSize )
{
    d_data->splineSize = qMax( splineSize, 10 );
}

/*!
  \return Spline size
  \sa setSplineSize()
*/
int QwtParametricSplineCurveFitter::splineSize() const
{
    return d_data->splineSize;
}

/*!
  Find a curve which has the best fit to a series of data points

  \param points Series of data points
  \return Curve points
*/
QPolygonF QwtParametricSplineCurveFitter::fitCurve( const QPolygonF &points ) const
{
    return qwtFitParametric( points, d_data->splineSize );
}
