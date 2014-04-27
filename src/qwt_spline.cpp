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

static inline void qwtCubicTo( const QPointF &p1, const QPointF &p2, 
    double dx, double b, double c, QPainterPath &path )
{
    const double stepX = dx / 3.0;

    const double cy1 = p1.y() + c * stepX;
    const double cy2 = cy1 + ( c + b * dx ) * stepX;

    path.cubicTo( p1.x() + stepX, cy1,
        p2.x() - stepX, cy2, p2.x(), p2.y() );
}

static inline void qwtHermiteCubicTo( const QPointF &p1, double m1,
    const QPointF &p2, double m2, QPainterPath &path )
{
    const double stepX = ( p2.x() - p1.x() ) / 3.0;

    path.cubicTo( p1.x() + stepX, p1.y() + stepX * m1,
        p2.x() - stepX, p2.y() - stepX * m2, p2.x(), p2.y() );
}

static inline void qwtNaturalCubicTo( const QPointF &p1, double cv1,
    const QPointF &p2, double cv2, QPainterPath &path )
{
    const double dx = p2.x() - p1.x();
    const double dy = p2.y() - p1.y();
    const double stepX = dx / 3.0;

    const double c = dy / dx - ( 0.5 * cv2 + cv1 ) * stepX;

    const double cy1 = p1.y() + c * stepX;
    const double cy2 = cy1 + ( c + 0.5 * cv1 * dx ) * stepX; 

    path.cubicTo( p1.x() + stepX, cy1, 
        p2.x() - stepX, cy2, p2.x(), p2.y() );
}

QVector<double> QwtSplineNatural::curvatures( const QPolygonF &points )
{
    const int size = points.size();

    QVector<double> aa0( size - 1 );
    QVector<double> bb0( size - 1 );

    {
        double dx1 = points[1].x() - points[0].x();
        double slope1 = ( points[1].y() - points[0].y() ) / dx1;

        for ( int i = 1; i < size - 1; i++ )
        {
            const double dx2 = points[i+1].x() - points[i].x();
            const double slope2 = ( points[i+1].y() - points[i].y() ) / dx2;

            aa0[i] = 2.0 * ( dx1 + dx2 );
            bb0[i] = 6.0 * ( slope1 - slope2 );

            slope1 = slope2;
            dx1 = dx2;
        }
    }

    // L-U Factorization
    QVector<double> cc0( size - 1 );
    for ( int i = 1; i < size - 2; i++ )
    {
        const double dx = points[i+1].x() - points[i].x();

        cc0[i] = dx / aa0[i];
        aa0[i+1] -= dx * cc0[i];
    }

    // forward elimination
    QVector<double> s( size );
    s[1] = bb0[1];
    for ( int i = 2; i < size - 1; i++ )
    {
        s[i] = bb0[i] - cc0[i-1] * s[i-1];
    }

    bb0.clear();
    cc0.clear();

    // backward elimination
    s[size - 2] = - s[size - 2] / aa0[size - 2];
    for ( int i = size - 3; i > 0; i-- )
    {
        const double dx = points[i+1].x() - points[i].x();
        s[i] = - ( s[i] + dx * s[i+1] ) / aa0[i];
    }

    s[size - 1] = s[0] = 0.0;

    return s;
}

static inline void qwtNaturalCoeff(double b1, double b2,
    const QPointF &p1, const QPointF &p2, double &a, double &c )
{
    const double dx = p2.x() - p1.x();
    const double dy = p2.y() - p1.y();

    a = ( b2 - b1 ) / ( 3.0 * dx );
    c = dy / dx - ( b2 + 2.0 * b1 ) * dx / 3.0;
}

QPolygonF QwtSplineNatural::polygon( const QPolygonF &points, int numPoints )
{
    const int size = points.size();
    if ( size <= 2 )
        return points;

    const QVector<double> cv = curvatures( points );

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

            QwtSpline::toCoefficients( 
                p[j].x(), p[j].y(), cv[j],
                p[j + 1].x(), p[j + 1].y(), cv[j + 1], 
                a, b, c );

            x0 = p[j].x();
            y0 = p[j].y();
        }

        const double y = qwtCubicPolynom( x - x0, a, b, c, y0 );
        fittedPoints += QPointF( x, y );
    }

    return fittedPoints;
}

QPainterPath QwtSplineNatural::path( const QPolygonF &points )
{
    QPainterPath path;

    const int size = points.size();
    if ( size <= 2 )
    {
        path.addPolygon( points );
        return path;
    }


    const QVector<double> cv = curvatures( points );
    const QPointF *p = points.constData();

    path.moveTo( p[0] );
    for ( int i = 0; i < size - 1; i++ )
        qwtNaturalCubicTo( p[i], cv[i], p[i+1], cv[i+1], path );

    return path;
}

// ---

namespace QwtSpline
{
    struct Polynomial
    {
        double a;
        double b;
        double c;
        double d;
    };
}

static QVector<QwtSpline::Polynomial> 
    qwtSplinePolynomials( const QPolygonF &points, 
    const QVector<double> &sigma, double lambda )
{
    const int n = points.size();

    QVector<double> h(n-1, 0.0);
    QVector<double> p(n-1, 0.0);
    QVector<double> q(n, 0.0);

    const double mu = 2 * ( 1 - lambda ) / ( 3 * lambda );

    h[0] = points[1].x() - points[0].x();

    for ( int i = 1; i < n - 1; i++ )
    {
        h[i] = points[i+1].x() - points[i].x();
        p[i] = 2 * ( points[i+1].x() - points[i-1].x());
        q[i] = 3 * ( points[i+1].y() - points[i].y()) / h[i] - 3 * ( points[i].y() - points[i-1].y() ) / h[i-1];
    }

    // diagonals of the matrix: W + LAMBDA T' SIGMA T
    QVector<double> u(n - 1, 0.0);
    QVector<double> v(n - 1, 0.0);
    QVector<double> w(n - 2, 100.0);

#if 1
    u[0] = v[0] = w[0] = 0.0;
#endif
    {
        double dx0 = 3 / ( points[1].x() - points[0].x() );
        double dx1 = 3 / ( points[2].x() - points[1].x() );

        for ( int i = 1; i < n - 1; i++ )
        {
            const double ff1 = -( dx0 + dx1 );

            u[i] = qwtSqr(dx0) * sigma[i-1] + qwtSqr( ff1 ) * sigma[i] + qwtSqr(dx1) * sigma[i+1];
            u[i] = mu * u[i] + p[i];

            if ( i == n - 2 )
            {
                const double ff2 = -( dx1 + dx1 );

                v[i] = ff1 * dx1 * sigma[i] + dx1 * ff2 * sigma[i+1];
                v[i] = mu * v[i] + h[i];

                break;
            }

            const double dx2 = 3 / ( points[i+2].x() - points[i+1].x() );

            const double ff2 = -( dx1 + dx2 );
            v[i] = ff1 * dx1 * sigma[i] + dx1 * ff2 * sigma[i+1];
            v[i] = mu * v[i] + h[i];

            w[i] = mu * dx1 * dx2 * sigma[i+1];

            dx0 = dx1;
            dx1 = dx2;
        }
    }

    {
        // factorisation

        v[1] = v[1] / u[1];
        w[1] = w[1] / u[1];

        for ( int j = 2; j < n - 1; j++ )
        {
            u[j] = u[j] - u[j-2] * qwtSqr( w[j-2] ) - u[j-1] * qwtSqr( v[j-1] );
            v[j] = ( v[j] - u[j-1] * v[j-1] * w[j-1] ) / u[j];

            if ( j == n - 2 )
                break;

            w[j] = w[j] / u[j];
        }

        // forward substitution
        q[0] = 0.0;
        for ( int j = 2; j < n - 1; j++ )
            q[j] = q[j] - v[j - 1] * q[j - 1] - w[j - 2] * q [j - 2];

        for ( int j = 1; j < n - 1; j++ )
            q[j] = q[j] / u[j];

        // back substitution
        q[n-2] = q[n-2] - v[n-2] * q[n-1];
        for ( int j = n - 3; j >= 1; j-- )
            q[j] = q[j] - v[j] * q[j + 1] - w[j] * q[j + 2];
    }

    w.clear();

    // Spline Parameters

    QVector<QwtSpline::Polynomial> s( n - 1 );

    double dxx1 = 3 / ( points[1].x() - points[0].x() );
    double dxx2 = 3 / ( points[2].x() - points[1].x() );

    s[0].d = points[0].y() - mu * dxx1 * q[1] * sigma[0];

    const double ff0 = -( dxx1 + dxx2 );

    s[1].d = points[1].y() - mu * ( ff0 * q[1] + dxx2 * q[2]) * sigma[0];
    s[0].a = q[1] / ( 3 * h[0] );
    s[0].b = 0; 
    s[0].c = ( s[1].d - s[0].d ) / h[0] - q[1] * h[0] / 3;

    double dx1 = 0.0;
    for( int j = 1; j < n - 1; j++ )
    {
        const double dx2 = 3 / ( points[j+1].x() - points[j].x() );
        const double ff = -( dx1 + dx2 );

        s[j].a = ( q[j+1] - q[j]) / ( 3 * h[j] );
        s[j].b = q[j];
        s[j].c = ( q[j]+ q[j-1] ) * h[j-1] + s[j-1].c;
        s[j].d = dx1 * q[j-1] + ff * q[j] + dx2 * q[j+1];
        s[j].d = points[j].y() - mu * s[j].d * sigma[j];

        dx1 = dx2;
    }

    return s;
}

static QVector<QwtSpline::Polynomial> 
qwtSplinePolynomials( const QPolygonF &points, double lambda )
{
    QVector<double> sigma(points.size());
    for ( int i = 0; i < points.size(); i++ )
        sigma[i] = 1.0;

    return qwtSplinePolynomials( points, sigma, lambda );
}

QPolygonF QwtSpline::polygon( const QPolygonF &points, double lambda, int numPoints )
{
    const int size = points.size();
    if ( size <= 2 )
        return points;

    const QVector<Polynomial> polynomials = qwtSplinePolynomials( points, lambda );

    const QPointF *p = points.constData();

    const double x1 = points.first().x();
    const double x2 = points.last().x();
    const double delta = ( x2 - x1 ) / ( numPoints - 1 );

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
        }

        const QwtSpline::Polynomial &s = polynomials[j];
        const double y = qwtCubicPolynom( x - p[j].x(), s.a, s.b, s.c, s.d );

        // s.d == p[j].y(), when lambda: 1.0
        fittedPoints += QPointF( x, y );
    }

    return fittedPoints;
}

QPainterPath QwtSpline::path( const QPolygonF &points, double lambda )
{
    QPainterPath path;

    const int size = points.size();
    if ( size <= 2 )
    {
        path.addPolygon( points );
        return path;
    }

    const QVector<Polynomial> polynomials = qwtSplinePolynomials( points, lambda );

    const QPointF *p = points.constData();

    path.moveTo( p[0] );

    for ( int i = 0; i < size - 1; i++ )
    {
        const QPointF &p1 = p[i];
        const QPointF &p2 = p[i+1];
        const Polynomial s = polynomials[i];

        // a cubic polynomial can be translated into
        // a bezier curve

        const double dx = p2.x() - p1.x();
        const double step = dx / 3.0;

        const double cy1 = s.d + s.c * step;
        const double cy2 = cy1 + ( s.c + s.b * dx ) * step;
    
        const double y2 = size - 1 ? p2.y() : polynomials[i+1].d;

        path.cubicTo( p1.x() + step, cy1, 
            p2.x() - step, cy2, p2.x(), y2 );
    }

    return path;
}

QPainterPath QwtSplineAkima::path( const QPolygonF &points )
{
    QPainterPath path;

    const int size = points.size();
    if ( size <= 2 )
    {
        path.addPolygon( points );
        return path;
    }

    const QPointF *p = points.constData();

    double m4 = ( p[2].y() - p[1].y() ) / ( p[2].x() - p[1].x() );
    double m3 = ( p[1].y() - p[0].y() ) / ( p[1].x() - p[0].x() );
    double m2 = 2.0 * m3 - m4;
    double m1 = 3.0 * m3 - 2.0 * m4;

    path.moveTo( p[0] );

    for ( int i = 0; i < size - 1; i++ )
    {
        const QPointF &p1 = p[i];
        const QPointF &p2 = p[i+1];

        const double dx = p2.x() - p1.x();

        double m5;
        if ( i == size - 2 )
        {
            m5 = 3.0 * m4 - 2.0 * m3;
        }
        else if ( i == size - 3 )
        {
            m5 = 2.0 * m4 - m3;
        }
        else
        {
            m5 = ( p[i+3].y() - p[i+2].y() ) / ( p[i+3].x() - p[i+2].x() );
        }

        double b, c;

        const double d12 = qAbs( m2 - m1 );
        const double d34 = qAbs( m4 - m3 );

        const double ry = d34 + d12;
        if ( ry == 0.0 )
        {
            b = 0.0;
            c = m3;
        }
        else
        {
            const double d23 = qAbs( m3 - m2 );
            const double d45 = qAbs( m5 - m4 );

            double t;
            const double ry_next = d45 + d23;
            if ( ry_next == 0.0 )
            {
                t = m3;
            }
            else
            {
                double alpha_next = d23 / ry_next;
                t = ( 1.0 - alpha_next ) * m3 + alpha_next * m4;
            }

            const double alpha = d12 / ry;
            c = ( 1.0 - alpha ) * m2 + alpha * m3;
            b = ( 3.0 * m3 - 2.0 * c - t ) / dx;
        }

        m1 = m2;
        m2 = m3;
        m3 = m4;
        m4 = m5;

        qwtCubicTo( p1, p2, dx, b, c, path );
    }

    return path;
}

static inline void vectorX( const QPointF &p1, const QPointF &p2, const QPointF &p3,
    double m, double &vx1, double &vx2 )
{
    const double m0 = -1.0 / m;

    const double t0 = p2.y() - m * p2.x();
    const double t1 = p1.y() - m0 * p1.x();
    const double t2 = p3.y() - m0 * p3.x();

    vx1 = p2.x() - ( t1 - t0 ) / ( m - m0 );
    vx2 = ( t2 - t0 ) / ( m - m0 ) - p2.x();
}

QPainterPath QwtSplineHarmonicMean::path( const QPolygonF &points )
{
    QPainterPath path;

    const int size = points.size();
    if ( size <= 2 )
    {
        path.addPolygon( points );
        return path;
    }

    const QPointF *p = points.constData();

    path.moveTo( p[0] );

    double dx1 = p[1].x() - p[0].x();
    double dy1 = p[1].y() - p[0].y();

    const double dy2 = p[2].y() - p[1].y();

    double vx1 = dx1;
    double vy1 = 1.5 * dy1;

    if ( ( dy1 > 0 ) == ( dy2 > 0 ) )
    {
        const double dx2 = p[2].x() - p[1].x();
        vy1 -= dx1 / ( dx1 / dy1 + dx2 / dy2 );
    }

    for ( int i = 1; i < size - 1; i++ )
    {
        const double dx2 = p[i+1].x() - p[i].x();
        const double dy2 = p[i+1].y() - p[i].y();

        double vy3;

        if ( ( dx2 == 0 ) && ( dx1 == 0 ) )
        {
            if ( dy2 < 0 )
                vy3 = -vy3;

            path.cubicTo( p[i-1] + QPointF( vx1, vy1 ) / 3.0, p[i], p[i] );

            vx1 = 0.0;
        }
        else if ( ( dx1 >= 0 ) == ( dx2 >= 0 ) )
        {
            double vy2;

            if ( ( dy1 > 0 ) == ( dy2 > 0 ) )
            {
                const double m = 2 / ( dx1 / dy1 + dx2 / dy2 );
                vy2 = dx1 * m;
                vy3 = dx2 * m;
            }
            else
            {
                vy2 = vy3 = 0.0;
            }

            path.cubicTo( p[i-1] + QPointF( vx1, vy1 ) / 3.0, 
                p[i] - QPointF( dx1, vy2 ) / 3.0, p[i] );

            vx1 = dx2;
        }
        else
        {
            const double m1 = dy1 / dx1;
            const double m2 = dy2 / dx2;

            double m;

            if ( ( dy1 > 0 ) == ( dy2 > 0 ) )
            {
                m = -2.0 / ( m1 + m2 );

                const double m0 = ( qAbs( m1 ) < qAbs( m2 ) ) ? m1 : m0;
                if ( qAbs( m ) < qAbs( m0 ) )
                    m = m0;
            }
            else
            {
                m = -2.0 / ( m1 + m2 );
            }


            double vx2, vx3;
            vectorX( p[i-1], p[i], p[i+1], m, vx2, vx3 );

            path.cubicTo( p[i-1] + QPointF( vx1, vy1 ) / 3.0, 
                p[i] - QPointF( vx2, m * vx2 ) / 3.0, p[i] );

            vx1 = vx3;
            vy3 = m * vx1;
        }

        vy1 = vy3;
        dx1 = dx2;
        dy1 = dy2;
    }

    QPointF vec2 = QPointF( dx1, 0.5 * ( 3.0 * dy1 - vy1 ) );
    path.cubicTo( p[size - 2] + QPointF( vx1, vy1 ) / 3.0, 
        p[size - 1] - vec2 / 3.0, p[size - 1] );

    return path;
}
