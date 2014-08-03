/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_cardinal.h"

struct qwtParamX
{
    inline double operator()( const QPointF &p1, const QPointF &p2 ) const
    {
        return QwtSpline::parameterX( p1, p2 );
    }
};

struct qwtParamChordal
{
    inline double operator()( const QPointF &p1, const QPointF &p2 ) const
    {
        return QwtSpline::parameterChordal( p1, p2 );
    }
};

static inline QwtSplineCardinalG1::Tension qwtTension(
    double d13, double d23, double d24,
    const QPointF &p1, const QPointF &p2,
    const QPointF &p3, const QPointF &p4 )
{
    const bool b1 = ( d13 / 3.0 ) < d23;
    const bool b2 = ( d24 / 3.0 ) < d23;

	QwtSplineCardinalG1::Tension tension;
    if ( b1 & b2 )
    {
        tension.t1 = ( p1 != p2 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
        tension.t2 = ( p3 != p4 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
    }
    else
    {
        tension.t1 = d23 / ( b1 ? d24 : d13 );
        tension.t2 = d23 / ( b2 ? d13 : d24 );
    }

	return tension;
}

template< class Param >
static inline QVector<QwtSplineCardinalG1::Tension> qwtTensions( 
	const QPolygonF &points, Param param ) 
{
    const int size = points.size();
    QVector<QwtSplineCardinalG1::Tension> tensions2( size - 1 );

    const QPointF *p = points.constData();
    QwtSplineCardinalG1::Tension *t = tensions2.data();

    double d13 = param(p[0], p[2]);

    const double d0 = param(p[0], p[1]);
    t[0] = qwtTension( d0, d0, d13, p[0], p[0], p[1], p[2] );

    for ( int i = 1; i < size - 2; i++ )
    {
        const double d23 = param( p[i], p[i+1] );
        const double d24 = param( p[i], p[i+2] );

        t[i] = qwtTension( d13, d23, d24, p[i-1], p[i], p[i+1], p[i+2] );

        d13 = d24;
    }

    const double dn = param(p[size - 2], p[size - 1]);

    t[size-2] = qwtTension( d13, dn, dn,
        p[size - 3], p[size - 2], p[size - 1], p[size - 1] );

    return tensions2;
}

QwtSplineCardinalG1::QwtSplineCardinalG1()
{
}

QwtSplineCardinalG1::~QwtSplineCardinalG1()
{
}

QVector<QLineF> QwtSplineCardinalG1::bezierControlPointsP( const QPolygonF &points ) const
{
	const int size = points.size();

	const QVector<Tension> tensions2 = tensions( points );
	if ( tensions2.size() != size - 1 )
		return QVector<QLineF>();

	QVector<QLineF> controlPoints( size - 1 );

	const QPointF *p = points.constData();
	const Tension *t = tensions2.constData();

	QLineF *cp = controlPoints.data();

    QPointF vec1 = ( p[1] - p[0] ) * 0.5;
    for ( int i = 0; i < size - 2; i++ )
    {
        const QPointF vec2 = ( p[i+2] - p[i] ) * 0.5;
        
		cp[i].setPoints( p[i] + vec1 * t[i].t1,
			p[i+1] - vec2 * t[i].t2 );
        
        vec1 = vec2;
    }   
    
	const QPointF vec2 = 0.5 * ( p[size - 1] - p[size - 2] );

	cp[size-2].setPoints( p[size - 2] + vec1 * t[size-2].t1,
		p[size - 1] - vec2 * t[size-2].t2 );

	return controlPoints;
}

QwtSplinePleasing::QwtSplinePleasing()
{
}

QwtSplinePleasing::~QwtSplinePleasing()
{
}

QVector<QwtSplineCardinalG1::Tension> 
QwtSplinePleasing::tensions( const QPolygonF &points ) const
{
	QVector<Tension> tensions2;

	if ( points.size() <= 2 )
		return tensions2;

	if ( parametrization() == ParametrizationChordal )
		tensions2 = qwtTensions( points, qwtParamChordal() );
	else
		tensions2 = qwtTensions( points, qwtParamX() );

	return tensions2;
}
