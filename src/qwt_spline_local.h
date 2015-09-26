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

class QWT_EXPORT QwtSplineLocal: public QwtSplineC1
{
public:
    enum Type
    {
        Cardinal,
        ParabolicBlending, // not implemented
        Akima,
        HarmonicMean,
        PChip // not implemented
    };

    QwtSplineLocal( Type type, double tension = 0.0 );
    virtual ~QwtSplineLocal();

    Type type() const;

    void setTension( double tension );
    double tension() const;

    virtual QPainterPath painterPath( const QPolygonF & ) const;
    virtual QVector<QLineF> bezierControlLines( const QPolygonF & ) const;

    virtual QVector<double> slopesX( const QPolygonF & ) const;

    virtual QPainterPath pathX( const QPolygonF & ) const;
    virtual QVector<QLineF> bezierControlPointsX( const QPolygonF & ) const;
    virtual QVector<QwtSplinePolynomial> polynomialsX( const QPolygonF & ) const;

    virtual uint locality() const;

private:
    const Type d_type;
    double d_tension;
};

#endif
