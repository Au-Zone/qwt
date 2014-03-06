/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_H
#define QWT_SPLINE_H 1

#include "qwt_global.h"
#include <qpolygon.h>

namespace QwtSpline
{
    QWT_EXPORT QPolygonF fitBezier( const QPolygonF &, int numPoints );
    QWT_EXPORT QPolygonF fitNatural( const QPolygonF &, int numPoints );
    QWT_EXPORT QPolygonF fitParametric( const QPolygonF &, int numPoints );
};

#endif
