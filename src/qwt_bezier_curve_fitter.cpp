/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_bezier_curve_fitter.h"
#include "qwt_bezier_spline.h"

class QwtBezierSplineCurveFitter::PrivateData
{
public:
    PrivateData():
        splineSize( 250 )
    {
    }

    QwtBezierSpline spline;
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
  Assign a bezier

  \param bezier Bezier
  \sa bezier()
*/
void QwtBezierSplineCurveFitter::setSpline( const QwtBezierSpline &spline )
{
    d_data->spline = spline;
    d_data->spline.reset();
}

/*!
  \return Bezier
  \sa setBezier()
*/
const QwtBezierSpline &QwtBezierSplineCurveFitter::spline() const
{
    return d_data->spline;
}

/*!
  \return Bezier
  \sa setBezier()
*/
QwtBezierSpline &QwtBezierSplineCurveFitter::spline()
{
    return d_data->spline;
}

/*!
   Assign a bezier size ( has to be at least 10 points )

   \param splineSize Spline size
   \sa splineSize()
*/
void QwtBezierSplineCurveFitter::setSplineSize( int splineSize )
{
    d_data->splineSize = qMax( splineSize, 10 );
}

/*!
  \return Bezier size
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
    const int size = points.size();
    if ( size <= 2 )
        return points;

    return fitSpline( points );
}

QPolygonF QwtBezierSplineCurveFitter::fitSpline( const QPolygonF &points ) const
{
    d_data->spline.setPoints( points );
    if ( !d_data->spline.isValid() )
        return points;

    QPolygonF fittedPoints( d_data->splineSize );

    const double x1 = points[0].x();
    const double x2 = points[int( points.size() - 1 )].x();
    const double dx = x2 - x1;
    const double delta = dx / ( d_data->splineSize - 1 );

    for ( int i = 0; i < d_data->splineSize; i++ )
    {
        QPointF &p = fittedPoints[i];

        const double v = x1 + i * delta;
        const double sv = d_data->spline.value( v );

        p.setX( v );
        p.setY( sv );
    }
    d_data->spline.reset();

    return fittedPoints;
}
