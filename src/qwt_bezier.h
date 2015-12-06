/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_BEZIER_H
#define QWT_BEZIER_H

#include "qwt_global.h"
#include <qpolygon.h>

class QWT_EXPORT QwtBezier
{
public:

    // recursive subdivision
    static QPolygonF toPolygon( double tolerance,
        double x1, double y1, double cx1, double cy1,
        double cx2, double cy2, double x2, double y2, 
        bool withLastPoint = true );
};

#endif
