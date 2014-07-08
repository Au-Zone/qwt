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
#include <qpainterpath.h>

class QWT_EXPORT QwtSpline
{
public:
    QwtSpline();
    virtual ~QwtSpline();

    virtual QPainterPath path( const QPolygonF & ) const;
    virtual QPolygonF polygon( int numPoints, const QPolygonF & ) const;

    virtual QVector<double> slopes( const QPolygonF & ) const = 0;

    static void toPolynom( const QPointF &p1, double m1,
        const QPointF &p2, double m2, double &a, double &b, double &c );

    static void toSlopes( const QPointF &p1, const QPointF &p2,
        double a, double b, double c, double &m1, double &m2 );

    static void toSlopes( double dx,
        double a, double b, double c, double &m1, double &m2 );

    static inline void cubicTo( const QPointF &p1, double m1,
        const QPointF &p2, double m2, QPainterPath &path );
};

inline void QwtSpline::toPolynom(
    const QPointF &p1, double m1,
    const QPointF &p2, double m2,
    double &a, double &b, double &c )
{
    const double dx = p2.x() - p1.x();
    const double slope = ( p2.y() - p1.y() ) / dx;

    c = m1;
    b = ( 3.0 * slope - 2 * m1 - m2 ) / dx;
    a = ( ( m2 - m1 ) / dx - 2.0 * b ) / ( 3.0 * dx );
}

inline void QwtSpline::toSlopes( const QPointF &p1, const QPointF &p2,
    double a, double b, double c, double &m1, double &m2 )
{   
    return toSlopes( p2.x() - p1.x(), a, b, c, m1, m2 ); 
}

inline void QwtSpline::toSlopes( double dx, 
    double a, double b, double c, double &m1, double &m2 )
{   
    m1 = c; 
    m2 = ( ( 3.0 * a * dx ) + 2.0 * b ) * dx + c;
} 

inline void QwtSpline::cubicTo( const QPointF &p1, double m1,
    const QPointF &p2, double m2, QPainterPath &path )
{   
    const double dx = ( p2.x() - p1.x() ) / 3.0;

    path.cubicTo( p1.x() + dx, p1.y() + m1 * dx,
        p2.x() - dx, p2.y() - m2 * dx,
        p2.x(), p2.y() );
}

#endif
