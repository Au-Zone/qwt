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
    static void toPolygon( double tolerance,
        const QPointF &p1, const QPointF &cp1,
        const QPointF &cp2, const QPointF &p2,
        QPolygonF &polygon );
};

#endif
