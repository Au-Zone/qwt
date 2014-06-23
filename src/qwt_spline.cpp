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
        void substitute( const QPolygonF &points, 
            int from, int to, double &w, double &r )
        {
            const double dx1 = points[from+1].x() - points[from].x();
            const double dy1 = points[from+1].y() - points[from].y();

            const double dx2 = points[from+2].x() - points[from+1].x();
            const double dy2 = points[from+2].y() - points[from+1].y();

            w = 2 * ( dx1 + dx2 ) - dx1 * d_q0 / d_p0;
            r = 3 * ( dy2 / dx2 - dy1 / dx1 ) - dx1 * d_r0 / d_p0;

            substitute2( points, from + 1, to, w, r );
        }

        void setStartCondition( double p0, double q0, double r0 )
        {
            // p * c[i] + q * c[i+1] = r0

            d_p0 = p0;
            d_q0 = q0;
            d_r0 = r0;
        }

        void setEndCondition( double pn, double qn, double rn )
        {
            // p * c[i] + q * c[i+1] = r0

            d_pn = pn;
            d_qn = qn;
            d_rn = rn;
        }

        void substitute2( const QPolygonF &p, int from, int to, double &w, double &r )
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

    public:
        void resolve2( const QPolygonF &p, int from, int to, 
            double &c2, QVector<double> &m )
        {
            for ( int i = to; i >= from; i-- )
            {
                const double dx = p[i+1].x() - p[i].x();
                const double c1 = ( d_r[i] - dx * c2 ) / d_w[i];

                m[i] = m[i+1] - ( c1 + c2 ) * dx;

                c2 = c1;
            }
        }

        void resolve3( const QPolygonF &p, int from, int to,
            double &c2, QVector<double> &m )
        {
            resolve2( p, from, to, c2, m );

            const double dx = p[from].x() - p[from-1].x();

            double c1;
			if ( d_p0 != 0.0 )
			{
                // p0 * c1 + q0 * c2 = r0
				c1 = ( d_r0 - d_q0 * c2 ) / d_p0;
			}
			else
			{
                // c2 = r0 / q0
#if 1
                c1 = ( d_r[from] - dx * d_r0 / d_q0 ) / d_w[from]; // ???
#endif
			}

            m[from - 1] = m[from] - ( c1 + c2 ) * dx;

            c2 = c1;
        }

        QVector<double> resolve( const QPolygonF &points ) 
        {
            const int n = points.size();

            const double dx = points[n-1].x() - points[n-2].x();
            const double dy = points[n-1].y() - points[n-2].y();
            const double s = dy / dx;

            double w, r;
            substitute( points, 0, n - 3, w, r ); 

            const double c2 = ( d_rn - d_pn * r / w ) / ( d_qn - d_pn * dx / w );
			double c1;
			if ( d_pn != 0.0 )
			{
            	c1 = ( d_rn - d_qn * c2 ) / d_pn;
			}
			else
			{
            	// d_pn = 0 => c2 = d_rn / d_qn
                c1 = ( r - dx * d_rn / d_qn ) / w;
			}

            QVector<double> m(n);

            m[n-1] = ( c1 + c2 ) * dx + s - dx * ( 2.0 * c1 + c2 ) / 3.0;
            m[n-2] = m[n-1] - ( c1 + c2 ) * dx;

            resolve3( points, 1, n - 3, c1, m );

            return m;
        }

    private:
        double d_p0, d_q0, d_r0;
        double d_pn, d_qn, d_rn;

        QVector<double> d_w;
        QVector<double> d_r;
    };

    class EquationSystem2
    {
    public:
        void substitute( const QPolygonF &p, int from, int to, 
            double &w, double &u, double &r, double &dh, double &dw, double &dr )
        {
            d_w.resize( p.size() );
            d_r.resize( p.size() );
            d_u.resize( p.size() );
            
            d_w[from] = w;
            d_r[from] = r;
            d_u[from] = u;

            double dx1 = p[from+1].x() - p[from].x();
            double slope1 = ( p[from+1].y() - p[from].y() ) / dx1;

            for ( int i = from; i <= to; i++ )
            {
                dw -= dh * d_u[i] / d_w[i];
                dr -= dh * d_r[i] / d_w[i];
                dh *= -dx1 / d_w[i];

                const double dx2 = p[i+2].x() - p[i+1].x();
                const double slope2 = ( p[i+2].y() - p[i+1].y() ) / dx2;

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

        void resolve( const QPolygonF &p, int from, int to, 
            double cn, double &c2, QVector<double> &m )
        {
            for ( int i = to; i >= from; i-- )
            {
                const double dx = p[i+1].x() - p[i].x();
                const double c1 = ( d_r[i] - dx * c2 - d_u[i] * cn ) / d_w[i];

                if ( i == to )
                {
                    const double slope = ( p[i+1].y() - p[i].y() ) / dx;
                    m[i] = slope - dx * ( c2 + 2.0 * c1 ) / 3.0;
                }
                else
                {
                    m[i] = m[i+1] - ( c1 + c2 ) * dx;
                }

                c2 = c1;
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

static QVector<double> qwtDerivatives5( const QPolygonF &p )
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

        return m;
    }


    const double h4 = ( p[n-2].x() - p[n-3].x() );
    const double s3 = ( p[n-2].y() - p[n-3].y() ) / h4;

    const double h5 = ( p[n-1].x() - p[n-2].x() );
    const double s4 = ( p[n-1].y() - p[n-2].y() ) / h5;

    const double wn = h5 + 2.0 * h4;
    const double rn = 3.0 * ( s4 - s3 ) * h4 / ( h5 + h4 );

    // h0 * c0 + 2 * c1 * ( h0 + h1 ) + h1 * c2 = 3 * ( s1 - s0 )
    // c0 = c1 * ( 1 + h0 / h1 ) - h0 / h1 * c2
    // => c1 * ( 3 * h0 + 2 * h1 + h0 * h0 / h1 ) + c2 ( h1 - h0 * h0 / h1 ) = 3 * ( s1 - s0 )
    double w, r;

    //const double cb0 = cb1 * ( 1.0 + h0 / h1 ) - c2 * ( h0 / h1 );
    const double p_0 = 1.0 + h0 / h1;
    const double q_0 = h0 / h1;

    const double p_1 = 3 * h0 + 2 * h1 + h0 * q_0;
    const double q_1 = h1 - h0 * q_0;
    const double r_1 = 3.0 * ( s1 - s0 ); 

    QwtSplineCubic::EquationSystem eqs;
    eqs.setStartCondition( p_1, q_1, r_1 ); 
    eqs.substitute( p, 1, n - 4, w, r ); 

    const double k0 = wn - ( h4 - h5 ) * h4 / w;
    const double q0 = ( h4 - h5 ) * r / w;

    const double ce1 = ( rn - q0 ) / k0; // resolved: c of the last spline !
    const double ce2 = ( r - h4 * ce1 ) / w; 
    // also: ce2 = ( rn - ( 2.0 * h4 + h5 ) * ce1 ) / ( h4 - h5 );

    const double m3 = s3 - h4 * ( ce1 + 2.0 * ce2 ) / 3.0; 

    QVector<double> m( n );

    // m[n-1]: 3.0 * ( s5 - cv * h5 - m ) + cv * h5 + m[n-2];
    m[n-1] = s4 + h5 * ( ce1 + h5 * ( 2.0 * ( ce1 - ce2 ) / h4 ) / 3.0 );
    m[n-2] = m3 + ( ce2 + ce1 ) * h4;
    m[n-3] = m3;

    double c2 = ce2;
    eqs.resolve2( p, 2, n - 4, c2, m );

    const double cb1 = ( r_1 - q_1 * c2 ) / p_1;
    m[1] = m[2] - ( cb1 + c2 ) * h1;

    const double cb0 = cb1 * p_0 - c2 * q_0;
    m[0] = m[1] - ( cb0 + cb1 ) * h0;

    return m;
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

static QVector<double> qwtDerivatives7( const QPolygonF &p )
{
    // cubic runout

    const int n = p.size();

    const double h0 = p[1].x() - p[0].x();
    const double s0 = ( p[1].y() - p[0].y() ) / h0;

    const double h1 = p[2].x() - p[1].x();
    const double s1 = ( p[2].y() - p[1].y() ) / h1;

    if ( n == 3 )
    {
        // the system is under-determined and we only 
        // and has many solutions. We chose the one with c1 = 0.0

        const double c0 = 3 * ( s1 - s0 ) / ( h0 - h1 );
        const double c1 = 0.0;
        const double c2 = -c0;

        QVector<double> m(3);
        m[0] = s0 - h0 * ( c1 + 2.0 * c0 ) / 3.0;
        m[1] = m[0] + ( c0 + c1 ) * h0;
        m[2] = m[1] + ( c1 + c2 ) * h1;

        return m;
    }

    const double h2 = p[3].x() - p[2].x();
    const double s2 = ( p[3].y() - p[2].y() ) / h2;

    const double r0 = 3 * ( s1 - s0 ); 
    const double r1 = 3 * ( s2 - s1 );
    const double k1 = 4 * h0 + 2 * h1;

    const double k0 = ( h1 - h2 );
    const double k4 = ( h1 - h0 );
    const double k2 = ( 2 * h1 + 4 * h2 );

#if 1
    if ( n == 4 )
    {
        // c0 = 2 * c1 - c2
        // c3 = 2 * c2 - c1
        // h0 * c0 + 2 * ( h0 + h1 ) * c1 + h1 * c2 = 3 * ( s1 - s0 )
        // h1 * c1 + 2 * ( h1 + h2 ) * c2 + h2 * c3 = 3 * ( s2 - s1 )

        // h0 * ( 2 * c1 - c2 ) + 2 * ( h0 + h1 ) * c1 + h1 * c2 = 3 * ( s1 - s0 )
        // h0 * 2 * c1 - h0 * c2 + ( 2 * h0 + 2 * h1 ) * c1 + h1 * c2 = 3 * ( s1 - s0 )
        // c1 * ( 4 * h0 + 2 * h1 ) + c2 * ( h1 - h0 ) = 3 * ( s1 - s0 )

        // h1 * c1 + 2 * ( h1 + h2 ) * c2 + h2 * ( 2 * c2 - c1 ) = 3 * ( s2 - s1 )
        // h1 * c1 + ( 2 * h1 + 2 * h2 ) * c2 + h2 * 2 * c2 - c1 * h2 = 3 * ( s2 - s1 )
        // c1 * ( h1 - h2 ) + c2 * ( 2 * h1 + 4 * h2 ) = 3 * ( s2 - s1 )


        // c1 * k1 + c2 * k4 = r0
        // c1 * k0 + c2 * k2 = r1;

        // c1 = ( r1 - c2 * k2 ) / k0;
        // ( r1 - c2 * k2 ) / k0 * k1 + c2 * k4 = r0
        // r1 * k1 / k0 - c2 * k2 * k1 / k0 + c2 * k4 = r0
        // c2 * ( k4 - k2 * k1 / k0 ) = r0 - r1 * k1 / k0

        // k0 == 0: h1 = h2 ???
        const double c2 = ( r0 - r1 * k1 / k0 ) / ( k4 - k2 * k1 / k0 );
        const double c1 = ( r1 - c2 * k2 ) / k0;
        const double c0 = 2 * c1 - c2;
        const double c3 = 2 * c2 - c1;

        QVector<double> m(4);
        m[0] = s0 - h0 * ( c1 + 2.0 * c0 ) / 3.0;
        m[1] = m[0] + ( c0 + c1 ) * h0;
        m[2] = m[1] + ( c1 + c2 ) * h1;
        m[3] = m[2] + ( c2 + c3 ) * h2;

        return m;
    }
#endif

    const double h4 = ( p[n-2].x() - p[n-3].x() );
    const double s4 = ( p[n-2].y() - p[n-3].y() ) / h4;

    const double h5 = ( p[n-1].x() - p[n-2].x() );

    // b0 = 2 * b1 - b2
    // => b1 * ( 2 * ( 2 * h0 + h1 ) ) + b2 * ( h1 - h0 ) = 3 * ( s1 - s0 )
    double w, r;

    // const double cb0 = 2 * cb2 - c2
    const double p_0 = 2.0;
    const double q_0 = -1.0;

    const double p_1 = 2 * ( 2 * h0 + h1 );
    const double q_1 = h1 - h0;
    const double r_1 = 3 * ( s1 - s0 ); 

    QwtSplineCubic::EquationSystem eqs;
    eqs.setStartCondition( p_1, q_1, r_1 ); 
    eqs.substitute( p, 1, n - 4, w, r ); 

    const double r4 = r;
    const double w4 = w;

    eqs.substitute2( p, n - 3, n - 3, w, r );

    const double rn = r + h5 * r4 / w4;
    const double wn = w + h5 * ( 2 + h4 / w4 );

    const double ce2 = rn / wn;
    const double ce3 = ( r4 - h4 * ce2 ) / w4;
    const double ce1 = 2 * ce2 - ce3;

    QVector<double> m( n );

    m[n-3] = s4 - h4 * ( ce2 + 2.0 * ce3 ) / 3.0;
    m[n-2] = ( ce2 + ce3 ) * h4 + m[n-3];
    m[n-1] = ( ce1 + ce2 ) * h5 + m[n-2];

    double c2 = ce3;
    eqs.resolve2( p, 2, n - 4, c2, m );

    const double cb2 = ( r_1 - q_1 * c2 ) / p_1;
    m[1] = m[2] - ( c2 + cb2 ) * h1;

    const double cb1 = p_0 * cb2 + q_0 * c2;
    m[0] = m[1] - ( cb2 + cb1 ) * h0;

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
