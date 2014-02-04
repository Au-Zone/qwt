/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_BEZIER_SPLINE_H
#define QWT_BEZIER_SPLINE_H

#include "qwt_global.h"
#include <qpolygon.h>
#include <qvector.h>

/*!
  \brief A class for Bezier spline interpolation

  The QwtBezierSpline class is used for cubical Bezier interpolation.

  \par Usage:
  <ol>
  <li>First call setPoints() to determine the Bezier coefficients
      for a tabulated function y(x).
  <li>After the coefficients have been set up, the interpolated
      function value for an argument x can be determined by calling
      QwtBezier::value().
  </ol>

  \par Example:
  \code
#include <qwt_bezier_spline.h>

QPolygonF interpolate(const QPolygonF& points, int numValues)
{
    QwtBezierSpline spline;
    if ( !spline.setPoints(points) )
        return points;

    QPolygonF interpolatedPoints(numValues);

    const double delta =
        (points[numPoints - 1].x() - points[0].x()) / (points.size() - 1);
    for(i = 0; i < points.size(); i++)  / interpolate
    {
        const double x = points[0].x() + i * delta;
        interpolatedPoints[i].setX(x);
        interpolatedPoints[i].setY(spline.value(x));
    }
    return interpolatedPoints;
}
  \endcode
*/

class QWT_EXPORT QwtBezierSpline
{
public:
    QwtBezierSpline();
    QwtBezierSpline( const QwtBezierSpline & );

    ~QwtBezierSpline();

    QwtBezierSpline &operator=( const QwtBezierSpline & );

    bool setPoints( const QPolygonF& points );
    QPolygonF points() const;

    void reset();

    bool isValid() const;
    double value( double x ) const;

protected:
    bool buildBezier( const QPolygonF & );

private:
    void computeBezierPoints( const QPointF &, const QPointF &,
                              const QPointF &, const QPointF &, int bz_curve );

    class PrivateData;
    PrivateData *d_data;
};

#endif
