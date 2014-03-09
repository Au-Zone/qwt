/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_curve_fitter.h"
#include "qwt_spline.h"

//! Constructor
QwtSplineCurveFitter::QwtSplineCurveFitter()
{
}

//! Destructor
QwtSplineCurveFitter::~QwtSplineCurveFitter()
{
}

/*!
  Find a curve which has the best fit to a series of data points

  \param points Series of data points
  \return Curve points
*/
QPolygonF QwtSplineCurveFitter::fitCurve( const QPolygonF &points ) const
{
    if ( points.size() <= 2 )
        return points;

    const QPainterPath path = QwtSpline::bezierPath( points );

    QPolygonF fittedPoints;

    const QList<QPolygonF> subPaths = path.toSubpathPolygons();
    if ( !subPaths.isEmpty() )
        fittedPoints = subPaths.first();

    return fittedPoints;
}
