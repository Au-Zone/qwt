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
#include "qwt_spline_polynom.h"
#include <qpolygon.h>
#include <qpainterpath.h>

class QWT_EXPORT QwtSpline
{
public:
    QwtSpline();
    virtual ~QwtSpline();

    virtual QPainterPath path( const QPolygonF & ) const = 0;
    virtual QPolygonF polygon( int numPoints, const QPolygonF & ) const = 0;
    virtual QVector<QwtSplinePolynom> polynoms( const QPolygonF & ) const = 0;

    virtual QPainterPath parametricPath( const QPolygonF & ) const = 0;
};

#endif
