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
    class Equation2
    {
    public:
        Equation2( double p0 = 0.0, double q0 = 0.0, double r0 = 0.0 ):
            p( p0 ),
            q( q0 ),
            r( r0 )
        {
        }

        void setup( double p0 , double q0, double r0 )
        {
            p = p0;
            q = q0;
            r = r0;
        }
            
        double resolved1( double x2 ) const
        {
            return ( r - q * x2 ) / p;
        }

        double resolved2( double x1 ) const
        {
            return ( r - p * x1 ) / q;
        }

        double resolved1( const Equation2 &eq ) const
        {
            // find x1
            double k = q / eq.q;
            return ( r - k * eq.r ) / ( p - k * eq.p );
        }

        double resolved2( const Equation2 &eq ) const
        {
            // find x2
            const double k = p / eq.p;
            return ( r - k * eq.r ) / ( q - k * eq.q );
        }

        void debug() const
        {
            qDebug() << "EQS2(" << p << "," << q << "," << r << ")";
        }

        // p * x1 + q * x2 = r
        double p, q, r;
    };

    class Equation3
    {
    public:
        Equation3( const QPointF &p1, const QPointF &p2, const QPointF &p3 )
        {
            const double h1 = p2.x() - p1.x();
            const double s1  = ( p2.y() - p1.y() ) / h1;

            const double h2 = p3.x() - p2.x();
            const double s2  = ( p3.y() - p2.y() ) / h2;

            p = h1;
            q = 2 * ( h1 + h2 );
            u = h2;
            r = 3 * ( s2 - s1 );
        }

        Equation3( double cp = 0.0, double cq = 0.0, double du = 0.0, double dr = 0.0 ):
            p( cp ),
            q( cq ),
            u( du ),
            r( dr )
        {
        }
    
        inline bool operator==( const Equation3 &c ) const
        {
            return ( p == c.p ) && ( q == c.q ) && 
                ( u == c.u ) && ( r == c.r );
        }

        void setup( double cp, double cq, double du, double dr )
        {
            p = cp;
            q = cq;
            u = du;
            r = dr;
        }

        inline Equation3 normalized() const
        {
            Equation3 c;
            c.p = 1.0;
            c.q = q / p;
            c.u = u / p;
            c.r = r / p;

            return c;
        }

        Equation2 substituted1( const Equation3 eq ) const
        {
            // eliminate x1
            const double k = p / eq.p;
            return Equation2( q - k * eq.q, u - k * eq.u, r - k * eq.r );
        }

        Equation2 substituted3( const Equation3 eq ) const
        {
            // eliminate x3

            const double k = u / eq.u;
            return Equation2 ( p - k * eq.p, q - k * eq.q, r - k * eq.r );
        }

        Equation2 substituted1( const Equation2 eq ) const
        {
            // eliminate x1
            const double k = p / eq.p;
            return Equation2 ( q - k * eq.q, u, r - k * eq.r );
        }

        double resolved1( double x2, double x3 ) const
        {
            // p * x1 + q * x2 + u * x3 = r
            return ( r - q * x2 - u * x3 ) / p;
        }

        double resolved3( double x1, double x2 ) const
        {
            // p * x1 + q * x2 + u * x3 = r
            return ( r - p * x1 - q * x2 ) / u;
        }

        void debug() const
        {
            qDebug() << "EQS3(" << p << "," << q << "," << u << "," << r << ")";
        }

        double p, q, u, r;
    };

    class EquationSystem
    {
    public:
        void setStartCondition( double p, double q, double u, double r )
        {
            d_eq[0].setup( p, q, u, r );
        }

        void setEndCondition( double p, double q, double u, double r )
        {
            d_eq[1].setup( p, q, u, r );
        }

        QVector<double> resolve( const QPolygonF &p ) 
        {
            const int n = p.size();
            if ( n < 3 )
                return QVector<double>();

            const double h0 = p[1].x() - p[0].x();
            const double h1 = p[2].x() - p[1].x();

            const double hn = p[n-1].x() - p[n-2].x();
            const double sn = ( p[n-1].y() - p[n-2].y() ) / hn;

            const Equation3 eqSpline0( p[0], p[1], p[2] );
            const Equation2 eq0 = d_eq[0].substituted1( eqSpline0 );

            const Equation3 eqSplineN( p[n-3], p[n-2], p[n-1] );
            const Equation2 eqN = d_eq[1].substituted3( eqSplineN );

            QVector<double> m( n );

            if ( n == 3 )
            {
                // The equation system can be solved without substitution
                // from the start/end conditions and eqSpline0 ( = eqSplineN ).

                double b1;

                if ( d_eq[0].normalized() == d_eq[1].normalized() )
                {
                    // When we have 3 points only and start/end conditions
                    // for 3 points mean the same condition the system
                    // is under-determined and has many solutions.
                    // We chose b1 = 0.0

                    b1 = 0.0;
                }
                else 
                {
                    const Equation2 eq = d_eq[1].substituted1( eqSpline0 );
                    b1 = eq0.resolved1( eq );
                }

                const double b0 = eqN.resolved1( b1 );
                const double b2 = eqSpline0.resolved3( b0, b1 );

                const double s0 = ( p[1].y() - p[0].y() ) / h0;

                m[0] = s0 - h0 * ( b1 + 2.0 * b0 ) / 3.0;
                m[1] = m[0] + ( b0 + b1 ) * h0;
                m[2] = m[1] + ( b1 + b2 ) * h1;

                return m;
            }

            const Equation2 eqn2 = d_eq[1].substituted1( eqSplineN );

            Equation2 eq = eq0;
            if ( n > 4 )
            {
                const Equation3 eqSpline1( p[1], p[2], p[3] );
                eq = eqSpline1.substituted1( eq );
                eq = substituteSpline( p, 2, n - 4, eq );
            }

            double bi = eqN.resolved2( eq );
            const double bn = eqn2.resolved2( bi );

            m[n-1] = sn + hn * ( bi + 2 * bn ) / 3.0;
            m[n-2] = m[n-1] - ( bn + bi ) * hn;

            bi = resolveSpline( p, 2, n - 3, bi, m );

            const double b1 = eq0.resolved1( bi );
            const double b0 = d_eq[0].resolved1( b1, bi );

            m[1] = m[2] - ( b1 + bi ) * h1;
            m[0] = m[1] - ( b0 + b1 ) * h0;

            return m;
        }

    private:

        inline bool fuzzyCompare( double a, double b ) const
        {
            return ( qFuzzyIsNull(a) && qFuzzyIsNull(b) ) || qFuzzyCompare(a, b);
        }

        Equation2 substituteSpline( const QPolygonF &p, int from, int to, const Equation2 &eq )
        {
            d_w.resize( p.size() );
            d_r.resize( p.size() );
            
            d_w[from] = eq.p;
            d_r[from] = eq.r;

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

            return Equation2( d_w[to+1], dx1, d_r[to+1] );
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
        Equation3 d_eq[2];

        QVector<double> d_w;
        QVector<double> d_r;
    };

    class EquationSystem2
    {
    public:
        QVector<double> resolve( const QPolygonF &p )
        {
            const int n = p.size();

            if ( p[n-1].y() != p[0].y() )
            {
                // TODO ???
            }

            const double h0 = p[1].x() - p[0].x();
            const double s0 = ( p[1].y() - p[0].y() ) / h0;

            if ( n == 3 )
            {
                const double h1 = p[2].x() - p[1].x();

                QVector<double> m( 3 );
                m[0] = m[1] = m[2] = s0 - s0 * h0 / h1;

                return m;
            }

            const Equation3 eqSpline0( p[0], p[1], p[2] );

            double w = eqSpline0.q;
            double u = eqSpline0.p;
            double r = eqSpline0.r;
            
            double dh = h0;
            double dw = 0;
            double dr = 0;

            substitute( p, w, u, r, dh, dw, dr );

            const double hn = p[n-1].x() - p[n-2].x();
            const double sn = ( p[n-1].y() - p[n-2].y() ) / hn;

            const double wn = 2.0 * ( h0 + hn ) - ( hn + dh ) * ( u + hn ) / w;
            const double rn = 3.0 * ( s0 - sn ) - ( hn + dh ) * r / w;
            const double bn = ( rn + dr ) / ( wn + dw );

            QVector<double> m(n);

            const double b1 = ( d_r[n-2] - hn * bn - d_u[n-2] * bn ) / d_w[n-2];

            m[n-2] = sn - hn * ( bn + 2.0 * b1 ) / 3.0;

            const double bi = resolveSpline( p, bn, b1, m );

            m[0] = s0 - h0 * ( bi + 2.0 * bn ) / 3.0; 
            m[n-1] = m[0];

            return m;
        }

    private:

        void substitute( const QPolygonF &points, 
            double &w, double &u, double &r, double &dh, double &dw, double &dr )
        {
            const int n = points.size();

            d_w.resize( n - 1 );
            d_r.resize( n - 1 );
            d_u.resize( n - 1 );
            
            d_w[1] = w;
            d_r[1] = r;
            d_u[1] = u;

            double dx1 = points[2].x() - points[1].x();
            double slope1 = ( points[2].y() - points[1].y() ) / dx1;

            for ( int i = 1; i < n - 2; i++ )
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

            w = d_w[n-2];
            r = d_r[n-2];
            u = d_u[n-2];
        }

        double resolveSpline( const QPolygonF &points, 
            double bn, double b2, QVector<double> &m )
        {
            const int n = points.size();

            for ( int i = n - 3; i >= 1; i-- )
            {
                const double dx = points[i+1].x() - points[i].x();
                const double b1 = ( d_r[i] - dx * b2 - d_u[i] * bn ) / d_w[i];

                m[i] = m[i+1] - ( b1 + b2 ) * dx;

                b2 = b1;
            }

            return b2;
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
    eqs.setStartCondition( 2 * dx0 / 3.0, dx0 / 3.0, 0.0, s0 - slopeBegin ); 
    eqs.setEndCondition( 0.0, 1.0 / 3.0 * dxn, 2.0 / 3.0 * dxn, slopeEnd - sn );

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
    eqs.setStartCondition( 1.0, 0.0, 0.0, 0.5 * cvStart ); 
    eqs.setEndCondition( 0.0, 0.0, 1.0, 0.5 * cvEnd ); 

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
    eqs.setStartCondition( 1.0, -1.0, 0.0, -0.5 * marg_0 * h0 ); 
    eqs.setEndCondition( 0.0, 1.0, -1.0, -0.5 * marg_n * hn ); 

    return eqs.resolve( p );
}

static QVector<double> qwtDerivatives4( const QPolygonF &points )
{
    // periodic spline

    QwtSplineCubic::EquationSystem2 eqs;
    return eqs.resolve( points );
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
    eqs.setStartCondition( 1.0, -( 1.0 + h0 / h1 ), h0 / h1, 0.0 );
    eqs.setEndCondition( h5 / h4, -( 1.0 + h5 / h4 ), 1.0, 0.0 );

    return eqs.resolve( points );
}

static QVector<double> qwtDerivatives6( const QPolygonF &p )
{
    // parabolic runout

    // b0 = b1 => ( 1.0 ) * b0 + ( -1.0 ) * b1 = 0.0;

    QwtSplineCubic::EquationSystem eqs;
    eqs.setStartCondition( 1.0, -1.0, 0.0, 0.0 ); 
    eqs.setEndCondition( 0.0, 1.0, -1.0, 0.0 ); 

    return eqs.resolve( p );
}

static QVector<double> qwtDerivatives7( const QPolygonF &points )
{
    // cubic runout

    // b0 = 2 * b1 - b2
    // => 1.0 * b0 - 2 * b1 + 1.0 * b2 = 0.0

    QwtSplineCubic::EquationSystem eqs;
    eqs.setStartCondition( 1.0, -2.0, 1.0, 0.0 ); 
    eqs.setEndCondition( 1.0, -2.0, 1.0, 0.0 ); 

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
