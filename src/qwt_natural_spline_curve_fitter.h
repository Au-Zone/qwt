/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_NATURAL_SPLINE_CURVE_FITTER_H
#define QWT_NATURAL_SPLINE_CURVE_FITTER_H

#include "qwt_curve_fitter.h"

class QWT_EXPORT QwtNaturalSplineCurveFitter: public QwtCurveFitter
{
public:
    QwtNaturalSplineCurveFitter( int splineSize = 250 );
    virtual ~QwtNaturalSplineCurveFitter();

    void setSplineSize( int size );
    int splineSize() const;

    virtual QPolygonF fitCurve( const QPolygonF & ) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
