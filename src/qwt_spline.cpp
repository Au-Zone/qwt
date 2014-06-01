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

static inline double qwtSlope( const QPointF &p1, const QPointF &p2 )
{
    const double dx = p2.x() - p1.x();
    return dx ? ( p2.y() - p1.y() ) / dx : 0.0;
}

static inline void qwtCubicTo( const QPointF &p1, double m1,
    const QPointF &p2, double m2, QPainterPath &path )
{
    const double dx = ( p2.x() - p1.x() ) / 3.0;

    path.cubicTo( p1.x() + dx, p1.y() + m1 * dx,
        p2.x() - dx, p2.y() - m2 * dx, 
        p2.x(), p2.y() );
}

static inline void qwtToCoefficients(
    const QPointF &p1, double m1,
    const QPointF &p2, double m2,
    double &a, double &b, double &c )
{
    const double dx = p2.x() - p1.x();
    const double dy = p2.y() - p1.y();

    const double slope = dy / dx;
   
    const double cv1 = 2 * ( 3 * slope - 2 * m1 - m2 ) / dx;
    const double cv2 = 2 * ( -3 * slope + m1 + 2 * m2 ) / dx;
    
    a = ( cv2 - cv1 ) / ( 6.0 * dx );
    b = 0.5 * cv1;
    c = slope - ( 0.5 * cv2 + cv1 ) * dx / 3.0;
}

static QPainterPath qwtSplinePath( const QPolygonF &points, const QVector<double> &m )
{
    QPainterPath path;

    const QPointF *p = points.constData();
    const int size = points.size();

    path.moveTo( p[0] );
    for ( int i = 0; i < size - 1; i++ )
    {
        const double dx = p[i+1].x() - p[i].x();

        path.cubicTo( p[i] + QPointF( dx, m[i] * dx ) / 3.0,
            p[i+1] - QPointF( dx, m[i+1] * dx ) / 3.0, p[i+1] );
    }

    return path;
}

static QPolygonF qwtSplinePolygon( 
    const QPolygonF &points, const QVector<double> m, int numPoints )
{
    const QPointF *p = points.constData();

    const double x1 = points.first().x();
    const double x2 = points.last().x();
    const double delta = ( x2 - x1 ) / ( numPoints - 1 );

    double a, b, c, x0, y0;

    QPolygonF fittedPoints;

    for ( int i = 0, j = 0; i < numPoints; i++ )
    {
        double x = x1 + i * delta;
        if ( x > x2 )
            x = x2;

        if ( i == 0 || x > p[j + 1].x() )
        {
            while ( x > p[j + 1].x() )
                j++;

            qwtToCoefficients( p[j], m[j], p[j + 1], m[j + 1], a, b, c );

            x0 = p[j].x();
            y0 = p[j].y();
        }

        const double y = qwtCubicPolynom( x - x0, a, b, c, y0 );
        fittedPoints += QPointF( x, y );
    }

    return fittedPoints;
}

// -- akima 

static inline double qwtAkima( double s1, double s2, double s3, double s4 )
{
    if ( ( s1 == s2 ) && ( s3 == s4 ) )
    {
        return 0.5 * ( s2 + s3 );
    }

    const double ds12 = qAbs( s2 - s1 );
    const double ds34 = qAbs( s4 - s3 );

    return ( s2 * ds34 + s3 * ds12 ) / ( ds12 + ds34 );
}

QPainterPath QwtSplineAkima::path( const QPolygonF &points )
{
    const int size = points.size();
    if ( size == 0 )
        return QPainterPath();

    if ( size == 1 )
    {
        QPainterPath path;
        path.moveTo( points[0] );
        return path;
    }

    const double m1 = qwtSlope( points[0], points[1] );
    const double m2 = qwtSlope( points[size-2], points[size-1] );

    return path( points, m1, m2 );
}

QPainterPath QwtSplineAkima::path( const QPolygonF &points, 
    double slopeStart, double slopeEnd )
{
    QPainterPath path;

    const int size = points.size();
    if ( size == 0 )
        return path;

    const QPointF *p = points.constData();
    path.moveTo( p[0] );

    if ( size == 1 )
        return path;

    if ( size == 2 )
    {
        qwtCubicTo( points[0], slopeStart, points[1], slopeEnd, path );
        return path;
    }

    double slope1 = slopeStart;
    double slope2 = qwtSlope( p[0], p[1] );
    double slope3 = qwtSlope( p[1], p[2] );

    double m1 = slope1;

    for ( int i = 0; i < size - 3; i++ )
    {
        const double slope4 = qwtSlope( p[i+2],  p[i+3] );

        const double m2 = qwtAkima( slope1, slope2, slope3, slope4 );
        qwtCubicTo( p[i], m1, p[i+1], m2, path );

        slope1 = slope2;
        slope2 = slope3;
        slope3 = slope4;

        m1 = m2;
    }

    const double m2 = qwtAkima( slope1, slope2, slope3, slopeEnd );

    qwtCubicTo( p[size - 3], m1, p[size - 2], m2, path );
    qwtCubicTo( p[size - 2], m2, p[size - 1], slopeEnd, path );

    return path;
}

// -- harmonic mean 

static inline double qwtHarmonicMean( 
    double dx1, double dy1, double dx2, double dy2 )
{
    if ( ( dy1 > 0.0 ) == ( dy2 > 0.0 ) )
    {
        if ( ( dy1 != 0.0 ) && ( dy2 != 0.0 ) )
            return 2.0 / ( dx1 / dy1 + dx2 / dy2 );
    }

    return 0.0;
}

static inline double qwtHarmonicMean( double s1, double s2 )
{
    if ( ( s1 > 0.0 ) == ( s2 > 0.0 ) )
    {
        if ( ( s1 != 0.0 ) && ( s2 != 0.0 ) )
            return 2 / ( 1 / s1 + 1 / s2 );
    }

    return 0.0;
}

QPainterPath QwtSplineHarmonicMean::path( const QPolygonF &points )
{
    const int size = points.size();
    if ( size == 0 )
        return QPainterPath();

    if ( size == 1 )
    {
        QPainterPath path;
        path.moveTo( points[0] );
        return path;
    }

    const double s1 = qwtSlope( points[0], points[1] );
    const double s2 = qwtSlope( points[1], points[2] );
    const double s3 = qwtSlope( points[size-3], points[size-2] );
    const double s4 = qwtSlope( points[size-2], points[size-1] );

    const double m1 = 1.5 * s1 - 0.5 * qwtHarmonicMean( s1, s2 );
    const double m2 = 1.5 * s4 - 0.5 * qwtHarmonicMean( s3, s4 );

    return path( points, m1, m2 );
}

QPainterPath QwtSplineHarmonicMean::path( const QPolygonF &points, 
    double slopeStart, double slopeEnd )
{
    QPainterPath path;

    const int size = points.size();
    if ( size == 0 )
        return path;

    const QPointF *p = points.constData();
    path.moveTo( p[0] );

    if ( size == 1 )
        return path;

    if ( size == 2 )
    {
        qwtCubicTo( points[0], slopeStart, points[1], slopeEnd, path );
        return path;
    }

    double dx1 = p[1].x() - p[0].x();
    double dy1 = p[1].y() - p[0].y();
    double m1 = slopeStart;

    for ( int i = 1; i < size - 1; i++ )
    {
        const double dx2 = p[i+1].x() - p[i].x();
        const double dy2 = p[i+1].y() - p[i].y();

        const double m2 = qwtHarmonicMean( dx1, dy1, dx2, dy2 );
        path.cubicTo( p[i-1] + QPointF( dx1, dx1 * m1 ) / 3.0,
            p[i] - QPointF( dx1, dx1 * m2 ) / 3.0, p[i] );

        dx1 = dx2;
        dy1 = dy2;
        m1 = m2;
    }

    path.cubicTo( p[size - 2] + QPointF( dx1, dx1 * m1 ) / 3.0,
        p[size - 1] - QPointF( dx1, dx1 * slopeEnd ) / 3.0, p[size - 1] );

    return path; 
}

static QVector<double> derivates_0( const QPolygonF &points )
{
    // not-a-knot-condition 

    const int size = points.size();

    const double rx1 = points[1].x() - points[0].x();
    const double ry1 = points[1].y() - points[0].y();

    const double rx2 = points[2].x() - points[1].x();
    const double ry2 = points[2].y() - points[1].y();

    const double rx3 = points[3].x() - points[2].x();
    const double ry3 = points[3].y() - points[2].y();

    const double rxEnd = points[size - 1].x() - points[size - 2].x();
    const double ryEnd = points[size - 1].y() - points[size - 2].y();

    if ( size == 3 )
    {
        /*
          the system is under-determined and we only 
          compute a quadratic spline.                   
         */

        const double slope1 = ry1 / rx1;
        const double slope2 = ry2 / rx2;

        const double c = ( slope2 - slope1 ) / ( rx1 + rx2 );

        QVector<double> m( 3 );
        m[0] = slope1 - rx1 * c;
        m[1] = slope2 - rx2 * c;
        m[2] = slope2 + rx2 * c; 

        return m;
    }

    // rr[0], tt[0] unsused
    QVector<double> rr( size - 2 );
    QVector<double> tt( size - 2 );

    tt[1] = rx1 + 2.0 * rx2;
    rr[1] = 3.0 * ( ry2 / rx2 - ry1 / rx1 ) * rx2 / ( rx1 + rx2 );

    const double v2 = rx2 / ( 2.0 * ( rx1 + rx2 ) );

    tt[2] = 2.0 * ( rx2 + rx3 ) - v2 * ( rx2 - rx1 );
    rr[2] = 3.0 * ( ry3 / rx3 - ry2 / rx2 ) - v2 * rr[1];

    double dx1 = rx3;
    double slope1 = ry3 / rx3;

    for ( int i = 3; i < size - 2; i++ )
    {
        const double dx2 = points[i+1].x() - points[i].x();
        const double dy2 = points[i+1].y() - points[i].y();
        const double slope2 = dy2 / dx2;

        const double v = dx1 / tt[i-1];

        tt[i] = 2.0 * ( dx1 + dx2 ) - v * dx1;
        rr[i] = 3.0 * ( slope2 - slope1 ) - v * rr[i-1];

        dx1 = dx2;
        slope1 = slope2;
    }

    // ---
    const double dx2 = rxEnd;
    const double slope2 = ryEnd / rxEnd;
    const double p0 = rr[size - 3] / tt[size - 3];

    double tt0 = 2.0 - ( dx1 - dx2 ) / tt[size - 3] + dx2 / dx1;
    double rr0 = 3.0 * ( slope2 - slope1 ) / ( dx2 + dx1 ) - ( p0 * ( dx1 - dx2 ) ) / dx1;

    const double c0 = rr0 / tt0;
    const double c1 = p0 - dx1 * c0 / tt[size-3];
    const double c2 = c0 + dx2 * ( c0 - c1 ) / dx1;

    QVector<double> m( size );

    m[size-1] = slope2 + dx2 * ( c0 + 2.0 * c2 ) / 3.0;
    m[size-2] = slope2 - dx2 * ( c2 + 2.0 * c0 ) / 3.0;
    m[size-3] = slope1 - dx1 * ( c0 + 2.0 * c1 ) / 3.0;

    double cv2 = c1;
    for ( int i = size - 4; i > 1; i-- )
    {
        const double dx = points[i+1].x() - points[i].x();
        const double dy = points[i+1].y() - points[i].y();

        const double cv1 = ( rr[i] - dx * cv2 ) / tt[i];
        m[i] = dy / dx - dx * ( cv2 + 2.0 * cv1 ) / 3.0;

        cv2 = cv1;
    }

    const double p1 = ( rr[1] - ( rx2 - rx1 ) * cv2 ) / tt[1];
    const double p2 = p1 + rx1 * ( p1 - cv2 ) / rx2;

    m[1] = ry2 / rx2 - rx2 * ( cv2 + 2.0 * p1 ) / 3.0;
    m[0] = ry1 / rx1 - rx1 * ( p1 + 2.0 * p2 ) / 3.0;

    return m;
}

static QVector<double> derivates_1( const QPolygonF &points, 
    double slopeBegin, double slopeEnd )
{
    /* first end point derivative given ? */

    const int size = points.size();

    const double rx1 = points[1].x() - points[0].x();
    const double ry1 = points[1].y() - points[0].y();

    const double rx2 = points[2].x() - points[1].x();
    const double ry2 = points[2].y() - points[1].y();

    if ( size == 3 )
    {
        const double slope1 = ry1 / rx1;
        const double slope2 = ry2 / rx2;
        const double s0 = slopeBegin - slopeEnd + 3.0 * ( slope2 - slope1 );

        QVector<double> m( 3 );
        m[0] = slopeBegin;
        m[1] = 1.5 * slope2 - 0.5 * ( slopeEnd + s0 * rx2 / ( rx1 + rx2 ) );
        m[2] = slopeEnd;

        return m;
    }

    const double rx3 = points[size - 2].x() - points[size - 3].x();
    const double ry3 = points[size - 2].y() - points[size - 3].y();

    const double rx4 = points[size - 1].x() - points[size - 2].x();
    const double ry4 = points[size - 1].y() - points[size - 2].y();

    // rr[0], tt[0] unsused
    QVector<double> rr( size - 2 );
    QVector<double> tt( size - 2 );

    tt[1] = 2.0 * ( rx1 + rx2 );
    rr[1] = 3.0 * ( ry2 / rx2 - ry1 / rx1 ) - 1.5 * ( ry1 / rx1 - slopeBegin );

    double dx1 = rx2;
    double slope1 = ry2 / rx2;

    for ( int i = 2; i < size - 2; i++ )
    {
        const double dx2 = points[i+1].x() - points[i].x();
        const double dy2 = points[i+1].y() - points[i].y();
        const double slope2 = dy2 / dx2;

        const double v = dx1 / tt[i-1];

        tt[i] = 2.0 * ( dx1 + dx2 ) - v * dx1;
        rr[i] = 3.0 * ( slope2 - slope1 ) - v * rr[i-1];

        dx1 = dx2;
        slope1 = slope2;
    }

    const double rr1 = 1.5 * ( ry4 / rx4 - slopeEnd ) - 3 * ry3 / rx3;

    const double ttx = 2.0 * rx3 + 1.5 * rx4 - rx3 * rx3 / tt[size - 3];
    const double rrx = rr1 - rx3 * rr[size - 3] / tt[size - 3];

    double cv2 = rrx / ttx;

    QVector<double> m( size );

    m[size-1] = slopeEnd;
    m[size-2] = 1.5 * ry4 / rx4 - 0.5 * ( cv2 * rx4 + slopeEnd );

    for ( int i = size - 3; i > 1; i-- )
    {
        const double dx = points[i+1].x() - points[i].x();
        const double dy = points[i+1].y() - points[i].y();

        const double cv1 = ( rr[i] - dx * cv2 ) / tt[i];
        m[i] = dy / dx - dx * ( cv2 + 2.0 * cv1 ) / 3.0;

        cv2 = cv1;
    }

    const double cv1 = ( rr[1] - rx2 * cv2 ) / ( 1.5 * rx1 + 2.0 * rx2 );
    m[1] = ry2 / rx2 - rx2 * ( cv2 + 2.0 * cv1 ) / 3.0;
    m[0] = slopeBegin;

    return m;
}

static QVector<double> derivates_2( const QPolygonF &points, 
    double cvStart, double cvEnd )
{
    const int size = points.size();

    const double rx1 = points[1].x() - points[0].x();
    const double ry1 = points[1].y() - points[0].y();

    const double rx2 = points[2].x() - points[1].x();
    const double ry2 = points[2].y() - points[1].y();

    const double rxEnd = points[size - 1].x() - points[size - 2].x();
    const double ryEnd = points[size - 1].y() - points[size - 2].y();

    // rr[0], tt[0] unsused
    QVector<double> rr( size - 1 );
    QVector<double> tt( size - 1 );

    tt[1] = 2.0 * ( rx1 + rx2 );
    rr[1] = 3.0 * ( ry2 / rx2 - ry1 / rx1 ) - rx1 * 0.5 * cvStart;

    double dx1 = rx2;
    double slope1 = ry2 / rx2;

    for ( int i = 2; i < size - 1; i++ )
    {
        const double dx2 = points[i+1].x() - points[i].x();
        const double dy2 = points[i+1].y() - points[i].y();
        const double slope2 = dy2 / dx2;

        const double v = dx1 / tt[i-1];

        tt[i] = 2.0 * ( dx1 + dx2 ) - v * dx1;
        rr[i] = 3.0 * ( slope2 - slope1 ) - v * rr[i-1];

        dx1 = dx2;
        slope1 = slope2;
    }

    // --
    const double cv3 = 0.5 * cvEnd;

    double cv2 = ( rr[size - 2] - rxEnd * cv3 ) / tt[size - 2];

    QVector<double> m( size );
    m[size-2] = ryEnd / rxEnd - rxEnd * ( cv3 + 2.0 * cv2 ) / 3.0;
    m[size-1] = m[size-2] + ( cv3 + cv2 ) * rxEnd;

    for ( int i = size - 3; i > 0; i-- )
    {
        const double dx = points[i+1].x() - points[i].x();
        const double dy = points[i+1].y() - points[i].y();

        double cv1 = ( rr[i] - dx * cv2 ) / tt[i];
        m[i] = dy / dx - dx * ( cv2 + 2.0 * cv1 ) / 3.0;

        cv2 = cv1;
    }

    m[0] = ry1 / rx1 - rx1 * ( cv2 + cvStart ) / 3.0;

    return m;
}

static QVector<double> derivates_3( const QPolygonF &points, 
    double marg_0, double marg_n )
{
    /* third derivative at end point ?    */

    const double rx1 = points[1].x() - points[0].x();
    const double ry1 = points[1].y() - points[0].y();

    const double rx2 = points[2].x() - points[1].x();
    const double ry2 = points[2].y() - points[1].y();

    const int size = points.size();
    if ( size == 3 )
    {
        const double slope1 = ry1 / rx1;
        const double slope2 = ry2 / rx2;

        const double m0 = marg_0 * rx1;
        const double m1 = marg_n * rx2;

        double v = 3.0 * ( slope2 - slope1 ) + 0.5 * ( m0 * rx1 - m1 * rx2 );

        const double c2 = v / ( rx1 + rx2 );

        QVector<double> m( 3 );
        m[0] = slope1 - rx1 * ( c2 - m0 ) / 3.0;
        m[1] = slope2 - rx2 * ( c2 + 0.5 * m1 ) / 3.0;
        m[2] = slope2 + rx2 * ( c2 + m1 ) / 3.0; 

        return m;
    }

    const double rx3 = points[size - 2].x() - points[size - 3].x();
    const double ry3 = points[size - 2].y() - points[size - 3].y();

    const double rx4 = points[size - 1].x() - points[size - 2].x();
    const double ry4 = points[size - 1].y() - points[size - 2].y();

    // rr[0], tt[0] unsused
    QVector<double> rr( size - 2 );
    QVector<double> tt( size - 2 );

    tt[1] = 2.0 * ( rx1 + rx2 );
    rr[1] = 3.0 * ( ry2 / rx2 - ry1 / rx1 ) + 0.5 * marg_0 * rx1 * rx1;

    double dx1 = rx2;
    double s1 = ry2 / rx2;

    for ( int i = 2; i < size - 2; i++ )
    {
        const double dx2 = points[i+1].x() - points[i].x();
        const double dy2 = points[i+1].y() - points[i].y();
        const double s2 = dy2 / dx2;

        const double v = dx1 / tt[i-1];

        tt[i] = 2.0 * ( dx1 + dx2 ) - v * dx1;
        rr[i] = 3.0 * ( s2 - s1 ) - v * rr[i-1];

        dx1 = dx2;
        s1 = s2;
    }

    QVector<double> m( size );

    const double r0 = 3.0 * ( ry4 / rx4 - ry3 / rx3 ) - 0.5 * marg_n * rx4 * rx4;
    const double rrx = r0 - rx3 * rr[size - 3] / tt[size - 3];
    const double ttx = 3.0 * rx4 + 2.0 * rx3 - rx3 * rx3 / tt[size - 3];

    double cv2 = rrx / ttx;
    const double cv0 = cv2 + marg_n * 0.5 * rx4;

    m[size - 1] = ry4 / rx4 + rx4 * ( cv2 + 2.0 * cv0 ) / 3.0;
    m[size - 2] = ry4 / rx4 - rx4 * ( cv0 + 2.0 * cv2 ) / 3.0;

    for ( int i = size - 3; i > 1; i-- )
    {
        const double dx = points[i+1].x() - points[i].x();
        const double dy = points[i+1].y() - points[i].y();

        const double cv1 = ( rr[i] - dx * cv2 ) / tt[i];
        m[i] = dy / dx - dx * ( cv2 + 2.0 * cv1 ) / 3.0;

        cv2 = cv1;
    }

    const double cv7 = ( rr[1] - rx2 * cv2 ) / ( 3.0 * rx1 + 2.0 * rx2 );

    m[1] = ry2 / rx2 - rx2 * ( cv2 + 2.0 * cv7 ) / 3.0;
    m[0] = ry1 / rx1 - rx1 * ( cv7 - marg_0 * rx1 / 3.0 );

    return m;
}

QVector<double> QwtSplineCubic::derivatives( 
    const QPolygonF &points, EndpointCondition condition )
{
    if ( points.size() <= 2 )
        return QVector<double>();

    QVector<double> m;
    switch( condition )
    {
        case Natural:
            m = derivates_2( points, 0.0, 0.0 );
            break;
        case NotAKnot:
            m = derivates_0( points );
            break;
        default:
            m = derivates_3( points, 0.0001, 0.0001 );
    }

    return m;
}

QVector<double> QwtSplineCubic::derivatives( 
    const QPolygonF &points, double slope1, double slope2 )
{
    if ( points.size() <= 2 )
        return QVector<double>();
    
    return derivates_1( points, slope1, slope2 );
}   

QPainterPath QwtSplineCubic::path( 
    const QPolygonF &points, EndpointCondition endpointCondition )
{
    if ( points.size() <= 2 )
    {
        QPainterPath path;
        path.addPolygon( points );
        return path;
    }

    const QVector<double> m = derivatives( points, endpointCondition );
    return qwtSplinePath( points, m );
}

QPainterPath QwtSplineCubic::path( const QPolygonF &points, 
    double slopeBegin, double slopeEnd )
{
    if ( points.size() <= 2 )
    {
        QPainterPath path;
        path.addPolygon( points );
        return path;
    }   
        
    const QVector<double> m = derivatives( points, slopeBegin, slopeEnd );
    return qwtSplinePath( points, m );
}   

QPolygonF QwtSplineCubic::polygon( int numPoints,
    const QPolygonF &points, EndpointCondition endpointCondition )
{
    if ( points.size() <= 2 )
        return points;

    const QVector<double> m = derivatives( points, endpointCondition );
    return qwtSplinePolygon( points, m, numPoints );
}   

QPolygonF QwtSplineCubic::polygon( int numPoints,
    const QPolygonF &points, double slopeBegin, double slopeEnd )
{
    if ( points.size() <= 2 )
        return points;

    const QVector<double> m = derivatives( points, slopeBegin, slopeEnd );
    return qwtSplinePolygon( points, m, numPoints );
}

