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

namespace QwtSplineHarmonicMean
{
    QWT_EXPORT QPainterPath path( const QPolygonF & );
    QWT_EXPORT QPainterPath path( const QPolygonF &, 
        double slopeStart, double slopeEnd );
}

namespace QwtSplineCubic
{
    enum EndpointCondition 
    {
        Natural,
        NotAKnot,

        Test1,
        Test2,
        Test3
    };

    // spline with endpoint conditions
    QWT_EXPORT QVector<double> derivatives( 
        const QPolygonF &, EndpointCondition = Natural );

    QWT_EXPORT QPainterPath path( 
        const QPolygonF &, EndpointCondition = Natural );

    QWT_EXPORT QPolygonF polygon( int numPoints,
        const QPolygonF &, EndpointCondition = Natural );

    // clamped spline

    QWT_EXPORT QVector<double> derivatives( 
        const QPolygonF &, double slopeBegin, double slopeEnd );

    QWT_EXPORT QPainterPath path( 
        const QPolygonF &, double slopeBegin, double slopeEnd );

    QWT_EXPORT QPolygonF polygon( int numPoints,
        const QPolygonF &, double slopeBegin, double slopeEnd );
};

#endif
