/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_CUBIC_H
#define QWT_SPLINE_CUBIC_H 1

#include "qwt_global.h"
#include "qwt_spline.h"

class QWT_EXPORT QwtSplineCubic: public QwtSpline
{
public:
    enum EndpointCondition 
    {
        Natural,
        ParabolicRunout,
        CubicRunout,
        NotAKnot,
        Periodic
    };

    QwtSplineCubic();
    virtual ~QwtSplineCubic();

    void setEndConditions( EndpointCondition );

    void setClamped( double slopeBegin, double slopeEnd );
    void setClamped2( double curvatureBegin, double curvatureEnd );
    void setClamped3( double valueBegin, double valueEnd );

    virtual QVector<double> slopes( const QPolygonF & ) const;
    virtual QVector<double> curvatures( const QPolygonF & ) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
