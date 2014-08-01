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

static inline double qwtChordal( const QPointF &p1, const QPointF &p2 )
{
   const double dx = p1.x() - p2.x();
   const double dy = p1.y() - p2.y();

   return qSqrt( dx * dx + dy * dy );
}

static inline void qwtCubicTo( const QPointF &p1, double m1,
    const QPointF &p2, double m2, QPainterPath &path )
{   
    const double dx = ( p2.x() - p1.x() ) / 3.0;
    
    path.cubicTo( p1.x() + dx, p1.y() + m1 * dx,
        p2.x() - dx, p2.y() - m2 * dx,
        p2.x(), p2.y() );
}

#if 0

static inline void qwtToCurvatures(
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

#endif

QwtSpline::QwtSpline():
    d_parametrization( QwtSpline::ParametrizationX )
{
}

QwtSpline::~QwtSpline()
{
}

void QwtSpline::setParametrization( Parametrization parametrization )
{
    d_parametrization = parametrization;
}   

QwtSpline::Parametrization QwtSpline::parametrization() const
{
    return d_parametrization;
}

QPainterPath QwtSpline::pathP( const QPolygonF &points ) const
{
    QPainterPath path;

    switch( d_parametrization )
    {
        case ParametrizationX:
            path = pathX( points );
            break;
        case ParametrizationChordal:
            path = pathChordal( points );
            break;
    }

    return path;
}

QwtSplineG1::QwtSplineG1()
{
}

QwtSplineG1::~QwtSplineG1()
{
}

QwtSplineC1::QwtSplineC1()
{
}

QwtSplineC1::~QwtSplineC1()
{
}

QPainterPath QwtSplineC1::pathX( const QPolygonF &points ) const
{
    QPainterPath path;

    const int n = points.size();
    if ( n <= 2 )
    {
        path.addPolygon( points );
        return path;
    }

    const QVector<double> m = slopesX( points );
    if ( m.size() != n )
        return path;

    const QPointF *pd = points.constData();
    const double *md = m.constData();

    path.moveTo( pd[0] );
    for ( int i = 0; i < n - 1; i++ )
        qwtCubicTo( pd[i], md[i], pd[i+1], md[i+1], path );

    return path;
}

QPolygonF QwtSplineC1::polygonX( int numPoints, const QPolygonF &points ) const
{
    if ( points.size() <= 2 )
        return points;

    QPolygonF fittedPoints;

    const QVector<double> m = slopesX( points );
    if ( m.size() != points.size() )
        return fittedPoints;

    const QPointF *p = points.constData();
    const double *s = m.constData();

    const double x1 = points.first().x();
    const double x2 = points.last().x();

    const double delta = ( x2 - x1 ) / ( numPoints - 1 );

    double x0, y0;
    QwtSplinePolynom polynom;

    for ( int i = 0, j = 0; i < numPoints; i++ )
    {
        double x = x1 + i * delta;
        if ( x > x2 )
            x = x2;

        if ( i == 0 || x > p[j + 1].x() )
        {
            while ( x > p[j + 1].x() )
                j++;

            polynom = QwtSplinePolynom::fromSlopes( p[j], s[j], p[j + 1], s[j + 1] );

            x0 = p[j].x();
            y0 = p[j].y();
        }

        const double y = y0 + polynom.value( x - x0 );
        fittedPoints += QPointF( x, y );
    }

    return fittedPoints;
}   

QVector<QwtSplinePolynom> QwtSplineC1::polynomsX( const QPolygonF &points ) const
{
    QVector<QwtSplinePolynom> polynoms;

    const QVector<double> m = slopesX( points );
    if ( m.size() < 2 )
        return polynoms;

    for ( int i = 1; i < m.size(); i++ )
    {
        polynoms += QwtSplinePolynom::fromSlopes( 
            points[i-1], m[i-1], points[i], m[i] );
    }

    return polynoms;
}

QPainterPath QwtSplineC1::pathChordal( const QPolygonF &points ) const
{
    QPainterPath path;

    if ( points.size() <= 2 )
        return path;

    QPolygonF px, py;

    px += QPointF( 0.0, points[0].x() );
    py += QPointF( 0.0, points[0].y() );

    double t = 0.0;
    for ( int i = 1; i < points.size(); i++ )
    {
        t += qwtChordal( points[i-1], points[i] );

        px += QPointF( t, points[i].x() );
        py += QPointF( t, points[i].y() );
    }

    const QVector<double> mx = slopesX( px );
    const QVector<double> my = slopesX( py );

    path.moveTo( points[0] );
    for ( int i = 1; i < points.size(); i++ )
    {
        const double t3 = qwtChordal( points[i-1], points[i] ) / 3.0;

        const double cx1 = points[i-1].x() + mx[i-1] * t3;
        const double cy1 = points[i-1].y() + my[i-1] * t3;

        const double cx2 = points[i].x() - mx[i] * t3;
        const double cy2 = points[i].y() - my[i] * t3;

        path.cubicTo( cx1, cy1, cx2, cy2, points[i].x(), points[i].y() );
    }

    return path;
}

QwtSplineC2::QwtSplineC2()
{
}

QwtSplineC2::~QwtSplineC2()
{
}
