/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline.h"
#include "qwt_math.h"
#include <qline.h>

// - bezier

static inline double qwtLineLength( const QPointF &p1, const QPointF &p2 )
{
   const double dx = p1.x() - p2.x();
   const double dy = p1.y() - p2.y();

   return qSqrt( dx * dx + dy * dy );
}

static inline QLineF qwtBezierControlLine(const QPointF &p0, const QPointF &p1, 
    const QPointF &p2, const QPointF &p3 )
{
    const double d02 = qwtLineLength(p0, p2);
    const double d13 = qwtLineLength(p1, p3);
    const double d12_2 = 0.5 * qwtLineLength(p1, p2);

    const bool b1 = ( d02 / 6.0 ) < d12_2;
    const bool b2 = ( d13 / 6.0 ) < d12_2;

    QPointF off1, off2;

    if( b1 )
    {
        if ( b2 )
        {
            // this is the normal case where both 1/6th 
            // vectors are less than half of d12_2

            const double s1 = ( p0 != p1 ) ? 6 : 3;
            off1 = ( p2 - p0 ) / s1;

            const double s2 = ( p2 != p3 ) ? 6 : 3;
            off2 = ( p1 - p3 ) / s2;
        }
        else
        {
            const double s = d12_2 / d13;

            off1 = ( p2 - p0 ) * s;
            off2 = ( p1 - p3 ) * s;
        }
    }
    else
    {
        if ( b2 )
        {
            // for this case d02/6 is more than half of d12_2, so
            // the d13/6 vector needs to be reduced

            const double s = d12_2 / d02;

            off1 = (p2 - p0) * s;
            off2 = (p1 - p3) * s;
        }
        else
        {
            off1 = (p2 - p0) * ( d12_2 / d02 );
            off2 = (p1 - p3) * ( d12_2 / d13 ); 
        }   
    }

    return QLineF( p1 + off1, p2 + off2 );
}

static inline QPointF qwtBezierPoint( const QLineF &ctrlLine, 
    const QPointF &p1, const QPointF &p2, double t )
{
    const double d1 = 3.0 * t;
    const double d2 = 3.0 * t * t;
    const double d3 = t * t * t;
    const double s  = 1.0 - t;

    const double x = (( s * p1.x() + d1 * ctrlLine.x1() ) * s + d2 * ctrlLine.x2() ) * s + d3 * p2.x();
    const double y = (( s * p1.y() + d1 * ctrlLine.y1() ) * s + d2 * ctrlLine.y2() ) * s + d3 * p2.y();

    return QPointF( x, y );
}

QPolygonF QwtSplineBezier::polygon( const QPolygonF& points, double dist )
{
    const int size = points.size();
    if ( size <= 2 || dist <= 0.0 )
        return points;

    const QPointF *p = points.constData();

    QLineF controlLine;

    QPolygonF fittedPoints;
    fittedPoints += p[0];

    for ( int i = 0; i < size - 1; i++ )
    {
        const double length = qwtLineLength( p[i], p[i + 1] );
        if ( dist < length )
        {
            if ( i == 0 )
            {
                controlLine = qwtBezierControlLine( p[0], p[0], p[1], p[2]);
            }
            else if ( i == points.size() - 2 )
            {
                controlLine = qwtBezierControlLine( p[size - 3], p[size - 2], p[size - 1], p[size - 1] );
            }
            else
            {
                controlLine = qwtBezierControlLine( p[i-1], p[i], p[i+1], p[i+2]);
            }

            const double off = dist / length;
            for( double t = off; t < 1.0; t += off ) 
            {
                fittedPoints += qwtBezierPoint( controlLine, p[i], p[i + 1], t );
            }
        }

        fittedPoints += p[i + 1];
    }

    return fittedPoints;
}

QLineF QwtSplineBezier::controlLine( 
    const QPointF &p0, const QPointF &p1,
    const QPointF &p2, const QPointF &p3 )
{
    return qwtBezierControlLine( p0, p1, p2, p3 );
}

QPointF QwtSplineBezier::point( const QLineF &controlLine, 
    const QPointF &p1, const QPointF &p2, double t )
{
    return qwtBezierPoint( controlLine, p1, p2, t );
}

static inline void qwtCubicTo( const QPointF *p, 
    int i1, int i2, int i3, int i4, QPainterPath &path )
{
    const QLineF l = qwtBezierControlLine( p[i1], p[i2], p[i3], p[i4]);
    path.cubicTo( l.p1(), l.p2(), p[i3] );
}

QPainterPath QwtSplineBezier::path( const QPolygonF &points, bool isClosed )
{
    const int size = points.size();

    QPainterPath path;
    if ( size == 0 )
        return path;

    const QPointF *p = points.constData();

    path.moveTo( p[0] );
    if ( size == 1 )
        return path;

    if ( size == 2 )
    {
        path.lineTo( p[1] );
    }
    else
    {
        if ( isClosed )
        {
            qwtCubicTo( p, size - 1, 0, 1, 2, path );
        }
        else
        {
            qwtCubicTo( p, 0, 0, 1, 2, path );
        }

        for ( int i = 1; i < size - 2; i++ )
            qwtCubicTo( p, i - 1, i, i + 1, i + 2, path );

        if ( isClosed )
        {
            qwtCubicTo( p, size - 3, size - 2, size - 1, 0, path );
            qwtCubicTo( p, size - 2, size - 1, 0, 1, path );
        }
        else
        {
            qwtCubicTo( p, size - 3, size - 2, size - 1, size - 1, path );
        }
    }

    return path;
}
