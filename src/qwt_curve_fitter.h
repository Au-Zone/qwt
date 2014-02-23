/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_CURVE_FITTER_H
#define QWT_CURVE_FITTER_H

#include "qwt_global.h"
#include <qpolygon.h>

/*!
  \brief Abstract base class for a curve fitter
*/
class QWT_EXPORT QwtCurveFitter
{
public:
    virtual ~QwtCurveFitter();

    /*!
        Find a curve which has the best fit to a series of data points

        \param polygon Series of data points
        \return Curve points
     */
    virtual QPolygonF fitCurve( const QPolygonF &polygon ) const = 0;

protected:
    QwtCurveFitter();

private:
    QwtCurveFitter( const QwtCurveFitter & );
    QwtCurveFitter &operator=( const QwtCurveFitter & );
};

#endif
