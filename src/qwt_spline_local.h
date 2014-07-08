/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_LOCAL_H
#define QWT_SPLINE_LOCAL_H 1

#include "qwt_global.h"
#include "qwt_spline.h"
#include <qpainterpath.h>

class QWT_EXPORT QwtSplineLocal: public QwtSpline
{
public:
    enum Type
    {
        Akima,
        HarmonicMean,
        PChip
    };

    QwtSplineLocal( Type type );
    virtual ~QwtSplineLocal();

    QPainterPath path( const QPolygonF &,
        double slopeStart, double slopeEnd ) const;

    QVector<double> slopes( const QPolygonF &,
        double slopeStart, double slopeEnd ) const;

    virtual QPainterPath path( const QPolygonF & ) const;
    virtual QVector<double> slopes( const QPolygonF & ) const;

private:
    const Type d_type;
};

#endif
