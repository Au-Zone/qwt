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
        slopeEnd = spline->slopeEnd( points, s0, m2 );
        slopeBegin = spline->slopeBegin( points, m2, slopeEnd );
    }
    else
    {
        const double m3 = qwtSlopeP( p[1], p[3] );
        slopeBegin = spline->slopeBegin( points, m2, m3 );

        const double mn1 = qwtSlopeP( p[n-4], p[n-2] );
        const double mn2 = qwtSlopeP( p[n-3], p[n-1] );

        slopeEnd = spline->slopeEnd( points, mn1, mn2 );
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

    const double s = 1.0 - spline->tension();

    SplineStore store;
    store.init( points );
    store.start( p[0], s * slopeBegin );

    double m1 = s * slopeBegin;
    for ( int i = 1; i < size - 1; i++ )
    {
        const double m2 = s * qwtSlopeP( p[i-1], p[i+1] );
        store.addCubic( p[i-1], m1, p[i], m2 );

        m1 = m2;
    }

    store.addCubic( p[size-2], s * m1, p[size-1], s * slopeEnd );

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

        const double s1 = qwtSlopeP( p1, p2 );
        const double s2 = qwtSlopeP( p2, p[0] );
        const double s3 = qwtSlopeP( p[0], p[1] );
        const double s4 = qwtSlopeP( p[1], p[2] );

        slopeBegin = slopeEnd = qwtAkima( s1, s2, s3, s4 );

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
        const double m = qwtAkima( s1, s1, s2, s2 );

        slopeBegin = spline->slopeBegin( points, m, s2 );
        slopeEnd = spline->slopeEnd( points, slopeBegin, m );
    }
    else if ( n == 4 )
    {
        const double s1 = qwtSlopeP( p[0], p[1] );
        const double s2 = qwtSlopeP( p[1], p[2] );
        const double s3 = qwtSlopeP( p[2], p[3] );

        const double m2 = qwtAkima( s1, s1, s2, s2 );
        const double m3 = qwtAkima( s2, s2, s3, s3 );

        slopeBegin = spline->slopeBegin( points, m2, m3 );
        slopeEnd = spline->slopeEnd( points, m2, m3 );
    }
    else
    {
        double s[4];

        for ( int i = 0; i < 4; i++ )
        {
            s[i] = qwtSlopeP( p[i], p[i+1] );
        }

        const double m2 = qwtAkima( s[0], s[0], s[1], s[1] );
        const double m3 = qwtAkima( s[0], s[1], s[2], s[3] );

        slopeBegin = spline->slopeBegin( points, m2, m3 );

        for ( int i = 0; i < 4; i++ )
        {
            const int j = n - 5 + i;
            s[i] = qwtSlopeP( p[j], p[j+1] );
        }

        const double mn1 = qwtAkima( s[0], s[1], s[2], s[3] );
        const double mn2 = qwtAkima( s[2], s[2], s[3], s[3] );

        slopeEnd = spline->slopeEnd( points, mn1, mn2 );
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

    const double s = 1.0 - spline->tension();

    SplineStore store;
    store.init( points );
    store.start( p[0], s * slopeBegin );

    double s1 = slopeBegin;
    double s2 = qwtSlopeP( p[0], p[1] );
    double s3 = qwtSlopeP( p[1], p[2] );

    double m1 = s * slopeBegin;

    for ( int i = 0; i < size - 3; i++ )
    {
        const double s4 = qwtSlopeP( p[i+2],  p[i+3] );

        const double m2 = s * qwtAkima( s1, s2, s3, s4 );
        store.addCubic( p[i], m1, p[i+1], m2 );

        s1 = s2;
        s2 = s3;
        s3 = s4;

        m1 = m2;
    }

    const double m2 = s * qwtAkima( s1, s2, s3, slopeEnd );

    store.addCubic( p[size - 3], m1, p[size - 2], m2 );
    store.addCubic( p[size - 2], m2, p[size - 1], slopeEnd );

    return store;
}

static inline void qwtSplineHarmonicMeanBoundaries(
    const QwtSplineLocal *spline, const QPolygonF &points,
    double &slopeBegin, double &slopeEnd )
{
    const int n = points.size();
    const QPointF *p = points.constData();

    if ( spline->isClosing()
        || ( spline->boundaryCondition() == QwtSplineC1::Periodic ) )
    {
        const QPointF pn = p[0] - ( p[n-1] - p[n-2] );
        const QPointF dp1 = p[0] - pn;
        const QPointF dp2 = p[1] - p[0];

        slopeBegin = slopeEnd =
            qwtHarmonicMean( dp1.x(), dp1.y(), dp2.x(), dp2.y() );

        return;
    }

    const double s1 = qwtSlopeP( p[0], p[1] );
    const double s2 = qwtSlopeP( p[1], p[2] );

    if ( n == 3 )
    {
        const double m = qwtHarmonicMean( s1, s2 );
        slopeBegin = spline->slopeBegin( points, m, s2 );
        slopeEnd = spline->slopeEnd( points, slopeBegin, m );
    }
    else
    {
        const double s3 = qwtSlopeP( p[2], p[3] );

        const double m2 = qwtHarmonicMean( s1, s2 );
        const double m3 = qwtHarmonicMean( s2, s3 );

        const double sn1 = qwtSlopeP( p[n-4], p[n-3] );
        const double sn2 = qwtSlopeP( p[n-3], p[n-2] );
        const double sn3 = qwtSlopeP( p[n-2], p[n-1] );

        const double mn1 = qwtHarmonicMean( sn1, sn2 );
        const double mn2 = qwtHarmonicMean( sn2, sn3 );

        slopeBegin = spline->slopeBegin( points, m2, m3 );
        slopeEnd = spline->slopeEnd( points, mn1, mn2 );
    }
}

template< class SplineStore >
static inline SplineStore qwtSplineHarmonicMean( const QwtSplineLocal *spline,
    const QPolygonF &points )
{
    const int size = points.size();
    const QPointF *p = points.constData();

    double slopeBegin, slopeEnd;
    qwtSplineHarmonicMeanBoundaries( spline, points, slopeBegin, slopeEnd );

    const double s = 1.0 - spline->tension();

    SplineStore store;
    store.init( points );
    store.start( p[0], s * slopeBegin );

    double dx1 = p[1].x() - p[0].x();
    double dy1 = p[1].y() - p[0].y();

    double m1 = s * slopeBegin;
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

    store.addCubic( p[size - 2], m1, p[size - 1], s * slopeEnd );

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
        const double s0 = qwtSlopeP( points[0], points[1] );
        const double m1 = spline->slopeBegin( points, s0, s0 ) * spline->tension();
        const double m2 = spline->slopeEnd( points, s0, s0 ) * spline->tension();

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
            // Bessel: not implemented
            break;
        }
        case QwtSplineLocal::Akima:
        {   
            store = qwtSplineAkima<SplineStore>( spline, points );
            break;
        }
        case QwtSplineLocal::HarmonicMean:
        {   
            store = qwtSplineHarmonicMean<SplineStore>( spline, points );
            break;
        }
        case QwtSplineLocal::PChip:
        {   
            // not implemented
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
        case HarmonicMean:
        {
            // polynoms: 1 left, 1 right
            return 1;
        }
        case ParabolicBlending:
        case PChip:
        {
            // not implemented
            return 0;
        }
    }

    return QwtSplineC1::locality(); 
}
