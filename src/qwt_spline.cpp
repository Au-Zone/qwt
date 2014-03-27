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

QVector<double> QwtSplineNatural::quadraticCoefficients( const QPolygonF &points )
{
    const int size = points.size();

    QVector<double> aa0( size - 1 );
    QVector<double> bb0( size - 1 );

    {
        double dx1 = points[1].x() - points[0].x();
        double dy1 = ( points[1].y() - points[0].y() ) / dx1;

        for ( int i = 1; i < size - 1; i++ )
        {
            const double dx2 = points[i+1].x() - points[i].x();
            const double dy2 = ( points[i+1].y() - points[i].y() ) / dx2;

            aa0[i] = 2.0 * ( dx1 + dx2 );
            bb0[i] = 6.0 * ( dy1 - dy2 );

            dy1 = dy2;
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

#if 1
	// todo: do it in the lopp above
	for ( int i = 1; i < size - 1; i++ )
		s[i] *= 0.5;
#endif

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

    const QVector<double> b = quadraticCoefficients( points );

    const QPointF *p = points.constData();

    const double x1 = points.first().x();
    const double x2 = points.last().x();
    const double delta = ( x2 - x1 ) / ( numPoints - 1 );

    double a, c, x0, y0;

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

            coefficients( p[j], p[j + 1], b[j], b[j+1], a, c );
            x0 = p[j].x();
            y0 = p[j].y();
        }

        const double y = qwtCubicPolynom( x - x0, a, b[j], c );
        fittedPoints += QPointF( x, y0 + y );
    }

    return fittedPoints;
}

// ---

struct Spline
{
	double a;
	double b;
	double c;
	double d;
};

static QVector<Spline> qwtCoefficients( const QPolygonF &points, 
	const QVector<double> &sigma, double lambda )
{
	const int n = points.size();

	QVector<double> h(n-1, 0.0);
	QVector<double> p(n-1, 0.0);
	QVector<double> q(n, 0.0);

	QVector<double> ff(n, 0.0);
	QVector<double> rr(n, 0.0);

	const double mu = 2 * ( 1 - lambda ) / ( 3 * lambda );
	h[0] = points[1].x() - points[0].x();
	rr[0] = 3 / h[0];

	for ( int i = 1; i < n - 1; i++ )
	{
		h[i] = points[i+1].x() - points[i].x();
		rr[i] = 3 / h[i];
		ff[i] = -( rr[i-1] + rr[i]);
		p[i] = 2 * ( points[i+1].x() - points[i-1].x());
		q[i] = 3 * ( points[i+1].y() - points[i].y()) / h[i] - 3 * ( points[i].y() - points[i-1].y() ) / h[i-1];
	}

	// diagonals of the matrix: W + LAMBDA T' SIGMA T
	QVector<double> u(n, 0.0);
	QVector<double> v(n, 0.0);
	QVector<double> w(n, 0.0);

#if 1
	u[0] = v[0] = w[0] = 0.0;
#endif
	for ( int i = 1; i < n - 1; i++ )
	{
		u[i] = qwtSqr(rr[i-1]) * sigma[i-1] + qwtSqr(ff[i]) * sigma[i] + qwtSqr(rr[i]) * sigma[i+1];
		u[i] = mu * u[i] + p[i];
		v[i] = ff[i] * rr[i] * sigma[i] + rr[i] * ff[i+1] * sigma[i+1];
		v[i] = mu * v[i] + h[i];
		w[i] = mu * rr[i] * rr[i+1] * sigma[i+1];
	}

	{
		// factorisation

		v[1] = v[1] / u[1];
		w[1] = w[1] / u[1];

		for ( int j = 2; j < n - 1; j++ )
		{
			u[j] = u[j] - u[j-2] * qwtSqr( w[j-2] ) - u[j-1] * qwtSqr( v[j-1] );
			v[j] = ( v[j] - u[j-1] * v[j-1] * w[j-1] ) / u[j];
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
		for ( int j = n - 3; j > 1; j-- )
			q[j] = q[j] - v[j] * q[j + 1] - w[j] * q[j + 2];
	}

	// Spline Parameters

	QVector<Spline> s( n - 1 );

	s[0].d = points[0].y() - mu * rr[0] * q[1] * sigma[0];
	s[1].d = points[1].y() - mu * ( ff[1] * q[1] + rr[1] * q[2]) * sigma[0];
	s[0].a = q[1] / ( 3 * h[0] );
	s[0].b = 0; 
	s[0].c = ( s[1].d - s[0].d ) / h[0] - q[1] * h[0] / 3;

	rr[0] = 0;
	for( int j = 1; j < n - 1; j++ )
	{
		s[j].a = ( q[j+1] - q[j]) / ( 3 * h[j] );
		s[j].b = q[j];
		s[j].c = ( q[j]+ q[j-1] ) * h[j-1] + s[j-1].c;
		s[j].d = rr[j-1] * q[j-1] + ff[j] * q[j]+ rr[j] * q[j+1];
		s[j].d = points[j].y() - mu * s[j].d * sigma[j];
	}

	return s;
}

static QVector<Spline> qwtCoefficients( const QPolygonF &points, double lambda )
{
	QVector<double> sigma(points.size());
	for ( int i = 0; i < points.size(); i++ )
		sigma[i] = 1.0;

	return qwtCoefficients( points, sigma, lambda );
}

QPolygonF QwtSpline::polygon( const QPolygonF &points, double lambda, int numPoints )
{
    const int size = points.size();
    if ( size <= 2 )
        return points;

	const QVector<Spline> splines = qwtCoefficients( points, lambda );

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

        const double dx = x - p[j].x();

		const Spline &s = splines[j];
        const double y = ( ( ( s.a * dx ) + s.b ) * dx + s.c ) * dx + s.d;

        fittedPoints += QPointF( x, y );
    }

    return fittedPoints;
}

