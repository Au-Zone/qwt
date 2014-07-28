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

static inline void qwtTensions(
    double d13, double d23, double d24,
    const QPointF &p1, const QPointF &p2, 
    const QPointF &p3, const QPointF &p4, 
    double &s1, double &s2 )
{
    const bool b1 = ( d13 / 3.0 ) < d23;
    const bool b2 = ( d24 / 3.0 ) < d23;

    if ( b1 & b2 )
    {
        s1 = ( p1 != p2 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
        s2 = ( p3 != p4 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
    }
    else
    {
        s1 = d23 / ( b1 ? d24 : d13 );
        s2 = d23 / ( b2 ? d13 : d24 );
    }
}

static inline void qwtBezierInterpolate(
    double d13, double d23, double d24,
    const QPointF &p1, const QPointF &p2,
    const QPointF &p3, const QPointF &p4,
    double &s1, double &s2 )
{
    const bool b1 = ( d13 / 3.0 ) < d23;
    const bool b2 = ( d24 / 3.0 ) < d23;

    if ( b1 & b2 )
    {
        s1 = ( p1 != p2 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
        s2 = ( p3 != p4 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
    }
    else
    {
        s1 = d23 / ( b1 ? d24 : d13 );
        s2 = d23 / ( b2 ? d13 : d24 );
    }
}

static inline void qwtCubicTo( const QPointF &p1, const QPointF &p2, 
    const QPointF &p3, const QPointF &p4, QPainterPath &path )
{
    const double d13 = qwtChordal(p1, p3);
    const double d24 = qwtChordal(p2, p4);
    const double d23 = qwtChordal(p2, p3);

    double s1, s2;
    qwtBezierInterpolate( d13, d23, d24, p1, p2, p3, p4, s1, s2 );

    const QPointF cp1 = p2 + 0.5 * ( p3 - p1 ) * s1;
    const QPointF cp2 = p3 - 0.5 * ( p4 - p2 ) * s2;

    path.cubicTo( cp1, cp2, p3 );
}

#if 0

void myPathTo( const QPointF &p1, const QPointF &cp1, 
    const QPointF &cp2, const QPointF &p2, QPainterPath &path )
{
    const double x1 = cp1.x() - p1.x();
    const double x2 = cp2.x() - p1.x();
    const double dx = p2.x() - p1.x();

    const double y1 = cp1.y() - p1.y();
    const double y2 = cp2.y() - p1.y();
    const double dy = p2.y() - p1.y();

    const double t = qwtLineLength( p1, p2 );

    const QwtSplinePolynom pX( ( dx - 3 * x2 + 3 * x1 ) / ( t * t * t ),
        ( 3 * x2 - 6 * x1 ) / ( t * t ), 3 * x1 / t );

    const QwtSplinePolynom pY( ( dy - 3 * y2 + 3 * y1 ) / ( t * t * t ),
        ( 3 * y2 - 6 * y1 ) / ( t * t ), 3 * y1 / t );
    
    const double mX1 = pX.slope( 0.0 );
    const double mX2 = pX.slope( t );
    const double mY1 = pY.slope( 0.0 );
    const double mY2 = pY.slope( t );

    {
        const double t3 = t / 3.0;

        const double cx1 = p1.x() + mX1 * t3;
        const double cy1 = p1.y() + mY1 * t3;

        const double cx2 = p2.x() - mX2 * t3;
        const double cy2 = p2.y() - mY2 * t3;

        path.cubicTo( cx1, cy1, cx2, cy2, p2.x(), p2.y() );
    }
}

#endif

QwtSplineCardinal::QwtSplineCardinal()
{
}

QwtSplineCardinal::~QwtSplineCardinal()
{
}

QPainterPath QwtSplineCardinal::parametricPath( const QPolygonF &points ) const
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
        return path;
    }

    qwtCubicTo( p[0], p[0], p[1], p[2], path );

    double d13 = qwtChordal(p[0], p[2]);

    double mx1 = ( p[2].x() - p[0].x() ) * 0.5;
    double my1 = ( p[2].y() - p[0].y() ) * 0.5;

    for ( int i = 1; i < size - 2; i++ )
    {
        const double d24 = qwtChordal(p[i], p[i+2]);

        double t1, t2;
        {
            const double d = 3.0 * qwtChordal(p[i], p[i+1]);
            if ( d13 < d )
            {
                if ( d24 < d )
                {
                    t1 = t2 = 1.0;

                    if ( p[i-1] == p[i] )
                        t1 = 0.5;

                    if ( p[i+1] == p[i+2] )
                        t2 = 0.5;
                }
                else
                {
                    t1 = t2 = d / d24;
                }
            }
            else
            {
                if ( d24 < d )
                {
                    t1 = t2 = d / d13;
                }
                else
                {
                    // d13 > 3 * d23 && d24 > 3 * d23
                    t1 = d / d13;
                    t2 = d / d24;
                }
            }
        }

        const double mx2 = ( p[i+2].x() - p[i].x() ) * 0.5;
        const double my2 = ( p[i+2].y() - p[i].y() ) * 0.5;

        const double cx1 = p[i].x() + mx1 * t1 / 3.0;
        const double cx2 = p[i+1].x() - mx2 * t2 / 3.0;

        const double cy1 = p[i].y() + my1 * t1 / 3.0;
        const double cy2 = p[i+1].y() - my2 * t2 / 3.0;

        path.cubicTo( cx1, cy1, cx2, cy2, p[i+1].x(), p[i+1].y() );

        d13 = d24;
        mx1 = mx2;
        my1 = my2;
    }

    qwtCubicTo( p[size - 3], p[size - 2], p[size - 1], p[size - 1], path );

    return path;
}

QPainterPath QwtSplineCardinal::path( const QPolygonF & ) const
{
    return QPainterPath();
}

QPolygonF QwtSplineCardinal::polygon( int numPoints, const QPolygonF & ) const
{
    Q_UNUSED(numPoints)

    return QPolygonF();
}

QVector<QwtSplinePolynom> QwtSplineCardinal::polynoms( const QPolygonF & ) const
{
    return QVector<QwtSplinePolynom>();
}

