/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_BEZIER_CURVE_FITTER_H
#define QWT_BEZIER_CURVE_FITTER_H

#include "qwt_curve_fitter.h"

/*!
  \brief A curve fitter interpolating with Bezier curves

  QwtBezierSplineCurveFitter implements a spline interpolation algorithm
  like it is done in certain office packages.

  \sa QwtSplineCurveFitter
*/

class QWT_EXPORT QwtBezierSplineCurveFitter: public QwtCurveFitter
{
public:
    QwtBezierSplineCurveFitter( int splineSize = 250 );
    virtual ~QwtBezierSplineCurveFitter();

    void setSplineSize( int size );
    int splineSize() const;

    virtual QPolygonF fitCurve( const QPolygonF & ) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
