/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_H
#define QWT_SPLINE_H 1

#include "qwt_global.h"
#include <qpolygon.h>
#include <qpainterpath.h>

namespace QwtSpline
{
    // General spline interpolation according to 
    // "Smoothing with Cubic Splines" by D. S. G. Pollock

    QWT_EXPORT QPolygonF polygon( const QPolygonF &, double lambda, int numPoints );
    QWT_EXPORT QPainterPath path( const QPolygonF &, double lambda );

    // calculate a, b, c
    QWT_EXPORT void toCoefficients( 
        double x1, double x2, double cv1,
        double y1, double y2, double cv2, 
        double &a, double &b, double &c );

    QWT_EXPORT void toBezier( 
        double x1, double x2, double cv1,
        double y1, double y2, double cv2, 
        double &cpx1, double &cpy1, double &cpx2, double &cpy2 );
}

namespace QwtSplineAkima
{
    QWT_EXPORT QPainterPath path( const QPolygonF & );
}

namespace QwtSplineHarmonic
{
    QWT_EXPORT QPainterPath path( const QPolygonF & );
}

namespace QwtSplineNatural
{
    // curvatures at each point
    QWT_EXPORT QVector<double> curvatures( const QPolygonF & );

    QWT_EXPORT QPolygonF polygon( const QPolygonF &, int numPoints );

    // interpolated spline as bezier curve
    QWT_EXPORT QPainterPath path( const QPolygonF & );
}

inline void QwtSpline::toCoefficients( 
    double x1, double y1, double cv1, 
    double x2, double y2, double cv2, 
    double &a, double &b, double &c )
{
    const double dx = x2 - x1;
    const double dy = y2 - y1;

    a = ( cv2 - cv1 ) / ( 6.0 * dx );
    b = 0.5 * cv1;
    c = dy / dx - ( 0.5 * cv2 + cv1 ) * dx / 3.0;
}

inline void QwtSpline::toBezier(
    double x1, double y1, double cv1, 
    double x2, double y2, double cv2, 
    double &cpx1, double &cpy1, double &cpx2, double &cpy2 )
{
    const double dx = x2 - x1;
    const double dy = y2 - y1;
    const double stepX = dx / 3.0;

    const double c = dy / dx - ( 0.5 * cv2 + cv1 ) * stepX;

    cpx1 = x1 + stepX;
    cpy1 = y1 + c * stepX;

    cpx2 = x2 - stepX;
    cpy2 = cpy1 + ( c + 0.5 * cv1 * dx ) * stepX;
}

#endif
