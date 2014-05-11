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

namespace QwtSplineAkima
{
    QWT_EXPORT QPainterPath path( const QPolygonF & );
    QWT_EXPORT QPainterPath path( const QPolygonF &,
        double slopeStart, double slopeEnd );
}

namespace QwtSplineBessel
{
    QWT_EXPORT QPainterPath path( const QPolygonF &, 
        double slopeStart, double slopeEnd );
}

namespace QwtSplineHarmonicMean
{
    QWT_EXPORT QPainterPath path( const QPolygonF & );
    QWT_EXPORT QPainterPath path( const QPolygonF &, 
        double slopeStart, double slopeEnd );
}

namespace QwtSplineNatural
{
    // curvatures at each point
    QWT_EXPORT QVector<double> curvatures( const QPolygonF & );

    QWT_EXPORT QPolygonF polygon( const QPolygonF &, int numPoints );

    // interpolated spline as bezier curve
    QWT_EXPORT QPainterPath path( const QPolygonF & );

    // calculate a, b, c
    QWT_EXPORT void toCoefficients( 
        double x1, double x2, double cv1,
        double y1, double y2, double cv2, 
        double &a, double &b, double &c );

}

inline void QwtSplineNatural::toCoefficients( 
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

#endif
