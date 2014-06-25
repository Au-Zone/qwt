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
#if 1
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

static inline void qwtToDerivative( double dx, double a, double b, double c, 
    double &m1, double &m2 )
{
    m1 = c;
    m2 = ( ( 3.0 * a * dx ) + 2.0 * b ) * dx + c;
}

static inline void qwtToDerivative( const QPointF &p1, const QPointF &p2,
    double a, double b, double c, double &m1, double &m2 )
{
    return qwtToDerivative( p2.x() - p1.x(), a, b, c, m1, m2 );
}

#if 0
static void qwtToDerivative( const QPointF &p1, const QPointF &p2,
    double b1, double b2, double &m1, double &m2 )
{
    const double dx = p2.x() - p1.x();
    const double slope = ( p2.y() - p1.y() ) / dx;

    m1 = slope - dx * ( 2.0 * b1 + b2 ) / 3.0;
    m2 = ( b1 + b2 ) * dx + m1;
}
#endif

static inline void qwtToPolynom(
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

static inline void qwtToPolynom2(
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

            qwtToPolynom( p[j], m[j], p[j + 1], m[j + 1], a, b, c );

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

namespace QwtSplineCubic
{
    class EquationSystem
    {
    public:
        void setStartCondition( double p0, double q0, double r0 )
        {
            // p0 * b[i] + q0 * b[i+1] = r0

            d_p0 = p0;
            d_q0 = q0;
            d_u0 = 0.0;
            d_r0 = r0;
        }

        void setStartCondition3( double p0, double q0, double u0, double r0 )
        {
            // p0 * b[0] + q0 * b[1] + u0 * b[2] = r0

            d_p0 = p0;
            d_q0 = q0;
            d_u0 = u0;
            d_r0 = r0;
        }

        void setEndCondition( double pn, double qn, double rn )
        {
            // pn * b[n-2] + qn * b[n-1] = rn

            d_pn = pn;
            d_qn = qn;
            d_un = 0.0;
            d_rn = rn;
        }

        void setEndCondition3( double pn, double qn, double un, double rn )
        {
            // pn * b[n-3] + qn * b[n-2] * un * b[n-1] = rn

            d_pn = pn;
            d_qn = qn;
            d_un = un;
            d_rn = rn;
        }


        QVector<double> resolve( const QPolygonF &points ) 
        {
            QVector<double> m;

            if ( d_u0 == 0.0 )
                m = resolve2( points );
            else
                m = resolve3( points );

            return m;
        }

    private:

        QVector<double> resolve3( const QPolygonF &points ) 
        {
            const int n = points.size();

            const double h0 = points[1].x() - points[0].x();
            const double s0  = ( points[1].y() - points[0].y() ) / h0;

            const double h1 = points[2].x() - points[1].x();
            const double s1  = ( points[2].y() - points[1].y() ) / h1;

            const double h2 = points[3].x() - points[2].x();
            const double s2  = ( points[3].y() - points[2].y() ) / h2;

            const double hn1 = ( points[n-2].x() - points[n-3].x() );

            const double hn2 = ( points[n-1].x() - points[n-2].x() );
            const double sn2 = ( points[n-1].y() - points[n-2].y() ) / hn2;

            // 1) p0 * b[0] + q0 * b[1] + u0 * b[2] = r0
            // 2) h[0] * b[0] + 2 * ( h[0] + h[1] ) + h[2] * b[2] = s[1] - s[0]
            //
            // we transform the 2 equation above into:
            // 3) e1 * b[1] * f1 + b[2] = g1

            const double e1 = 2 * ( h0 + h1 ) - h0 * d_q0 / d_p0;
            const double f1 = h1 - h0 * d_u0 / d_p0;
            const double g1 = 3 * ( s1 - s0 ) - h0 * d_r0 / d_p0;
            
            double w = 2 * ( h1 + h2 ) - h1 * f1 / e1;
            double r = 3 * ( s2 - s1 ) - h1 * g1 / e1;

            double en, fn, gn;
            if ( n == 4 )
            {
                // there is no substituition for 4 points
                // and we can resolve from the initial equation

                en = e1;
                fn = f1;
                gn = g1;
            }
            else
            {
                substituteSpline( points, 2, n - 3, w, r );

                // as an result from the substitution above:
                // => b[n-3] * w[n-3] + b[n-2] * h[n-2] = r[n-2]

                en = d_w[n-3];
                fn = hn1;
                gn = d_r[n-3];
            }

            // 4) en * b[n-3] * fn + b[n-2] = gn

            const double bn1 = ( d_rn - d_pn * gn / en - d_un / hn2 * r ) 
                / ( d_qn - d_pn * fn / en - d_un / hn2 * w );

            const double bn2 = ( r - w * bn1 ) / hn2; 

            QVector<double> m( n );
            m[n-1] = sn2 + hn2 * ( bn1 + 2 * bn2 ) / 3.0;
            m[n-2] = m[n-1] - ( bn2 + bn1 ) * hn2;

            const double b2 = resolveSpline( points, 2, n - 3, bn1, m );
            const double b1 = ( g1 - f1 * b2 ) / e1;
            const double b0 = ( d_r0 - d_q0 * b1 - d_u0 * b2 ) / d_p0;

            m[1] = m[2] - ( b1 + b2 ) * h1;
            m[0] = m[1] - ( b0 + b1 ) * h0;

            return m;
        }

        QVector<double> resolve2( const QPolygonF &points ) 
        {
            const int n = points.size();

            const double h0 = points[1].x() - points[0].x();
            const double s0  = ( points[1].y() - points[0].y() ) / h0;

            const double h1 = points[2].x() - points[1].x();
            const double s1  = ( points[2].y() - points[1].y() ) / h1;

            const double hn = points[n-1].x() - points[n-2].x();
            const double sn = ( points[n-1].y() - points[n-2].y() ) / hn;

            // substitute
            double w = 2 * ( h0 + h1 ) - h0 * d_q0 / d_p0;
            double r = 3 * ( s1 - s0 ) - h0 * d_r0 / d_p0;

            substituteSpline( points, 1, n - 3, w, r );

            // resolve
            double b2 = ( d_rn - d_pn * r / w ) / ( d_qn - d_pn * hn / w );

            double b1;
            if ( d_pn != 0.0 )
            {
                b1 = ( d_rn - d_qn * b2 ) / d_pn;
            }
            else
            {
                // d_pn = 0 => b2 = d_rn / d_qn
                b1 = ( r - hn * d_rn / d_qn ) / w;
            }

            QVector<double> m(n);

            m[n-1] = sn + hn * ( b1 + 2 * b2 ) / 3.0;
            m[n-2] = m[n-1] - ( b1 + b2 ) * hn;

            const double bn2 = resolveSpline( points, 1, n - 3, b1, m );
            const double bn1 = ( d_r0 - d_q0 * bn2 ) / d_p0;

            m[0] = m[1] - ( bn1 + bn2 ) * h0;
            return m;
        }

        void substituteSpline( const QPolygonF &p, int from, int to, double &w, double &r )
        {
            d_w.resize( p.size() );
            d_r.resize( p.size() );
            
            d_w[from] = w;
            d_r[from] = r;

            double dx1 = p[from+1].x() - p[from].x();
            double slope1 = ( p[from+1].y() - p[from].y() ) / dx1;

            for ( int i = from; i <= to; i++ )
            {
                double dx2 = p[i+2].x() - p[i+1].x();
                double slope2 = ( p[i+2].y() - p[i+1].y() ) / dx2;

                const double v = dx1 / d_w[i];

                d_r[i+1] = 3.0 * ( slope2 - slope1 ) - v * d_r[i];
                d_w[i+1] = 2.0 * ( dx1 + dx2 ) - dx1 * v;

                dx1 = dx2;
                slope1 = slope2;
            }

            w = d_w[to+1];
            r = d_r[to+1];
        }

        double resolveSpline( const QPolygonF &points, int from, int to, 
            double b2, QVector<double> &m )
        {
            for ( int i = to; i >= from; i-- )
            {
                const double dx = points[i+1].x() - points[i].x();
                const double b1 = ( d_r[i] - dx * b2 ) / d_w[i];

                m[i] = m[i+1] - ( b1 + b2 ) * dx;

                b2 = b1;
            }

            return b2;
        }

    private:
        double d_p0, d_q0, d_u0, d_r0;
        double d_pn, d_qn, d_un, d_rn;

        QVector<double> d_w;
        QVector<double> d_r;
    };

    class EquationSystem2
    {
    public:
        void substitute( const QPolygonF &points, int from, int to, 
            double &w, double &u, double &r, double &dh, double &dw, double &dr )
        {
            d_w.resize( points.size() );
            d_r.resize( points.size() );
            d_u.resize( points.size() );
            
            d_w[from] = w;
            d_r[from] = r;
            d_u[from] = u;

            double dx1 = points[from+1].x() - points[from].x();
            double slope1 = ( points[from+1].y() - points[from].y() ) / dx1;

            for ( int i = from; i <= to; i++ )
            {
                dw -= dh * d_u[i] / d_w[i];
                dr -= dh * d_r[i] / d_w[i];
                dh *= -dx1 / d_w[i];

                const double dx2 = points[i+2].x() - points[i+1].x();
                const double slope2 = ( points[i+2].y() - points[i+1].y() ) / dx2;

                const double wx = dx1 / d_w[i];

                d_w[i+1] = 2.0 * ( dx1 + dx2 ) - dx1 * wx;
                d_u[i+1] = -d_u[i] * wx;
                d_r[i+1] = ( 3.0 * ( slope2 - slope1 ) - d_r[i] * wx );

                dx1 = dx2;
                slope1 = slope2;
            }

            w = d_w[to+1];
            r = d_r[to+1];
            u = d_u[to+1];
        }

        void resolve( const QPolygonF &points, int from, int to, 
            double bn, double &b2, QVector<double> &m )
        {
            for ( int i = to; i >= from; i-- )
            {
                const double dx = points[i+1].x() - points[i].x();
                const double b1 = ( d_r[i] - dx * b2 - d_u[i] * bn ) / d_w[i];

                if ( i == to )
                {
                    const double slope = ( points[i+1].y() - points[i].y() ) / dx;
                    m[i] = slope - dx * ( b2 + 2.0 * b1 ) / 3.0;
                }
                else
                {
                    m[i] = m[i+1] - ( b1 + b2 ) * dx;
                }

                b2 = b1;
            }
        }

    public:
        QVector<double> d_w;
        QVector<double> d_r;
        QVector<double> d_u;
    };
}

static QVector<double> qwtDerivatives1( const QPolygonF &points,
    double slopeBegin, double slopeEnd )
{
    /* first end point derivative given ? */

    const int n = points.size();

    const double dx0 = points[1].x() - points[0].x();
    const double s0 = ( points[1].y() - points[0].y() ) / dx0;

    const double dxn = ( points[n-1].x() - points[n-2].x() ); 
    const double sn = ( points[n-1].y() - points[n-2].y() ) / dxn;

    // 3 * a1 * h + b1 = b2
    // a1 * h * h + b1 * h + c1 = s

    // c1 = slopeBegin
    // => b1 * ( 2 * h / 3.0 ) + b2 * ( h / 3.0 ) = s - slopeBegin

    // c2 = slopeEnd
    // => b1 * ( 1.0 / 3.0 ) + b2 * ( 2.0 / 3.0 ) = ( slopeEnd - s ) / h;

    QwtSplineCubic::EquationSystem eqs;
    eqs.setStartCondition( 2 * dx0 / 3.0, dx0 / 3.0, s0 - slopeBegin ); 
    eqs.setEndCondition( 1.0 / 3.0 * dxn, 2.0 / 3.0 * dxn, slopeEnd - sn );

    return eqs.resolve( points );
}

static QVector<double> qwtDerivatives2( const QPolygonF &points,
    double cvStart, double cvEnd )
{
    /* second derivative at end points ?  */

    // b0 = 0.5 * cvStart
    // => b0 * 1.0 + b1 * 0.0 = 0.5 * cvStart

    // b1 = 0.5 * cvEnd
    // => b0 * 0.0 + b1 * 1.0 = 0.5 * cvEnd

    QwtSplineCubic::EquationSystem eqs;
    eqs.setStartCondition( 1.0, 0.0, 0.5 * cvStart ); 
    eqs.setEndCondition( 0.0, 1.0, 0.5 * cvEnd ); 

    return eqs.resolve( points );
}

static QVector<double> qwtDerivatives3( const QPolygonF &p,
    double marg_0, double marg_n )
{
    /* third derivative at end point ?    */

    const int n = p.size();

    const double h0 = p[1].x() - p[0].x();
    const double hn = ( p[n-1].x() - p[n-2].x() );

    // 3 * a * h0 + b[0] = b[1]

    // a = marg_0 / 6.0
    // => b[0] * 1.0 + b[1] * ( -1.0 ) = -0.5 * marg_0 * h0

    // a = marg_n / 6.0
    // => b[n-2] * 1.0 + b[n-1] * ( -1.0 ) = -0.5 * marg_n * h5

    QwtSplineCubic::EquationSystem eqs;
    eqs.setStartCondition( 1.0, -1.0, -0.5 * marg_0 * h0 ); 
    eqs.setEndCondition( 1.0, -1.0, -0.5 * marg_n * hn ); 

    return eqs.resolve( p );
}

static QVector<double> qwtDerivatives4( const QPolygonF &p )
{
    // periodic spline

    const int n = p.size();

    if ( p[n-1].y() != p[0].y() )
    {
        // TODO ???
    }

    const double h0 = p[1].x() - p[0].x();
    const double s0 = ( p[1].y() - p[0].y() ) / h0;

    const double h1 = p[2].x() - p[1].x();
    const double s1 = ( p[2].y() - p[1].y() ) / h1;

    if ( n == 3 )
    {
        QVector<double> m( 3 );
        m[0] = m[1] = m[2] = s0 - s0 * h0 / h1;

        return m;
    }

    const double hn = p[n-1].x() - p[n-2].x();
    const double sn = ( p[n-1].y() - p[n-2].y() ) / hn;

    double w = 2.0 * ( h0 + h1 );
    double u = h0;
    double r = 3.0 * ( s1 - s0 );
    double dh = h0;
    double dw = 0;
    double dr = 0;

    QwtSplineCubic::EquationSystem2 eqs;
    eqs.substitute( p, 1, n - 3, w, u, r, dh, dw, dr );

    const double wn = 2.0 * ( h0 + hn ) - ( hn + dh ) * ( u + hn ) / w;
    const double rn = 3.0 * ( s0 - sn ) - ( hn + dh ) * r / w;
    const double cn = ( rn + dr ) / ( wn + dw );

    QVector<double> m(n);

    double c2 = cn;
    eqs.resolve( p, 1, n - 2, cn, c2, m );

    m[0] = s0 - h0 * ( c2 + 2.0 * cn ) / 3.0; 
    m[n-1] = m[0];

    return m;
}

static QVector<double> qwtDerivatives5( const QPolygonF &points )
{
    /* not-a-node condition ?             */

    const int n = points.size();

    const double h0 = points[1].x() - points[0].x();
    const double h1 = points[2].x() - points[1].x();

    const double s0 = ( points[1].y() - points[0].y() ) / h0;
    const double s1 = ( points[2].y() - points[1].y() ) / h1;

    if ( n == 3 )
    {
        /*
          the system is under-determined and we only 
          compute a quadratic spline.                   
         */

        const double b = ( s1 - s0 ) / ( h0 + h1 );

        QVector<double> m( 3 );
        m[0] = s0 - h0 * b;
        m[1] = s1 - h1 * b;
        m[2] = s1 + h1 * b;

        return m;
    }

    const double h4 = ( points[n-2].x() - points[n-3].x() );
    const double h5 = ( points[n-1].x() - points[n-2].x() );

    QwtSplineCubic::EquationSystem eqs;
    eqs.setStartCondition3( 1.0, -( 1.0 + h0 / h1 ), h0 / h1, 0.0 );
    eqs.setEndCondition3( h5 / h4, -( 1.0 + h5 / h4 ), 1.0, 0.0 );

    return eqs.resolve( points );
}

static QVector<double> qwtDerivatives6( const QPolygonF &p )
{
    // parabolic runout

    // b0 = b1 => ( 1.0 ) * b0 + ( -1.0 ) * b1 = 0.0;

    QwtSplineCubic::EquationSystem eqs;
    eqs.setStartCondition( 1.0, -1.0, 0.0 ); 
    eqs.setEndCondition( 1.0, -1.0, 0.0 ); 

    return eqs.resolve( p );
}

static QVector<double> qwtDerivatives7( const QPolygonF &points )
{
    // cubic runout

    const int n = points.size();

    if ( n == 3 )
    {
        const double h0 = points[1].x() - points[0].x();
        const double s0 = ( points[1].y() - points[0].y() ) / h0;

        const double h1 = points[2].x() - points[1].x();
        const double s1 = ( points[2].y() - points[1].y() ) / h1;

        // the system is under-determined and has many solutions. 
        // We chose the one with c1 = 0.0

        const double b0 = 3 * ( s1 - s0 ) / ( h0 - h1 );
        const double b1 = 0.0;
        const double b2 = -b0;

        QVector<double> m(3);
        m[0] = s0 - h0 * ( b1 + 2.0 * b0 ) / 3.0;
        m[1] = m[0] + ( b0 + b1 ) * h0;
        m[2] = m[1] + ( b1 + b2 ) * h1;

        return m;
    }

    // b0 = 2 * b1 - b2
    // => 1.0 * b0 - 2 * b1 + 1.0 * b2 = 0.0

    QwtSplineCubic::EquationSystem eqs;
    eqs.setStartCondition3( 1.0, -2.0, 1.0, 0.0 ); 
    eqs.setEndCondition3( 1.0, -2.0, 1.0, 0.0 ); 

    return eqs.resolve( points );
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
        case Periodic:
            m = qwtDerivatives4( points );
            break;
        case NotAKnot:
            m = qwtDerivatives5( points );
            break;
        case ParabolicRunout:
            m = qwtDerivatives6( points );
            break;
        case CubicRunout:
            m = qwtDerivatives7( points );
            break;
    }

    return m;
}

QVector<double> QwtSplineCubic::derivatives( 
    const QPolygonF &points, double slopeBegin, double slopeEnd )
{
    if ( points.size() <= 2 )
        return QVector<double>();
    
    return qwtDerivatives1( points, slopeBegin, slopeEnd );
}   

QVector<double> QwtSplineCubic::derivatives2(
    const QPolygonF &points, double cvBegin, double cvEnd )
{
    if ( points.size() <= 2 )
        return QVector<double>();

    return qwtDerivatives2( points, cvBegin, cvEnd );
}

QVector<double> QwtSplineCubic::derivatives3(
    const QPolygonF &points, double valueBegin, double valueEnd )
{   
    if ( points.size() <= 2 )
        return QVector<double>();
    
    return qwtDerivatives3( points, valueBegin, valueEnd );
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
