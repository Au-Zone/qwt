/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_local.h"
#include <qmath.h>

static inline double qwtSlopeP( const QPointF &p1, const QPointF &p2 )
{
    // ???
    const double dx = p2.x() - p1.x();
    return dx ? ( p2.y() - p1.y() ) / dx : 0.0;
}

static inline void qwtCubicToP( const QPointF &p1, double m1,
    const QPointF &p2, double m2, QPainterPath &path )
{
    const double dx3 = ( p2.x() - p1.x() ) / 3.0;

    path.cubicTo( p1.x() + dx3, p1.y() + m1 * dx3,
        p2.x() - dx3, p2.y() - m2 * dx3,
        p2.x(), p2.y() );
}

namespace QwtSplineLocalP
{
    class PathStore
    {
    public:
        inline void init( int )
        {
        }

        inline void start( const QPointF &p0, double )
        {
            path.moveTo( p0 );
        }

        inline void addCubic( const QPointF &p1, double m1,
            const QPointF &p2, double m2 )
        {
            qwtCubicToP( p1, m1, p2, m2, path );
        }

        QPainterPath path;
    };

    class ControlPointsStore
    {
    public:
        inline void init( int size )
        {
            controlPoints.resize( size );
            d_cp = controlPoints.data();
        }

        inline void start( const QPointF &, double )
        {
        }

        inline void addCubic( const QPointF &p1, double m1,
            const QPointF &p2, double m2 )
        {
            const double dx3 = ( p2.x() - p1.x() ) / 3.0;

            QLineF &l = *d_cp++;
            l.setLine( p1.x() + dx3, p1.y() + m1 * dx3, 
                p2.x() - dx3, p2.y() - m2 * dx3 );
        }

        QVector<QLineF> controlPoints;

    private:
        QLineF *d_cp;
    };

    class SlopeStore
    {
    public:
        void init( int size )
        {
            slopes.resize( size );
            d_m = slopes.data();
        }

        inline void start( const QPointF &, double m0 )
        {   
            *d_m++ = m0;
        }

        inline void addCubic( const QPointF &, double,
            const QPointF &, double m2 )
        {
            *d_m++ = m2;
        }

        QVector<double> slopes;

    private:
        double *d_m;
    };
};
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
            return 2.0 / ( 1.0 / s1 + 1.0 / s2 );
    }

    return 0.0;
}

static inline void qwtLocalEndpoints( const QPolygonF &points,
    QwtSplineLocal::Type type, double tension, double &slopeStart, double &slopeEnd )
{
    slopeStart = slopeEnd = 0.0;

    const int n = points.size();
    if ( type == QwtSplineLocal::HarmonicMean )
    {
        if ( n >= 3 )
        {
            const double s1 = qwtSlopeP( points[0], points[1] );
            const double s2 = qwtSlopeP( points[1], points[2] );
            const double s3 = qwtSlopeP( points[n-3], points[n-2] );
            const double s4 = qwtSlopeP( points[n-2], points[n-1] );

            slopeStart = 1.5 * s1 - 0.5 * qwtHarmonicMean( s1, s2 );
            slopeEnd = 1.5 * s4 - 0.5 * qwtHarmonicMean( s3, s4 );
        }
    }
    else
    {
        if ( n >= 2 )
        {
            slopeStart = qwtSlopeP( points[0], points[1] );
            slopeEnd = qwtSlopeP( points[n-2], points[n-1] );
        }
    }

    slopeStart *= ( 1.0 - tension );
    slopeEnd *= ( 1.0 - tension );
}

template< class SplineStore >
static inline SplineStore qwtSplinePathCardinal( const QPolygonF &points,
    double tension, double slopeStart, double slopeEnd )
{
    const double s = 1.0 - tension;
    const int size = points.size();

    const QPointF *p = points.constData();

    SplineStore store;
    store.init( size );
    store.start( p[0], slopeStart );

    double m1 = slopeStart;
    for ( int i = 1; i < size - 1; i++ )
    {
        const double m2 = s * qwtSlopeP( p[i-1], p[i+1] );
        store.addCubic( p[i-1], m1, p[i], m2 );

        m1 = m2;
    }

    store.addCubic( p[size-2], m1, p[size-1], slopeEnd );

    return store;
}

template< class SplineStore >
static inline SplineStore qwtSplinePathAkima( const QPolygonF &points,
    double tension, double slopeStart, double slopeEnd )
{
    const double s = 1.0 - tension;

    const int size = points.size();
    const QPointF *p = points.constData();

    SplineStore store;
    store.init( size );
    store.start( p[0], slopeStart );

    double slope1 = slopeStart;
    double slope2 = qwtSlopeP( p[0], p[1] );
    double slope3 = qwtSlopeP( p[1], p[2] );

    double m1 = s * slope1;

    for ( int i = 0; i < size - 3; i++ )
    {
        const double slope4 = qwtSlopeP( p[i+2],  p[i+3] );

        const double m2 = s * qwtAkima( slope1, slope2, slope3, slope4 );
        store.addCubic( p[i], m1, p[i+1], m2 );

        slope1 = slope2;
        slope2 = slope3;
        slope3 = slope4;

        m1 = m2;
    }

    const double m2 = s * qwtAkima( slope1, slope2, slope3, slopeEnd );

    store.addCubic( p[size - 3], m1, p[size - 2], m2 );
    store.addCubic( p[size - 2], m2, p[size - 1], slopeEnd );

    return store;
}

template< class SplineStore >
static inline SplineStore qwtSplinePathHarmonicMean( const QPolygonF &points,
    double tension, double slopeStart, double slopeEnd )
{
    const double s = 1.0 - tension;

    const int size = points.size();
    const QPointF *p = points.constData();

    SplineStore store;
    store.init( size );
    store.start( p[0], slopeStart );

    double dx1 = p[1].x() - p[0].x();
    double dy1 = p[1].y() - p[0].y();
    double m1 = slopeStart;

    for ( int i = 1; i < size - 1; i++ )
    {
        const double dx2 = p[i+1].x() - p[i].x();
        const double dy2 = p[i+1].y() - p[i].y();

        const double m2 = s * qwtHarmonicMean( dx1, dy1, dx2, dy2 );

        store.addCubic( p[i-1], m1, p[i], m2 );

        dx1 = dx2;
        dy1 = dy2;
        m1 = m2;
    }

    store.addCubic( p[size - 2], m1, p[size - 1], slopeEnd );

    return store; 
}

template< class SplineStore >
static inline SplineStore qwtSplinePathLocal( int type, 
    const QPolygonF &points, double tension, double slopeStart, double slopeEnd )
{   
    SplineStore store;
    
    switch( type )
    {   
        case QwtSplineLocal::Cardinal:
        {   
            store = qwtSplinePathCardinal<SplineStore>(
                points, tension, slopeStart, slopeEnd );
            break;
        }
        case QwtSplineLocal::ParabolicBlending:
        {   
            // Bessel
            break;
        }
        case QwtSplineLocal::Akima:
        {   
            store = qwtSplinePathAkima<SplineStore>(
                points, tension, slopeStart, slopeEnd );
            break;
        }
        case QwtSplineLocal::HarmonicMean:
        {   
            store = qwtSplinePathHarmonicMean<SplineStore>(
                points, tension, slopeStart, slopeEnd );
            break;
        }
        case QwtSplineLocal::PChip:
        {   
            break;
        }
        default:
            break;
    }

    return store;
}

QwtSplineLocal::QwtSplineLocal( Type type, double tension ):
    d_type( type ),
    d_tension( 0.0 )
{
    setTension( tension );
}

QwtSplineLocal::~QwtSplineLocal()
{
}

void QwtSplineLocal::setTension( double tension )
{
    d_tension = qBound( 0.0, tension, 1.0 );
}

double QwtSplineLocal::tension() const
{
    return d_tension;
}

QPainterPath QwtSplineLocal::pathP( const QPolygonF &points ) const
{
    if ( parametrization()->type() == QwtSplineParameter::ParameterX )
        return pathX( points );

    return QwtSplineC1::pathP( points );
}

QVector<QLineF> QwtSplineLocal::bezierControlPointsP( const QPolygonF &points ) const
{
    if ( parametrization()->type() == QwtSplineParameter::ParameterX )
    {   
        return bezierControlPointsX( points );
    }

    return QwtSplineC1::bezierControlPointsP( points );
}

QPainterPath QwtSplineLocal::pathX( const QPolygonF &points ) const
{
    QPainterPath path;

    const int size = points.size();
    if ( size == 0 )
        return path;

    if ( size == 1 )
    {
        path.moveTo( points[0] );
        return path;
    }

    double slopeStart, slopeEnd;
    qwtLocalEndpoints( points, d_type, d_tension, slopeStart, slopeEnd );
    
    if ( size == 2 )
    {
        path.moveTo( points[0] );
        qwtCubicToP( points[0], slopeStart, points[1], slopeEnd, path );

        return path;
    }

    using namespace QwtSplineLocalP;

    const PathStore store = qwtSplinePathLocal<PathStore>(
        d_type, points, d_tension, slopeStart, slopeEnd );

    return store.path;
}
    
QVector<QLineF> QwtSplineLocal::bezierControlPointsX( const QPolygonF &points ) const
{
    QVector<QLineF> controlPoints;

    const int size = points.size();
    if ( size <= 2 )
        return controlPoints;

    using namespace QwtSplineLocalP;

    double slopeStart, slopeEnd;
    qwtLocalEndpoints( points, d_type, d_tension, slopeStart, slopeEnd );

    const ControlPointsStore store = qwtSplinePathLocal<ControlPointsStore>(
        d_type, points, d_tension, slopeStart, slopeEnd );

    return store.controlPoints;
}

QVector<double> QwtSplineLocal::slopesX( const QPolygonF &points ) const
{
    const int size = points.size();
    if ( size <= 1 )
        return QVector<double>();

    double slopeStart, slopeEnd;
    qwtLocalEndpoints( points, d_type, d_tension, slopeStart, slopeEnd );

    if ( size == 2 )
    {
        QVector<double> m(2);
        m[0] = slopeStart;
        m[1] = slopeEnd;

        return m;
    }

    using namespace QwtSplineLocalP;

    const SlopeStore store = qwtSplinePathLocal<SlopeStore>(
        d_type, points, d_tension, slopeStart, slopeEnd );

    return store.slopes;
}
