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

#define TEST_SPLINE 1

#if TEST_SPLINE
#include <QDebug>
#endif

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

static inline void qwtToHermite( double dx, double a, double b, double c, 
    double &m1, double &m2 )
{
    m1 = c;
    m2 = ( ( 3.0 * a * dx ) + 2.0 * b ) * dx + c;
}

static inline void qwtToHermite( const QPointF &p1, const QPointF &p2,
    double a, double b, double c, double &m1, double &m2 )
{
    return qwtToHermite( p2.x() - p1.x(), a, b, c, m1, m2 );
}

#if 0
static inline void qwtToHermite2( 
    const QPointF &p1, double cv1, 
    const QPointF &p2, double cv2,
    double &m1, double &m2 )
{
    const double dx = p2.x() - p1.x();
    const double slope = ( p2.y() - p1.y() ) / dx;

    m1 = slope - 0.5 * ( cv1 + ( cv2 - cv1 ) / 3.0 ) * dx;
    m2 = m1 + 0.5 * ( cv2 + cv1 ) * dx;
}

#endif

#if 0
static double qwtDerivateEnd( 
    const QPointF &p1, const QPointF &p2, double m, double cv )
{
    const double dx = p2.x() - p1.x();
    const double slope = ( p2.y() - p1.y() ) / dx;

    return 3.0 * ( slope - cv * dx - m ) + cv * dx + m;
}
#endif

static inline void qwtToCoefficients(
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

static inline void qwtToCoefficients2(
    const QPointF &p1, double cv1,
    const QPointF &p2, double cv2,
    double &a, double &b, double &c )
{
    const double dx = p2.x() - p1.x();
    const double slope = ( p2.y() - p1.y() ) / dx;

    a = ( cv2 - cv1 ) / ( 6.0 * dx ); 
    b = 0.5 * cv1;
    c = slope - ( a * dx + b ) * dx;
}

static inline void qwtToHermite3( const QPointF &p1, const QPointF &p2,
    double cv1, double cv2, double &m2 )
{
    const double dx = p2.x() - p1.x();
    const double slope = ( p2.y() - p1.y() ) / dx;

    m2 = slope + ( cv2 + ( cv1 - cv2 ) / 6.0 ) * dx;
}

static inline void qwtToCurvature( const QPointF &p1, const QPointF &p2,
    double a, double b, double c, double &cv1, double &cv2 )
{
    const double dx = p2.x() - p1.x();
    const double slope = ( p2.y() - p1.y() ) / dx;

    const double m1 = c;
    const double m2 = ( ( 3.0 * a * dx ) + b ) * dx + c;

    cv1 = 2.0 * ( 3 * slope - 2 * m1 - m2 ) / dx;
    cv2 = 2.0 * ( -3 * slope + m1 + 2 * m2 ) / dx;
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

// -- cubic splines

#if TEST_SPLINE

static bool testNodes( const QPolygonF &p, const QVector<double> &m )
{
    const int size = p.size();

    bool ok = true;

    for ( int i = 1; i <= size - 2; i++ )
    {
        double a1, b1, c1;
        double a2, b2, c2;

        qwtToCoefficients( p[i-1], m[i-1], p[i], m[i], a1, b1, c1 );
        qwtToCoefficients( p[i], m[i], p[i+1], m[i+1], a2, b2, c2 );

        if ( !qFuzzyCompare( 3.0 * a1 * ( p[i].x() - p[i-1].x() ) + b1, b2 ) )
        {
            //qDebug() << 3.0 * a1 * ( p[i].x() - p[i-1].x() ) + b1 << b2;
            //qDebug() << "invalid node condition" << i;
            ok = false;
        }
    }

    return ok;
}

static bool test_0( const QPolygonF &p, const QVector<double> &m )
{
    bool ok = true;

    const int size = p.size();

    double a1, b1, c1;
    double a2, b2, c2;

    qwtToCoefficients( p[0], m[0], p[1], m[1], a1, b1, c1 );
    qwtToCoefficients( p[1], m[1], p[2], m[2], a2, b2, c2 );

    if ( !qFuzzyCompare( a1, a2 ) )
    {
        ok = false;
    }

    qwtToCoefficients( p[size-3], m[size-3], p[size-2], m[size-2], a1, b1, c1 );
    qwtToCoefficients( p[size-2], m[size-2], p[size-1], m[size-1], a2, b2, c2 );

    if ( !qFuzzyCompare( a1, a2 ) )
    {
        ok = false;
    } 

    if ( !testNodes( p, m ) )
        ok = false;

    return ok;
}

static bool test_1( const QPolygonF &p, const QVector<double> &m,
    double slopeBegin, double slopeEnd )
{
    bool ok = true;
    
    if ( !qFuzzyCompare( m.first(), slopeBegin ) )
    {
        ok = false;
    }

    if ( !qFuzzyCompare( m.last(), slopeEnd ) )
    {
        ok = false;
    }

    if ( !testNodes( p, m ) )
        ok = false;
    
    return ok;
}

static bool test_2( const QPolygonF &p, const QVector<double> &m,
    double cvStart, double cvEnd )
{
    bool ok = true;
    {
        const double dx = p[1].x() - p[0].x();
        const double dy = p[1].y() - p[0].y();

        const double cv = 2 * ( 3 * dy / dx - 2 * m[0] - m[1] ) / dx;
        if ( !qFuzzyCompare( cvStart, cv ) )
        {
            ok = false; 
        }   
    }

    {
        const int size = p.size();
        const double dx = p[size-1].x() - p[size-2].x();
        const double dy = p[size-1].y() - p[size-2].y();
        const double cv = 2 * ( -3 * dy / dx + m[size-2] + 2 * m[size-1] ) / dx;

        if ( !qFuzzyCompare( cvEnd, cv ) )
        {
            ok = false;
        }
    }

    if ( !testNodes( p, m ) )
        ok = false;

    return ok;
}

static bool test_3( const QPolygonF &p, const QVector<double> &m,
    double marg_0, double marg_n )
{
    bool ok = true;
    {
        const double dx = p[1].x() - p[0].x();
        const double dy = p[1].y() - p[0].y();

        const double slope = dy / dx;

        const double cv1 = 2 * ( 3 * slope - 2 * m[0] - m[1] ) / dx;
        const double cv2 = 2 * ( -3 * slope + m[0] + 2 * m[1] ) / dx;

        if ( !qFuzzyCompare( marg_0, ( cv2 - cv1 ) / dx ) )
        {
            ok = false; 
        }
    }

    {
        const int size = p.size();
        const double dx = p[size-1].x() - p[size-2].x();
        const double dy = p[size-1].y() - p[size-2].y();

        const double slope = dy / dx;

        const double cv1 = 2 * ( 3 * slope - 2 * m[size-2] - m[size-1] ) / dx;
        const double cv2 = 2 * ( -3 * slope + m[size-2] + 2 * m[size-1] ) / dx;

        if ( !qFuzzyCompare( marg_n, ( cv2 - cv1 ) / dx ) )
        {
            ok = false; 
        }
    }

    if ( !testNodes( p, m ) )
        ok = false;

    return ok;
}

static bool test_4( const QPolygonF &p, const QVector<double> &m )
{
    double a1, b1, c1;
    double a2, b2, c2;

    const int n = p.size();
    qwtToCoefficients( p[n-2], m[n-2], p[n-1], m[n-1], a1, b1, c1 );
    qwtToCoefficients( p[0], m[0], p[1], m[1], a2, b2, c2 );

    bool ok = true;

    const double dx = p[n-1].x() - p[n-2].x();
    if ( !qFuzzyCompare( 6.0 * a1 * dx + 2 * b1, 2 * b2 ) )
    {
        ok = false;
    }

    if ( !qFuzzyCompare( 3 * a1 * dx * dx + 2 * b1 * dx + c1, m[0] ) )
    {
        ok = false;
    }

    if ( !testNodes( p, m ) )
        ok = false;

    return ok;
}

#endif // TEST_SPLINE

static QVector<double> qwtDerivatives0( const QPolygonF &p )
{
    /* not-a-node condition ?             */

    const int n = p.size();

    const double h0 = p[1].x() - p[0].x();
    const double h1 = p[2].x() - p[1].x();

    const double s0 = ( p[1].y() - p[0].y() ) / h0;
    const double s1 = ( p[2].y() - p[1].y() ) / h1;

    if ( n == 3 )
    {
        /*
          the system is under-determined and we only 
          compute a quadratic spline.                   
         */

        const double c = ( s1 - s0 ) / ( h0 + h1 );

        QVector<double> m( 3 );
        m[0] = s0 - h0 * c;
        m[1] = s1 - h1 * c;
        m[2] = s1 + h1 * c;

#if TEST_SPLINE
        qDebug() << "COMPARE 0: under-determined";
#endif

        return m;
    }

    if ( n == 4 )
    {
        // TODO ...

        return QVector<double>( 4, 0.0 );
    }

    const double h2 = p[3].x() - p[2].x();

    const double h4 = ( p[n-2].x() - p[n-3].x() );
    const double h5 = ( p[n-1].x() - p[n-2].x() );

    const double s2 = ( p[3].y() - p[2].y() ) / h2;

    const double s3 = ( p[n-2].y() - p[n-3].y() ) / h4;
    const double s4 = ( p[n-1].y() - p[n-2].y() ) / h5;

    QVector<double> w(n-3);
    QVector<double> v(n-3);

    w[0] = h0 + 2.0 * h1;
    v[0] = 3.0 * ( s1 - s0 ) * h1 / ( h0 + h1 );

    w[1] = 2.0 * ( h1 + h2 ) - h1 / w[0] * ( h1 - h0 );
    v[1] = 3.0 * ( s2 - s1 ) - h1 / w[0] * v[0];

    double dx1 = h2;
    double slope1 = s2;

    for ( int i = 2; i < n - 3; i++ )
    {
        const double dx2 = p[i+2].x() - p[i+1].x();
        const double slope2 = ( p[i+2].y() - p[i+1].y() ) / dx2;

        v[i] = 3.0 * ( slope2 - slope1 ) - dx1 / w[i-1] * v[i-1];
        w[i] = 2.0 * ( dx1 + dx2 ) - dx1 * dx1 / w[i-1];

        dx1 = dx2;
        slope1 = slope2;
    }

    // --

    const double vn = 3.0 * ( s4 - s3 ) * h4 / ( h5 + h4 );
    const double k0 = 2.0 * h4 + h5 - ( h4 - h5 ) * h4 / w[n-4];
    const double q0 = ( h4 - h5 ) * v[n-4] / w[n-4];

    const double ce1 = ( vn - q0 ) / k0; // resolved: c of the last spline !
    const double ce2 = ( vn - ( 2.0 * h4 + h5 ) * ce1 ) / ( h4 - h5 );
    // also: ce2 = ( v[n-4] - h4 * ce1 ) / w[n-4]; 

    const double m3 = s3 - h4 * ( ce1 + 2.0 * ce2 ) / 3.0; 

    QVector<double> m( n );

    // m[n-1]: 3.0 * ( s5 - cv * h5 - m ) + cv * h5 + m[n-2];
    m[n-1] = s4 + h5 * ( ce1 + h5 * ( 2.0 * ( ce1 - ce2 ) / h4 ) / 3.0 );
    m[n-2] = m3 + ( ce2 + ce1 ) * h4;
    m[n-3] = m3;

    double c2 = ce2;
    for ( int i = n - 4; i > 1; i-- )
    {
        const double dx = p[i+1].x() - p[i].x();
        const double slope = ( p[i + 1].y() - p[i].y() ) / dx;

        const double c1 = ( v[i-1] - dx * c2 ) / w[i-1];
        m[i] = slope - dx * ( c2 + 2.0 * c1 ) / 3.0;

        c2 = c1;
    }

    const double cb0 = ( v[0] - ( h1 - h0 ) * c2 ) / w[0];
    const double cb1 = cb0 + h0 * ( cb0 - c2 ) / h1;

    m[1] = s1 - h1 * ( c2 + 2.0 * cb0 ) / 3.0;
    m[0] = s0 - h0 * ( cb0 + 2.0 * cb1 ) / 3.0;

#if TEST_SPLINE
    qDebug() << "COMPARE 0:" << test_0( p, m );
#endif

    return m;
}

static QVector<double> qwtDerivatives1( const QPolygonF &p,
    double slopeBegin, double slopeEnd )
{
    /* first end point derivative given ? */

    const int n = p.size();

    const double h0 = p[1].x() - p[0].x();
    const double h1 = p[2].x() - p[1].x();

    const double s0 = ( p[1].y() - p[0].y() ) / h0;
    const double s1 = ( p[2].y() - p[1].y() ) / h1;

    if ( n == 3 )
    {
        const double k = slopeBegin - slopeEnd + 3.0 * ( s1 - s0 );

        QVector<double> m( 3 );
        m[0] = slopeBegin;
        m[1] = 1.5 * s1 - 0.5 * ( slopeEnd + k * h1 / ( h0 + h1 ) );
        m[2] = slopeEnd;

#if TEST_SPLINE
        qDebug() << "COMPARE 1:" << test_1( p, m, slopeBegin, slopeEnd );
#endif
        return m;
    }

    const double h4 = ( p[n-2].x() - p[n-3].x() ); 
    const double h5 = ( p[n-1].x() - p[n-2].x() ); 

    const double s4 = ( p[n-2].y() - p[n-3].y() ) / h4;
    const double s5 = ( p[n-1].y() - p[n-2].y() ) / h5;

    QVector<double> w(n-3);
    QVector<double> v(n-3);

    w[0] = 2.0 * ( h0 + h1 ) - 0.5 * h0;
    v[0] = 3.0 * ( s1 - s0 ) - 1.5 * ( s0 - slopeBegin );

    double dx1 = h1;
    double slope1 = s1;

    for ( int i = 1; i < n - 3; i++ )
    {
        double dx2 = p[i+2].x() - p[i+1].x();
        double slope2 = ( p[i+2].y() - p[i+1].y() ) / dx2;

        v[i] = 3.0 * ( slope2 - slope1 ) - dx1 * v[i - 1] / w[i-1];
        w[i] = 2.0 * ( dx1 + dx2 ) - dx1 * dx1 / w[i-1];

        dx1 = dx2;
        slope1 = slope2;
    }

    const double vn = 3.0 * ( ( s5 - s4 ) - 0.5 * ( slopeEnd - s5 ) ) - h4 * v[n-4] / w[n-4];
    const double wn = 2.0 * ( h4 + h5 ) - 0.5 * h5 - h4 * h4 / w[n-4];

    const double ce1 = -( 1.5 * ( s5 - slopeEnd ) + vn / wn * h5 * 0.5 ) / h5;
    const double ce2 = vn / wn;

    QVector<double> m( n );

    m[n-1] = slopeEnd;
    m[n-2] = s5 - h5 * ( ce1 + 2.0 * ce2 ) / 3.0;

    double c2 = ce2;
    for ( int i = n-3; i > 0; i-- )
    {
        const double dx = p[i+1].x() - p[i].x();
        const double slope = ( p[i + 1].y() - p[i].y() ) / dx;

        const double c1 = ( v[i-1] - dx * c2 ) / w[i - 1];
        m[i] = slope - dx * ( c2 + 2.0 * c1 ) / 3.0;

        c2 = c1;
    }

    m[0] = slopeBegin;

#if TEST_SPLINE
    qDebug() << "COMPARE 1:" << test_1( p, m, slopeBegin, slopeEnd );
#endif

    return m;
}

static QVector<double> qwtDerivatives2( const QPolygonF &p,
    double cvStart, double cvEnd )
{
    /* second derivative at end points ?  */

    const int n = p.size();

    const double h0 = p[1].x() - p[0].x();
    const double h1 = p[2].x() - p[1].x();

    const double s0 = ( p[1].y() - p[0].y() ) / h0;
    const double s1 = ( p[2].y() - p[1].y() ) / h1;

    if ( n == 3 )
    {
        //   0) 2 * b1 = cvStart;
        //   1) 6 * a2 * x2 + 2 * b2 = cvEnd;
        //   2) 6 * a1 * x1 + 2 * b1 = 2 * b2;
        //   3) c2 = 3 * a1 * x1 * x1 + 2 * b1 * x1 + c1;
        //   4) a1 * x1 * x1 + b1 * x1 + c1 = s0
        //   5) a2 * x2 * x2 + b2 * x2 + c2 = s1

        const double b1 = 0.5 * cvStart;
        const double b3 = 0.5 * cvEnd;

        const double a1 = ( 3.0 * ( s1 - s0 - b1 * h0 ) / h1 - 0.5 * cvEnd - 2 * b1 ) / ( 6 * ( h0 + h0 * h0 / h1 ) );
        const double b2 = 1.5 * ( s1 - s0 - 2 * a1 * h0 * h0 - b1 * h0 ) / h1 - 0.5 * b3;

#if 0
        double m1, m2, m3, m4;
        qwtToHermite2( p[0], b1, p[1], b2, m1, m2 );
        qwtToHermite2( p[1], b2, p[2], b3, m3, m4 );
#endif

        const double a2 = ( s1 - s0 - b2 * h1 - 2 * a1 * h0 * h0 - b1 * h0 ) / ( h1 * h1 );
        const double c1 = s1 - a2 * h1 * h1 - b2 * h1 - 3 * a1 * h0 * h0 - 2 * b1 * h0;
        const double c2 = 3 * a1 * h0 * h0 + 2 * b1 * h0 + c1;

        QVector<double> m(3);
        m[0] = c1;
        m[1] = c2;
        m[2] = 3.0 * a2 * h1 * h1 + 2 * b2 * h1 + c2;

#if TEST_SPLINE
        qDebug() << "COMPARE 2:" << test_2( p, m, cvStart, cvEnd );
#endif

        return m;
    }

    const double h4 = ( p[n-2].x() - p[n-3].x() );
    const double h5 = ( p[n-1].x() - p[n-2].x() );

    const double s4 = ( p[n-2].y() - p[n-3].y() ) / h4;
    const double s5 = ( p[n-1].y() - p[n-2].y() ) / h5;

    QVector<double> w(n-3);
    QVector<double> v(n-3);

    const double v0 = 3.0 * ( s1 - s0 ) - h0 * 0.5 * cvStart;
    const double w0 = 2.0 * ( h0 + h1 );

    v[0] = v0;
    w[0] = w0;

    double dx1 = h1;
    double slope1 = s1;

    for ( int i = 1; i < n-3; i++ )
    {
        double dx2 = p[i+2].x() - p[i+1].x();
        double slope2 = ( p[i+2].y() - p[i+1].y() ) / dx2;

        const double l = dx1 / w[i - 1];

        v[i] = 3.0 * ( slope2 - slope1 ) - l * v[i-1];
        w[i] = 2.0 * ( dx1 + dx2 ) - l * dx1;

        dx1 = dx2;
        slope1 = slope2;
    }

    const double vn = 3.0 * ( s5 - s4 ) - h5 * 0.5 * cvEnd - h4 / w[n-4] * v[n-4];
    const double wn = 2.0 * ( h4 + h5 ) - h4 / w[n-4] * h4;

    QVector<double> m( n );

    const double ce1 = cvEnd * 0.5;
    const double ce2 = vn / wn;

    m[n-2] = s5 - h5 * ( ce1 + 2.0 * ce2 ) / 3.0;

    const double d = ( ce1 - ce2 ) / ( 3.0 * h5 );
    m[n-1] = m[n-2] + 2 * ce2 * h5 + 3 * d * h5 * h5;
    
    double c2 = ce2;
    for ( int i = n - 3; i > 0; i-- )
    {
        const double dx = p[i+1].x() - p[i].x();
        const double slope = ( p[i + 1].y() - p[i].y() ) / dx;

        double c1 = ( v[i-1] - dx * c2 ) / w[i-1];
        m[i] = slope - dx * ( c2 + 2.0 * c1 ) / 3.0;

        c2 = c1;
    }

    const double cb0 = cvStart * 0.5;
    m[0] = s0 - h0 * ( c2 + 2.0 * cb0 ) / 3.0;

#if TEST_SPLINE
    qDebug() << "COMPARE 2:" << test_2( p, m, cvStart, cvEnd );
#endif

    return m;
}

static QVector<double> qwtDerivatives3( const QPolygonF &p,
    double marg_0, double marg_n )
{
    /* third derivative at end point ?    */

    const int n = p.size();

    const double h0 = p[1].x() - p[0].x();
    const double h1 = p[2].x() - p[1].x();

    const double s0 = ( p[1].y() - p[0].y() ) / h0;
    const double s1 = ( p[2].y() - p[1].y() ) / h1;

    if ( n == 3 )
    {
        const double m0 = marg_0 * h0;
        const double m1 = marg_n * h1;

        double v = 3.0 * ( s1 - s0 ) + 0.5 * ( m0 * h0 - m1 * h1 );

        const double c2 = v / ( h0 + h1 );

        QVector<double> m( 3 );
        m[0] = s0 - h0 * ( c2 - m0 ) / 3.0;
        m[1] = s1 - h1 * ( c2 + 0.5 * m1 ) / 3.0;
        m[2] = s1 + h1 * ( c2 + m1 ) / 3.0;

#if TEST_SPLINE
        qDebug() << "COMPARE 3:" << test_3( p, m, marg_0, marg_n );
#endif
        return m;
    }

    const double h4 = ( p[n-2].x() - p[n-3].x() );
    const double h5 = ( p[n-1].x() - p[n-2].x() );

    const double s4 = ( p[n-2].y() - p[n-3].y() ) / h4;
    const double s5 = ( p[n-1].y() - p[n-2].y() ) / h5;

    QVector<double> w(n-3);
    QVector<double> v(n-3);

    v[0] = 3.0 * ( s1 - s0 ) + 0.5 * marg_0 * h0 * h0;
    w[0] = 2.0 * ( h0 + h1 ) + h0;

    double dx1 = h1;
    double slope1 = s1;

    for ( int i = 1; i < n-3; i++ )
    {
        double dx2 = p[i+2].x() - p[i+1].x();
        double slope2 = ( p[i+2].y() - p[i+1].y() ) / dx2;

        const double l = dx1 / w[i - 1];

        v[i] = 3.0 * ( slope2 - slope1 ) - l * v[i-1];
        w[i] = 2.0 * ( dx1 + dx2 ) - l * dx1;

        dx1 = dx2;
        slope1 = slope2;
    }

    const double vn = 3.0 * ( s5 - s4 ) - 0.5 * marg_n * h5 * h5 - h4 / w[n-4] * v[n-4];
    const double wn = 2.0 * ( h4 + h5 ) + h5 - h4 / w[n-4] * h4;

    QVector<double> m( n );

    const double ce1 = vn / wn + marg_n * 0.5 * h5;
    const double ce2 = vn / wn;

    m[n-2] = s5 - h5 * ( ce1 + 2.0 * ce2 ) / 3.0;
    const double d = ( ce1 - ce2 ) / ( 3.0 * h5 );
    m[n-1] = m[n-2] + 2 * ce2 * h5 + 3 * d * h5 * h5;

    double c2 = ce2;
    for ( int i = n - 3; i > 0; i-- )
    {
        const double dx = p[i+1].x() - p[i].x();
        const double slope = ( p[i + 1].y() - p[i].y() ) / dx;

        const double c1 = ( v[i-1] - dx * c2 ) / w[i - 1];
        m[i] = slope - dx * ( c2 + 2.0 * c1 ) / 3.0;

        c2 = c1;
    }

    const double cb0 = c2 - marg_0 * 0.5 * h0;
    m[0] = s0 - h0 * ( c2 + 2.0 * cb0 ) / 3.0;

#if TEST_SPLINE
    qDebug() << "COMPARE 3:" << test_3( p, m, marg_0, marg_n );
#endif

    return m;
}

static QVector<double> qwtDerivatives4( const QPolygonF &p )
{
    // periodic spline

    const int n = p.size();

    if ( p[n-1].y() != p[0].y() )
    {
    }

    const double h0 = p[1].x() - p[0].x();
    const double h1 = p[2].x() - p[1].x();

    const double s0 = ( p[1].y() - p[0].y() ) / h0;
    const double s1 = ( p[2].y() - p[1].y() ) / h1;

    if ( n == 3 )
    {
        const double c0 = 3.0 * s0 / h1;
        const double c1 = -c0;
        const double c2 = c0;

        QVector<double> m( 3 );
        m[0] = s0 - h0 * ( c1 + 2.0 * c0 ) / 3.0;
        m[1] = s1 - h1 * ( c2 + 2.0 * c1 ) / 3.0;
        m[2] = m[1] + ( c0 + c1 ) * h1;

#if TEST_SPLINE
        qDebug() << "COMPARE 4:" << test_4( p, m );
#endif
        return m;
    }

    if ( n == 4 )
    {
        const double h2 = p[3].x() - p[2].x();
        const double s2 = ( p[3].y() - p[2].y() ) / h2;

        const double v0 = 2.0 * ( h0 + h1 );
        const double d1 = 2.0 * ( h1 + h2 ) - h1 * h1 / v0;
        const double ln = h2 - h0 * h1 / v0;
        const double dn = 2.0 * ( h0 + h2 ) - h0 * h0 / v0 - ln * ( h2 - h1 * h0 / v0 ) / d1;

        const double k0 = 3.0 * ( s1 - s0 ) / v0;
        const double k1 = ( 3.0 * ( s2 - s1 ) - k0 * h1 ) / d1;

        const double c3 = ( 3.0 * ( s0 - s2 ) - h0 * k0 - ln * k1 ) / dn;
        const double c2 = k1 - c3 * ( h2 - h1 * h0 / v0 ) / d1;
        const double c1 = k0 - h1 / v0 * c2 - h0 / v0 * c3;

        QVector<double> m( 4 );
        m[0] = s0 - h0 * ( c1 + 2.0 * c3 ) / 3.0;
        m[1] = s1 - h1 * ( c2 + 2.0 * c1 ) / 3.0;
        m[2] = s2 - h2 * ( c3 + 2.0 * c2 ) / 3.0;
        m[3] = m[2] + ( c3 + c2 ) * h2;

#if TEST_SPLINE
        qDebug() << "COMPARE 4:" << test_4( p, m );
#endif

        return m;
    }

    const double h2 = ( p[3].x() - p[2].x() );
    const double s2 = ( p[3].y() - p[2].y() ) / h2;

    const double h4 = p[n-2].x() - p[n-3].x();
    const double s4 = ( p[n-2].y() - p[n-3].y() ) / h4;

    const double h5 = p[n-1].x() - p[n-2].x();
    const double s5 = ( p[n-1].y() - p[n-2].y() ) / h5;

    double off0;
    double off1;
    double off3;

    QVector<double> w( n-2 );
    QVector<double> v( n-3 );
    QVector<double> col( n-3 );
    {
        const double w0 = 2.0 * ( h0 + h1 );

        w[0] = w0;
        w[1] = 2.0 * ( h1 + h2 ) - h1 * h1 / w0;
        col[1] = -h1 * h0 / ( w0 * w[1] );

        off0 = -h0 * h1 / w0;
        off1 = -h0 * h0 / w0; 
        off1 -= off0 * col[1];

        const double k0 = 3.0 * ( s1 - s0 ) / ( 2.0 * ( h0 + h1 ) );
        const double k1 = ( 3.0 * ( s2 - s1 ) - k0 * h1 ) / w[1];

        v[0] = k0;
        v[1] = k1;

        off3 = -h0 * k0;
        off3 -= off0 * k1;

        double dx1 = h2;
        double slope1 = s2;

        for ( int i = 2; i < n-3; i++ )
        {
            const double dx2 = p[i+2].x() - p[i+1].x();
            const double slope2 = ( p[i+2].y() - p[i+1].y() ) / dx2;

            w[i] = 2.0 * ( dx1 + dx2 ) - dx1 * dx1 / w[i-1];
            v[i] = ( 3.0 * ( slope2 - slope1 ) - v[i-1] * dx1 ) / w[i];

            col[i] = -col[i-1] * dx1 / w[i];

            off0 *= -dx1 / w[i-1];
            off1 -= off0 * col[i];
            off3 -= off0 * v[i];

            dx1 = dx2;
            slope1 = slope2;
        }
        w[n-3] = 2.0 * ( h4 + h5 ) - h4 * h4 / w[n-4];
    }

    QVector<double> m( n );
    {
        const double ln = h5 - off0 * h4 / w[n-4];
        const double dn = 2.0 * ( h0 + h5 ) + off1 - 
            ln * ( h5 - h4 * col[n-4] ) / w[n-3];

        const double cx = ( 3.0 * ( s5 - s4 ) - v[n-4] * h4 ) / w[n-3];
        const double cn = ( 3.0 * ( s0 - s5 ) + off3 - ln * cx ) / dn;
        const double cn2 = cx - cn * ( h5 - h4 * col[n-4] ) / w[n-3];

        m[n-2] = s5 - h5 * ( cn + 2.0 * cn2 ) / 3.0;
        m[n-1] = m[n-2] + ( cn + cn2 ) * h5;

        double c2 = cn2;
        for ( int i = n - 3; i > 1; i-- )
        {
            const double dx = p[i+1].x() - p[i].x();
            const double slope = ( p[i+1].y() - p[i].y() ) / dx;

            const double c1 = v[i-1] - dx / w[i-1] * c2 - col[i-1] * cn;
            m[i] = slope - dx * ( c2 + 2.0 * c1 ) / 3.0;

            c2 = c1;
        }

        const double c0 = v[0] - h1 / w[0] * c2 - h0 / w[0] * cn;
        m[1] = s1 - h1 * ( c2 + 2.0 * c0 ) / 3.0;
        m[0] = s0 - h0 * ( c0 + 2.0 * cn ) / 3.0;
    }

#if TEST_SPLINE
    qDebug() << "COMPARE 4:" << test_4( p, m );
#endif

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
            m = qwtDerivatives2( points, 0.0, 0.0 );
            break;
        case NotAKnot:
            m = qwtDerivatives0( points );
            break;
        case Periodic:
            m = qwtDerivatives4( points );
            break;
        case Test1:
            m = qwtDerivatives1( points, 2.0, 0.5 );
            break;
        case Test2:
            m = qwtDerivatives2( points, 2.0, 0.5 );
            break;
        case Test3:
            m = qwtDerivatives3( points, 0.001, 0.001 );
            break;
        case ParabolicRunout:
        case CubicRunout:
            break;
    }

    return m;
}

QVector<double> QwtSplineCubic::derivatives( 
    const QPolygonF &points, double slope1, double slope2 )
{
    if ( points.size() <= 2 )
        return QVector<double>();
    
    return qwtDerivatives1( points, slope1, slope2 );
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
