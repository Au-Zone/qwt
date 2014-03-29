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
}

namespace QwtSplineAkima
{
    QWT_EXPORT QPainterPath path( const QPolygonF & );
}

namespace QwtSplineNatural
{
    // all b's
    QWT_EXPORT QVector<double> quadraticCoefficients( const QPolygonF & );

    // calculate a, b from b[i] and b[i+1]
    QWT_EXPORT void coefficients( const QPointF &p1, const QPointF &p2, 
        double b, double bnext, double &a, double &c );

    QWT_EXPORT QPolygonF polygon( const QPolygonF &, int numPoints );

    // interpolated spline as bezier curve
    QWT_EXPORT QPainterPath path( const QPolygonF & );
}

inline void QwtSplineNatural::coefficients( const QPointF &p1, const QPointF &p2, 
    double b, double bnext, double &a, double &c )
{
    const double dx = p2.x() - p1.x();
    const double dy = p2.y() - p1.y();

    a = ( bnext - b ) / ( 3.0 * dx );
    c = dy / dx - ( bnext + 2.0 * b ) * dx / 3.0;
}

#endif
