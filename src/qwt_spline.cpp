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

QwtSpline::QwtSpline()
{
}

QwtSpline::~QwtSpline()
{
}

QPainterPath QwtSpline::path( const QPolygonF &points ) const
{
    QPainterPath path;

    const int n = points.size();
    if ( n <= 2 )
    {
        path.addPolygon( points );
        return path;
    }

    const QVector<double> m = slopes( points );
    if ( m.size() != n )
        return path;

    const QPointF *pd = points.constData();
    const double *md = m.constData();

    path.moveTo( pd[0] );
    for ( int i = 0; i < n - 1; i++ )
        cubicTo( pd[i], md[i], pd[i+1], md[i+1], path );

    return path;
}

QPolygonF QwtSpline::polygon( int numPoints, const QPolygonF &points ) const
{
    if ( points.size() <= 2 )
        return points;

    QPolygonF fittedPoints;

    const QVector<double> m = slopes( points );
    if ( m.size() != points.size() )
        return fittedPoints;

    const QPointF *p = points.constData();
    const double *s = m.constData();

    const double x1 = points.first().x();
    const double x2 = points.last().x();

    const double delta = ( x2 - x1 ) / ( numPoints - 1 );

    double a, b, c, x0, y0;

    for ( int i = 0, j = 0; i < numPoints; i++ )
    {
        double x = x1 + i * delta;
        if ( x > x2 )
            x = x2;

        if ( i == 0 || x > p[j + 1].x() )
        {
            while ( x > p[j + 1].x() )
                j++;

            toPolynom( p[j], s[j], p[j + 1], s[j + 1], a, b, c );

            x0 = p[j].x();
            y0 = p[j].y();
        }

        const double y = qwtCubicPolynom( x - x0, a, b, c, y0 );
        fittedPoints += QPointF( x, y );
    }

    return fittedPoints;
}   
