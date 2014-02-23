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

class QwtSpline;

/*!
  \brief A curve fitter using cubic splines
  \sa QwtBezierSplineCurveFitter
*/
class QWT_EXPORT QwtSplineCurveFitter: public QwtCurveFitter
{
public:
    /*!
      Spline type
      The default setting is Auto
      \sa setFitMode(), FitMode()
     */
    enum FitMode
    {
        /*!
          Use the default spline algorithm for polygons with
          increasing x values ( p[i-1] < p[i] ), otherwise use
          a parametric spline algorithm.
         */
        Auto,

        //! Use a default spline algorithm
        Spline,

        //! Use a parametric spline algorithm
        ParametricSpline
    };

    QwtSplineCurveFitter( int splineSize = 250 );
    virtual ~QwtSplineCurveFitter();

    void setFitMode( FitMode );
    FitMode fitMode() const;

    void setSpline( const QwtSpline& );

    const QwtSpline &spline() const;
    QwtSpline &spline();

    void setSplineSize( int size );
    int splineSize() const;

    virtual QPolygonF fitCurve( const QPolygonF & ) const;

private:
    QPolygonF fitSpline( const QPolygonF & ) const;
    QPolygonF fitParametric( const QPolygonF & ) const;

    class PrivateData;
    PrivateData *d_data;
};

#endif
