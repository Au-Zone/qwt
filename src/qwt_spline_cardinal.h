/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_CARDINAL_H
#define QWT_SPLINE_CARDINAL_H 1

#include "qwt_global.h"
#include "qwt_spline.h"

class QWT_EXPORT QwtSplineCardinal: public QwtSplineC1
{
public:
    QwtSplineCardinal( double tension = 0.0 );
    virtual ~QwtSplineCardinal();

    void setTension( double tension );
    double tension() const;

    virtual QVector<double> slopesX( const QPolygonF & ) const;

private:
    double d_tension;
};

#endif
