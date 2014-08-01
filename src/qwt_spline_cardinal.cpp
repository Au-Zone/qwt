/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_cardinal.h"
#include <qmath.h>

static inline double qwtChordal( const QPointF &p1, const QPointF &p2 )
{
   const double dx = p1.x() - p2.x();
   const double dy = p1.y() - p2.y();

   return qSqrt( dx * dx + dy * dy );
}

QwtSplineCardinal::QwtSplineCardinal( double tension ):
    d_tension( 0.0 )
{
    setTension( tension );
}

QwtSplineCardinal::~QwtSplineCardinal()
{
}

void QwtSplineCardinal::setTension( double tension )
{
    d_tension = qBound( 0.0, tension, 1.0 );
}

double QwtSplineCardinal::tension() const
{
    return d_tension;
}

QVector<double> QwtSplineCardinal::slopesX( const QPolygonF &points ) const
{
    const int size = points.size();
    if ( size <= 1 )
        return QVector<double>();

    const QPointF *p = points.constData();

    QVector<double> m( size );
    m[0] = ( p[1].y() - p[0].y() ) / ( p[1].x() - p[0].x() );

    const double c = 1.0 - d_tension;
    for ( int i = 1; i < size - 1; i++ )
    {
        const double slope = ( p[i+1].y() - p[i-1].y() ) / ( p[i+1].x() - p[i-1].x() );
        m[i] = c * slope;
    }

    m[size-1] = ( p[size-1].y() - p[size-2].y() ) / ( p[size-1].x() - p[size-2].x() );

    return m;
}
