/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_CURVE_FITTER_H
#define QWT_SPLINE_CURVE_FITTER_H

#include "qwt_curve_fitter.h"

/*!
  \brief A curve fitter using cubic splines
  \sa QwtBezierSplineCurveFitter
*/
class QWT_EXPORT QwtSplineCurveFitter: public QwtCurveFitter
{
public:
    /*!
      Spline type
      \sa setFitMode(), FitMode()
     */
    enum FitMode
    {
        /*!
          A spline algorithm using Bezier interpolation like
          it is used in certain office packages
         */
        BezierSpline,

        //! Use a default spline algorithm
        NaturalSpline,

        //! Use a parametric spline algorithm
        ParametricSpline
    };

    QwtSplineCurveFitter( int splineSize = 250, FitMode = BezierSpline );
    virtual ~QwtSplineCurveFitter();

    void setFitMode( FitMode );
    FitMode fitMode() const;

    void setSplineSize( int size );
    int splineSize() const;

    virtual QPolygonF fitCurve( const QPolygonF & ) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
