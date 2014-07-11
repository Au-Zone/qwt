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
#include "qwt_spline_polynom.h"
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

    static QwtSplinePolynom toPolynom2( const QPointF &p1, double cv1,
        const QPointF &p2, double cv2 );

    static void toSlopes( const QPointF &p1, const QPointF &p2,
        const QwtSplinePolynom &, double &m1, double &m2 );

    static void toSlopes( double dx,
        const QwtSplinePolynom &, double &m1, double &m2 );

    static void toCurvatures( const QPointF &p1, const QPointF &p2,
        double a, double b, double &cv1, double &cv2 );

    static void toCurvatures( double dx,
        double a, double b, double &cv1, double &cv2 );

    static void toCurvatures2( const QPointF &p1, double m1,
        const QPointF &p2, double m2, double &cv1, double &cv2 );

    static inline void cubicTo( const QPointF &p1, double m1,
        const QPointF &p2, double m2, QPainterPath &path );
};

inline void QwtSpline::toSlopes( const QPointF &p1, const QPointF &p2,
    const QwtSplinePolynom &polynom, double &m1, double &m2 )
{   
    return toSlopes( p2.x() - p1.x(), polynom, m1, m2 ); 
}

inline void QwtSpline::toSlopes( double dx, 
    const QwtSplinePolynom &polynom, double &m1, double &m2 )
{   
    m1 = polynom.c; 
    m2 = polynom.slope( dx );
} 

inline void QwtSpline::toCurvatures(
    const QPointF &p1, const QPointF &p2,
    double a, double b, double &cv1, double &cv2 )
{   
    toCurvatures( p2.x() - p1.x(), a, b, cv1, cv2 );
}   

inline void QwtSpline::toCurvatures( double dx,
    double a, double b, double &cv1, double &cv2 )
{
    cv1 = 2.0 * b;
    cv2 = 2.0 * ( 3.0 * a * dx + b );
}

inline void QwtSpline::toCurvatures2(
    const QPointF &p1, double m1, const QPointF &p2, double m2,
    double &cv1, double &cv2 )
{   
    const double dx = p2.x() - p1.x();
    const double dy = p2.y() - p1.y();
    
    const double v = 3 * dy / dx - m1 - m2;
    const double k = 2.0 / dx;
    
    cv1 = k * ( v - m1 );
    cv2 = k * ( m2 - v );
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
