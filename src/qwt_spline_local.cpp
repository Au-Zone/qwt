/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_local.h"
#include "qwt_spline_parametrization.h"
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
        inline void init( const QPolygonF & )
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
        inline void init( const QPolygonF &points )
        {
            if ( points.size() > 0 )
                controlPoints.resize( points.size() - 1 );
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
        void init( const QPolygonF &points )
        {
            slopes.resize( points.size() );
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

static inline double qwtSlopeParabolicBlending(
    double dx01, double s1, double dx12, double s2 )
{
    return ( dx12 * s1 + dx01 * s2 ) / ( dx01 + dx12 );
}

static inline bool qwtIsStrictlyMonotonic( double dy1, double dy2 )
{
    if ( dy1 == 0.0 || dy2 == 0.0 )
        return false;

    return ( dy1 > 0.0 ) == ( dy2 > 0.0 );
}

static inline double qwtSlopeAkima( double s1, double s2, double s3, double s4 )
{
    if ( ( s1 == s2 ) && ( s3 == s4 ) )
    {
        return 0.5 * ( s2 + s3 );
    }

    const double ds12 = qAbs( s2 - s1 );
    const double ds34 = qAbs( s4 - s3 );

    return ( s2 * ds34 + s3 * ds12 ) / ( ds12 + ds34 );
}

static inline double qwtSlopePChip(
    double dx1, double dy1, double s1, double dx2, double dy2, double s2 )
{
    if ( qwtIsStrictlyMonotonic( dy1, dy2 ) )
    {
#if 0
        // weighting the slopes by the dx1/dx2
        const double w1 = ( 3 * dx1 + 3 * dx2 ) / ( 2 * dx1 + 4 * dx2 );
        const double w2 = ( 3 * dx1 + 3 * dx2 ) / ( 4 * dx1 + 2 * dx2 );

        s1 *= w1;
        s2 *= w2;

        // harmonic mean ( see https://en.wikipedia.org/wiki/Pythagorean_means )
        return 2.0 / ( 1.0 / s1 + 1.0 / s2 );
#endif
        // the same as above - but faster

        const double s12 = ( dy1 + dy2 ) / ( dx1 + dx2 );
        return 3.0 * ( s1 * s2 ) / ( s1 + s2 + s12 );
    }

    return 0.0;
}

static inline double qwtSlopePChip(
    const QPointF &p1, const QPointF &p2, const QPointF &p3 )
{
    const double dx1 = p2.x() - p1.x();
    const double dy1 = p2.y() - p1.y();
    const double dx2 = p3.x() - p2.x();
    const double dy2 = p3.y() - p2.y();

    return qwtSlopePChip( dx1, dy1, dy1 / dx1, dx2, dy2, dy2 / dx2 );
}

static inline double qwtSlopeCardinal( 
    const QPointF &p1, const QPointF &p2, const QPointF &p3 )
{
    Q_UNUSED( p2 )
    return qwtSlopeP( p1, p3 );
}

static inline double qwtSlopeParabolicBlending( 
    const QPointF &p1, const QPointF &p2, const QPointF &p3 )
{
    const double dx1 = p2.x() - p1.x();
    const double s1 = ( p2.y() - p1.y() ) / dx1;

    const double dx2 = p3.x() - p2.x();
    const double s2 = ( p3.y() - p2.y() ) / dx2;

    return qwtSlopeParabolicBlending( dx1, s1, dx2, s2 );
}

static inline double qwtSlopeAkima( const QPointF &p1, const QPointF &p2, 
    const QPointF &p3, const QPointF &p4, const QPointF &p5 )
{
    const double s1 = qwtSlopeP( p1, p2 );
    const double s2 = qwtSlopeP( p2, p3 );
    const double s3 = qwtSlopeP( p3, p4 );
    const double s4 = qwtSlopeP( p4, p5 );

    return qwtSlopeAkima( s1, s2, s3, s4 );
}

static double qwtSlopeBegin( const QwtSplineC1 *spline,
    const QPolygonF &points, double slope1, double slope2 ) 
{
    const int size = points.size();
    if ( size < 2 )
        return 0.0;

    const double boundaryValue = spline->boundaryValueBegin();

    if ( spline->boundaryCondition() == QwtSplineC1::Clamped )
        return boundaryValue;

    const double dx = points[1].x() - points[0].x();
    const double dy = points[1].y() - points[0].y();

    if ( spline->boundaryCondition() == QwtSplineC1::LinearRunout )
    {
        const double s = dy / dx;
        return s - boundaryValue * ( s - slope1 );
    }

    const QwtSplinePolynomial pnom = 
        QwtSplinePolynomial::fromSlopes( points[1], slope1, points[2], slope2 ); 

    const double cv2 = pnom.curvatureAt( 0.0 );

    double cv1;
    switch( spline->boundaryCondition() )
    {
        case QwtSplineC1::Clamped2:
        {
            cv1 = boundaryValue;
            break;
        }
        case QwtSplineC1::Clamped3:
        {
            cv1 = cv2 - 6 * boundaryValue;
            break;
        }
        case QwtSplineC1::NotAKnot:
        {
            cv1 = cv2 - 6 * pnom.c3;
            break;
        }
        case QwtSplineC1::ParabolicRunout:
        {
            cv1 = cv2;
            break;
        }
        case QwtSplineC1::CubicRunout:
        {
            cv1 = 2 * cv2 - pnom.curvatureAt( 1.0 );
            break;
        }
        case QwtSplineC1::Natural:
        default:
            cv1 = 0.0;
    }


    const QwtSplinePolynomial pnomBegin =
        QwtSplinePolynomial::fromCurvatures( dx, dy, cv1, cv2 );

    return pnomBegin.slopeAt( 0.0 );
}

static double qwtSlopeEnd( const QwtSplineC1 *spline,
    const QPolygonF &points, double slope1, double slope2 ) 
{
    const int size = points.size();
    if ( size < 2 )
        return 0.0;

    const double boundaryValue = spline->boundaryValueEnd();

    if ( spline->boundaryCondition() == QwtSplineC1::Clamped )
        return boundaryValue;

    const double dx = points[size-1].x() - points[size-2].x();
    const double dy = points[size-1].y() - points[size-2].y();

    if ( spline->boundaryCondition() == QwtSplineC1::LinearRunout )
    {
        const double s = dy / dx;
        return s - boundaryValue * ( s - slope1 );
    }

    const QwtSplinePolynomial pnom = QwtSplinePolynomial::fromSlopes(
         points[size-3], slope1, points[size-2], slope2 ); 

    const double cv1 = pnom.curvatureAt( points[size-2].x() - points[size-3].x() );

    double cv2;
    switch( spline->boundaryCondition() )
    {
        case QwtSplineC1::Clamped2:
        {
            cv2 = boundaryValue;
            break;
        }
        case QwtSplineC1::NotAKnot:
        {
            cv2 = cv1 - 6 * pnom.c3;
            break;
        }
        case QwtSplineC1::Clamped3:
        {
            cv2 = cv1 - 6 * boundaryValue;
            break;
        }
        case QwtSplineC1::ParabolicRunout:
        {
            cv2 = cv1;
            break;
        }
        case QwtSplineC1::CubicRunout:
        {
            cv2 = 2 * cv1 - pnom.curvatureAt( 0.0 );
            break;
        }
        case QwtSplineC1::Natural:
        default:
            cv2 = 0.0;
    }

    const QwtSplinePolynomial pnomEnd = 
        QwtSplinePolynomial::fromCurvatures( dx, dy, cv1, cv2 );

    return pnomEnd.slopeAt( dx );
}

static inline void qwtSplineParabolicBlendingBoundaries(
    const QwtSplineLocal *spline, const QPolygonF &points,
    double &slopeBegin, double &slopeEnd )
{
    const int n = points.size();
    const QPointF *p = points.constData();

    if ( spline->isClosing()
        || ( spline->boundaryCondition() == QwtSplineC1::Periodic ) )
    {
        const QPointF pn = p[0] - ( p[n-1] - p[n-2] );
        slopeBegin = slopeEnd = qwtSlopeParabolicBlending( pn, p[0], p[1] );
        return;
    }

    if ( n == 3 )
    {
        const double s0 = qwtSlopeP( p[0], p[1] );
        const double m2 = qwtSlopeParabolicBlending( p[0], p[1], p[2] );

        slopeEnd = qwtSlopeEnd( spline, points, s0, m2 );
        slopeBegin = qwtSlopeBegin( spline, points, m2, slopeEnd );
    }
    else
    {
        const double m2 = qwtSlopeParabolicBlending( p[0], p[1], p[2] );
        const double m3 = qwtSlopeParabolicBlending( p[1], p[2], p[3] );
        slopeBegin = qwtSlopeBegin( spline, points, m2, m3 );

        const double mn1 = qwtSlopeParabolicBlending( p[n-4], p[n-3], p[n-2] );
        const double mn2 = qwtSlopeParabolicBlending( p[n-3], p[n-2], p[n-1] );
        slopeEnd = qwtSlopeEnd( spline, points, mn1, mn2 );
    }
}

template< class SplineStore >
static inline SplineStore qwtSplineParabolicBlending(
    const QwtSplineLocal *spline, const QPolygonF &points )
{
    Q_UNUSED( spline )

    const int size = points.size();
    const QPointF *p = points.constData();

    double slopeBegin, slopeEnd; 
    qwtSplineParabolicBlendingBoundaries( spline, points, slopeBegin, slopeEnd );

    const double ts = 1.0 - spline->tension();
    double m1 = ts * slopeBegin;

    SplineStore store;
    store.init( points );
    store.start( p[0], m1 );

    double dx01 = p[1].x() - p[0].x();
    double s1 = ( p[1].y() - p[0].y() ) / dx01;

    for ( int i = 1; i < size - 1; i++ )
    {
        const double dx12 = p[i+1].x() - p[i].x();
        const double s2 = ( p[i+1].y() - p[i].y() ) / dx12;

        const double m2 = ts * qwtSlopeParabolicBlending( dx01, s1, dx12, s2 );

        store.addCubic( p[i-1], m1, p[i], m2 );

        dx01 = dx12;
        s1 = s2;
        m1 = m2;
    }

    store.addCubic( p[size-2], m1, p[size-1], ts * slopeEnd );

    return store;
}

static inline void qwtSplineCardinalBoundaries( 
    const QwtSplineLocal *spline, const QPolygonF &points,
    double &slopeBegin, double &slopeEnd )
{
    const int n = points.size();
    const QPointF *p = points.constData();

    if ( spline->isClosing()
        || ( spline->boundaryCondition() == QwtSplineC1::Periodic ) )
    {
        const QPointF pn = p[0] - ( p[n-1] - p[n-2] );
        slopeBegin = slopeEnd = qwtSlopeP( pn, p[1] );
        return;
    }

    const double m2 = qwtSlopeP( p[0], p[2] );

    if ( n == 3 )
    {
        const double s0 = qwtSlopeP( p[0], p[1] );
        slopeEnd = qwtSlopeEnd( spline, points, s0, m2 );
        slopeBegin = qwtSlopeBegin( spline, points, m2, slopeEnd );
    }
    else
    {
        const double m3 = qwtSlopeP( p[1], p[3] );
        slopeBegin = qwtSlopeBegin( spline, points, m2, m3 );

        const double mn1 = qwtSlopeP( p[n-4], p[n-2] );
        const double mn2 = qwtSlopeP( p[n-3], p[n-1] );

        slopeEnd = qwtSlopeEnd( spline, points, mn1, mn2 );
    }
}

template< class SplineStore >
static inline SplineStore qwtSplineCardinal(
    const QwtSplineLocal *spline, const QPolygonF &points )
{
    const int size = points.size();
    const QPointF *p = points.constData();

    double slopeBegin, slopeEnd;
    qwtSplineCardinalBoundaries( spline, points, slopeBegin, slopeEnd );

    const double ts = 1.0 - spline->tension();
    double m1 = ts * slopeBegin;

    SplineStore store;
    store.init( points );
    store.start( p[0], m1 );

    for ( int i = 1; i < size - 1; i++ )
    {
        const double m2 = ts * qwtSlopeP( p[i-1], p[i+1] );
        store.addCubic( p[i-1], m1, p[i], m2 );

        m1 = m2;
    }

    store.addCubic( p[size-2], m1, p[size-1], ts * slopeEnd );

    return store;
}

static inline void qwtSplineAkimaBoundaries(
    const QwtSplineLocal *spline, const QPolygonF &points,
    double &slopeBegin, double &slopeEnd )
{
    const int n = points.size();
    const QPointF *p = points.constData();

    if ( spline->isClosing()
        || ( spline->boundaryCondition() == QwtSplineC1::Periodic ) )
    {
        const QPointF p2 = p[0] - ( p[n-1] - p[n-2] );
        const QPointF p1 = p2 - ( p[n-2] - p[n-3] );

        slopeBegin = slopeEnd = qwtSlopeAkima( p1, p2, p[0], p[1], p[2] );

        return;
    }

    if ( spline->boundaryCondition() == QwtSplineC1::Clamped )
    {
        slopeBegin = spline->boundaryValueBegin();
        slopeEnd = spline->boundaryValueEnd();

        return;
    }

    if ( n == 3 )
    {
        const double s1 = qwtSlopeP( p[0], p[1] );
        const double s2 = qwtSlopeP( p[1], p[2] );
        const double m = qwtSlopeAkima( s1, s1, s2, s2 );

        slopeBegin = qwtSlopeBegin( spline, points, m, s2 );
        slopeEnd = qwtSlopeEnd( spline, points, slopeBegin, m );
    }
    else if ( n == 4 )
    {
        const double s1 = qwtSlopeP( p[0], p[1] );
        const double s2 = qwtSlopeP( p[1], p[2] );
        const double s3 = qwtSlopeP( p[2], p[3] );

        const double m2 = qwtSlopeAkima( s1, s1, s2, s2 );
        const double m3 = qwtSlopeAkima( s2, s2, s3, s3 );

        slopeBegin = qwtSlopeBegin( spline, points, m2, m3 );
        slopeEnd = qwtSlopeEnd( spline, points, m2, m3 );
    }
    else
    {
        double s[4];

        for ( int i = 0; i < 4; i++ )
        {
            s[i] = qwtSlopeP( p[i], p[i+1] );
        }

        const double m2 = qwtSlopeAkima( s[0], s[0], s[1], s[1] );
        const double m3 = qwtSlopeAkima( s[0], s[1], s[2], s[3] );

        slopeBegin = qwtSlopeBegin( spline, points, m2, m3 );

        for ( int i = 0; i < 4; i++ )
        {
            const int j = n - 5 + i;
            s[i] = qwtSlopeP( p[j], p[j+1] );
        }

        const double mn1 = qwtSlopeAkima( s[0], s[1], s[2], s[3] );
        const double mn2 = qwtSlopeAkima( s[2], s[2], s[3], s[3] );

        slopeEnd = qwtSlopeEnd( spline, points, mn1, mn2 );
    }
}

template< class SplineStore >
static inline SplineStore qwtSplineAkima(
    const QwtSplineLocal *spline, const QPolygonF &points )
{
    const int size = points.size();
    const QPointF *p = points.constData();

    double slopeBegin, slopeEnd;
    qwtSplineAkimaBoundaries( spline, points, slopeBegin, slopeEnd );

    const double ts = 1.0 - spline->tension();
    double m1 = ts * slopeBegin;

    SplineStore store;
    store.init( points );
    store.start( p[0], m1 );

    double s1 = slopeBegin;
    double s2 = qwtSlopeP( p[0], p[1] );
    double s3 = qwtSlopeP( p[1], p[2] );

    for ( int i = 0; i < size - 3; i++ )
    {
        const double s4 = qwtSlopeP( p[i+2],  p[i+3] );

        const double m2 = ts * qwtSlopeAkima( s1, s2, s3, s4 );
        store.addCubic( p[i], m1, p[i+1], m2 );

        s1 = s2;
        s2 = s3;
        s3 = s4;

        m1 = m2;
    }

    const double m2 = ts * qwtSlopeAkima( s1, s2, s3, slopeEnd );

    store.addCubic( p[size - 3], m1, p[size - 2], m2 );
    store.addCubic( p[size - 2], m2, p[size - 1], ts * slopeEnd );

    return store;
}

static inline void qwtSplinePchipBoundaries(
    const QwtSplineLocal *spline, const QPolygonF &points,
    double &slopeBegin, double &slopeEnd )
{
    const int n = points.size();
    const QPointF *p = points.constData();

    if ( spline->isClosing()
        || ( spline->boundaryCondition() == QwtSplineC1::Periodic ) )
    {
        const QPointF pn = p[0] - ( p[n-1] - p[n-2] );
        slopeBegin = slopeEnd = qwtSlopePChip( pn, p[0], p[1] );
    }
    else if ( n == 3 )
    {
        const double s0 = qwtSlopeP( p[0], p[1] );
        const double m2 = qwtSlopePChip( p[0], p[1], p[2] );

        slopeEnd = qwtSlopeEnd( spline, points, s0, m2 );
        slopeBegin = qwtSlopeBegin( spline, points, m2, slopeEnd );
    }
    else
    {
        const double m2 = qwtSlopePChip( p[0], p[1], p[2] );
        const double m3 = qwtSlopePChip( p[1], p[2], p[3] );

        const double mn1 = qwtSlopePChip( p[n-4], p[n-3], p[n-2] );
        const double mn2 = qwtSlopePChip( p[n-3], p[n-2], p[n-1] );

        slopeBegin = qwtSlopeBegin( spline, points, m2, m3 );
        slopeEnd = qwtSlopeEnd( spline, points, mn1, mn2 );
    }
}

template< class SplineStore >
static inline SplineStore qwtSplinePchip( const QwtSplineLocal *spline,
    const QPolygonF &points )
{
    const int size = points.size();
    const QPointF *p = points.constData();

    double slopeBegin, slopeEnd;
    qwtSplinePchipBoundaries( spline, points, slopeBegin, slopeEnd );

    const double ts = 1.0 - spline->tension();
    double m1 = ts * slopeBegin;

    SplineStore store;
    store.init( points );
    store.start( p[0], ts * slopeBegin );

    double dx1 = p[1].x() - p[0].x();
    double dy1 = p[1].y() - p[0].y();
    double s1 = dy1 / dx1;

    for ( int i = 1 ; i < size - 1; i++ )
    {
        const double dx2 = p[i+1].x() - p[i].x() ;
        const double dy2 = p[i+1].y() - p[i].y() ;
        const double s2 = dy2 / dx2;

        double m2 = ts * qwtSlopePChip( dx1, dy1, s1, dx2, dy2, s2 );

        store.addCubic( p[i-1], m1, p[i], m2 );

        dx1 = dx2;
        dy1 = dy2;
        s1 = s2;
        m1 = m2;
    }

    store.addCubic( p[size-2], m1, p[size-1], ts * slopeEnd );

    return store;
}

template< class SplineStore >
static inline SplineStore qwtSplineLocal( 
    const QwtSplineLocal *spline, const QPolygonF &points )
{   
    SplineStore store;

    const int size = points.size();
    if ( size <= 1 )
        return store;

    if ( size == 2 )
    {
        const double ts = 1.0 - spline->tension();

        const double s0 = qwtSlopeP( points[0], points[1] );
        const double m1 = qwtSlopeBegin( spline, points, s0, s0 ) * ts;
        const double m2 = qwtSlopeEnd( spline, points, s0, s0 ) * ts;

        store.init( points );
        store.start( points[0], m1 );
        store.addCubic( points[0], m1, points[1], m2 );

        return store;
    }

    switch( spline->type() )
    {   
        case QwtSplineLocal::Cardinal:
        {   
            store = qwtSplineCardinal<SplineStore>( spline, points );
            break;
        }
        case QwtSplineLocal::ParabolicBlending:
        {   
            store = qwtSplineParabolicBlending<SplineStore>( spline, points );
            break;
        }
        case QwtSplineLocal::Akima:
        {   
            store = qwtSplineAkima<SplineStore>( spline, points );
            break;
        }
        case QwtSplineLocal::PChip:
        {   
            store = qwtSplinePchip<SplineStore>( spline, points );
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
    setBoundaryConditions( QwtSplineLocal::LinearRunout );
    setBoundaryValues( 0.0, 0.0 );
}

QwtSplineLocal::~QwtSplineLocal()
{
}

QwtSplineLocal::Type QwtSplineLocal::type() const
{
    return d_type;
}

void QwtSplineLocal::setTension( double tension )
{
    // breaking endpoint conditions ???
    d_tension = qBound( 0.0, tension, 1.0 );
}

double QwtSplineLocal::tension() const
{
    return d_tension;
}

/*!
  \brief Interpolate a curve with Bezier curves

  Interpolates a polygon piecewise with cubic Bezier curves
  and returns them as QPainterPath.

  \param points Control points
  \return Painter path, that can be rendered by QPainter
 */
QPainterPath QwtSplineLocal::painterPath( const QPolygonF &points ) const
{
    if ( parametrization()->type() == QwtSplineParametrization::ParameterX )
    {
        using namespace QwtSplineLocalP;
        return qwtSplineLocal<PathStore>( this, points).path;
    }

    return QwtSplineC1::painterPath( points );
}

/*! 
  \brief Interpolate a curve with Bezier curves
    
  Interpolates a polygon piecewise with cubic Bezier curves
  and returns the 2 control points of each curve as QLineF.

  \param points Control points
  \return Control points of the interpolating Bezier curves
 */
QVector<QLineF> QwtSplineLocal::bezierControlLines( const QPolygonF &points ) const
{
    if ( parametrization()->type() == QwtSplineParametrization::ParameterX )
    {   
        using namespace QwtSplineLocalP;
        return qwtSplineLocal<ControlPointsStore>( this, points ).controlPoints;
    }

    return QwtSplineC1::bezierControlLines( points );
}

QVector<double> QwtSplineLocal::slopes( const QPolygonF &points ) const
{
    using namespace QwtSplineLocalP;
    return qwtSplineLocal<SlopeStore>( this, points ).slopes;
}

QVector<QwtSplinePolynomial> QwtSplineLocal::polynomials( const QPolygonF &points ) const
{
    return QwtSplineC1::polynomials( points );
}

uint QwtSplineLocal::locality() const
{
    // should be a QPair<uint, uint>
    switch ( d_type )
    {
        case Cardinal:
        {
            // polynoms: 2 left, 2 right
            return 2;
        }
        case Akima:
        {
            // polynoms: 3 left, 3 right
            return 3;
        }
        case ParabolicBlending:
        case PChip:
        {
            // polynoms: 1 left, 1 right
            return 1;
        }
    }

    return QwtSplineC1::locality(); 
}
