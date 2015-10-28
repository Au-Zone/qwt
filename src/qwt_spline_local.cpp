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

static inline bool qwtIsStrictlyMonotonic( double dy1, double dy2 )
{
    if ( dy1 == 0.0 || dy2 == 0.0 )
        return false;

    return ( dy1 > 0.0 ) == ( dy2 > 0.0 );
}

static inline double qwtSlopeLine( const QPointF &p1, const QPointF &p2 )
{
    // ???
    const double dx = p2.x() - p1.x();
    return dx ? ( p2.y() - p1.y() ) / dx : 0.0;
}

static inline double qwtSlopeCardinal(
    double dx1, double dy1, double s1, double dx2, double dy2, double s2 )
{
    Q_UNUSED(s1)
    Q_UNUSED(s2)

    return ( dy1 + dy2 ) / ( dx1 + dx2 );
}

static inline double qwtSlopeParabolicBlending(
    double dx1, double dy1, double s1, double dx2, double dy2, double s2 )
{
    Q_UNUSED( dy1 )
    Q_UNUSED( dy2 )

    return ( dx2 * s1 + dx1 * s2 ) / ( dx1 + dx2 );
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
        inline void init( const QVector<QPointF> & )
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
        inline void init( const QVector<QPointF> &points )
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
        void init( const QVector<QPointF> &points )
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

    struct slopeCardinal
    {
        static inline double value( double dx1, double dy1, double s1,
            double dx2, double dy2, double s2 ) 
        {
            return qwtSlopeCardinal( dx1, dy1, s1, dx2, dy2, s2 );
        }
    };

    struct slopeParabolicBlending
    {
        static inline double value( double dx1, double dy1, double s1,
            double dx2, double dy2, double s2 ) 
        {
            return qwtSlopeParabolicBlending( dx1, dy1, s1, dx2, dy2, s2 );
        }
    };  

    struct slopePChip
    {
        static inline double value( double dx1, double dy1, double s1,
            double dx2, double dy2, double s2 ) 
        {
            return qwtSlopePChip( dx1, dy1, s1, dx2, dy2, s2 );
        }
    };  
};

template< class Slope >
static inline double qwtSlopeP3( 
    const QPointF &p1, const QPointF &p2, const QPointF &p3 )
{
    const double dx1 = p2.x() - p1.x();
    const double dy1 = p2.y() - p1.y();
    const double dx2 = p3.x() - p2.x();
    const double dy2 = p3.y() - p2.y();

    return Slope::value( dx1, dy1, dy1 / dx1, dx2, dy2, dy2 / dx2 );
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

static inline double qwtSlopeAkima( const QPointF &p1, const QPointF &p2, 
    const QPointF &p3, const QPointF &p4, const QPointF &p5 )
{
    const double s1 = qwtSlopeLine( p1, p2 );
    const double s2 = qwtSlopeLine( p2, p3 );
    const double s3 = qwtSlopeLine( p3, p4 );
    const double s4 = qwtSlopeLine( p4, p5 );

    return qwtSlopeAkima( s1, s2, s3, s4 );
}

static double qwtSlopeBoundary( 
    QwtSplineC1::BoundaryCondition boundaryCondition, double boundaryValue,
    const QPointF &p1, const QPointF &p2, const QPointF &p3,
    double slope1, double slope2 )
{
    double m = 0.0;

    switch( boundaryCondition )
    {
        case QwtSplineC1::CubicRunout:
        case QwtSplineC1::ParabolicRunout:
        {
            // doesn't make much sense without C2 continuity
            const double dx = p2.x() - p1.x();
            const double dy = p2.y() - p1.y();

            m = 2 * dy / dx - slope1;
            break;
        }
        case QwtSplineC1::Clamped:
        {
            m = boundaryValue;
            break;
        }
        case QwtSplineC1::LinearRunout:
        {
            const double s = qwtSlopeLine( p1, p2 );
            const double r = qBound( 0.0, boundaryValue, 1.0 );

            m = s - r * ( s - slope1 );
            break;
        }
        case QwtSplineC1::Clamped2:
        {
            const double c2 = 0.5 * boundaryValue;
            const double c1 = slope1;

            const double dx = p2.x() - p1.x();
            const double dy = p2.y() - p1.y();

            m = 0.5 * ( 3.0 * dy / dx - c1 - c2 * dx );
            break;
        }
        case QwtSplineC1::NotAKnot:
        {
            // doesn't make much sense without C2 continuity

            const QwtSplinePolynomial pnom = 
                QwtSplinePolynomial::fromSlopes( p2, slope1, p3, slope2 ); 

            boundaryValue = 6.0 * pnom.c3;

            // fallthrough
        }
        case QwtSplineC1::Clamped3:
        {
            const double dx = p2.x() - p1.x();
            const double dy = p2.y() - p1.y();

            const double c3 = boundaryValue / 6.0;
            m = c3 * dx * dx + 2 * dy / dx - slope1;

            break;
        }
        default:
        {
            m = qwtSlopeLine( p1, p2 );
        }
    }

    return m;
}

static double qwtSlopeBegin( const QwtSplineC1 *spline,
    const QVector<QPointF> &points, double slope1, double slope2 )
{
    if ( points.size() < 2 )
        return 0.0;

    return qwtSlopeBoundary( 
        spline->boundaryCondition(), spline->boundaryValueBegin(),
        points[0], points[1], points[2], slope1, slope2 );
}

static double qwtSlopeEnd( const QwtSplineC1 *spline,
    const QVector<QPointF> &points, double slope1, double slope2 ) 
{
    const int n = points.size();

    const QPointF p1( points[n-1].x(), -points[n-1].y() );
    const QPointF p2( points[n-2].x(), -points[n-2].y() );
    const QPointF p3( points[n-3].x(), -points[n-3].y() );

    double boundaryValue = spline->boundaryValueEnd();
    if ( spline->boundaryCondition() != QwtSplineC1::LinearRunout )
    {
        // beside LinearRunout the boundaryValue is a slope or curvature
        // and needs to be inverted too
        boundaryValue = -boundaryValue;
    }

    const double m = qwtSlopeBoundary(
        spline->boundaryCondition(), boundaryValue,
        p1, p2, p3, -slope2, -slope1 );

    return -m;
}

template< class Slope >
static void qwtSplineBoundariesL1( 
    const QwtSplineLocal *spline, const QVector<QPointF> &points, 
    double &slopeBegin, double &slopeEnd )
{
    const int n = points.size();
    const QPointF *p = points.constData();

    if ( spline->isClosing()
        || ( spline->boundaryCondition() == QwtSplineC1::Periodic ) )
    {
        const QPointF pn = p[0] - ( p[n-1] - p[n-2] );
        slopeBegin = slopeEnd = qwtSlopeP3<Slope>( pn, p[0], p[1] );
    }
    else if ( n == 3 )
    {
        const double s0 = qwtSlopeLine( p[0], p[1] );
        const double m = qwtSlopeP3<Slope>( p[0], p[1], p[2] );
    
        slopeEnd = qwtSlopeEnd( spline, points, s0, m );
        slopeBegin = qwtSlopeBegin( spline, points, m, slopeEnd );
    }
    else
    {
        const double m2 = qwtSlopeP3<Slope>( p[0], p[1], p[2] );
        const double m3 = qwtSlopeP3<Slope>( p[1], p[2], p[3] );
        slopeBegin = qwtSlopeBegin( spline, points, m2, m3 );

        const double mn1 = qwtSlopeP3<Slope>( p[n-4], p[n-3], p[n-2] );
        const double mn2 = qwtSlopeP3<Slope>( p[n-3], p[n-2], p[n-1] );
        slopeEnd = qwtSlopeEnd( spline, points, mn1, mn2 );
    }
}

template< class SplineStore, class Slope >
static inline SplineStore qwtSplineL1(
    const QwtSplineLocal *spline, const QVector<QPointF> &points )
{
    const int size = points.size();
    const QPointF *p = points.constData();

    double slopeBegin, slopeEnd; 
    qwtSplineBoundariesL1<Slope>( spline, points, slopeBegin, slopeEnd );

    const double ts = 1.0 - spline->tension();
    double m1 = ts * slopeBegin;

    SplineStore store;
    store.init( points );
    store.start( p[0], m1 );

    double dx1 = p[1].x() - p[0].x();
    double dy1 = p[1].y() - p[0].y();
    double s1 = dy1 / dx1;

    for ( int i = 1; i < size - 1; i++ )
    {
        const double dx2 = p[i+1].x() - p[i].x();
        const double dy2 = p[i+1].y() - p[i].y() ;

        // cardinal spline doesn't need the line slopes, but 
        // the compiler will eliminate pointless calculations
        const double s2 = dy2 / dx2;

        const double m2 = ts * Slope::value( dx1, dy1, s1, dx2, dy2, s2 );

        store.addCubic( p[i-1], m1, p[i], m2 );

        dx1 = dx2;
        dy1 = dy2;
        s1 = s2;
        m1 = m2;
    }

    store.addCubic( p[size-2], m1, p[size-1], ts * slopeEnd );

    return store;
}

static inline void qwtSplineAkimaBoundaries(
    const QwtSplineLocal *spline, const QVector<QPointF> &points,
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
        const double s1 = qwtSlopeLine( p[0], p[1] );
        const double s2 = qwtSlopeLine( p[1], p[2] );
        const double m = qwtSlopeAkima( s1, s1, s2, s2 );

        slopeBegin = qwtSlopeBegin( spline, points, m, s2 );
        slopeEnd = qwtSlopeEnd( spline, points, slopeBegin, m );
    }
    else if ( n == 4 )
    {
        const double s1 = qwtSlopeLine( p[0], p[1] );
        const double s2 = qwtSlopeLine( p[1], p[2] );
        const double s3 = qwtSlopeLine( p[2], p[3] );

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
            s[i] = qwtSlopeLine( p[i], p[i+1] );
        }

        const double m2 = qwtSlopeAkima( s[0], s[0], s[1], s[1] );
        const double m3 = qwtSlopeAkima( s[0], s[1], s[2], s[3] );

        slopeBegin = qwtSlopeBegin( spline, points, m2, m3 );

        for ( int i = 0; i < 4; i++ )
        {
            const int j = n - 5 + i;
            s[i] = qwtSlopeLine( p[j], p[j+1] );
        }

        const double mn1 = qwtSlopeAkima( s[0], s[1], s[2], s[3] );
        const double mn2 = qwtSlopeAkima( s[2], s[2], s[3], s[3] );

        slopeEnd = qwtSlopeEnd( spline, points, mn1, mn2 );
    }
}

template< class SplineStore >
static inline SplineStore qwtSplineAkima(
    const QwtSplineLocal *spline, const QVector<QPointF> &points )
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
    double s2 = qwtSlopeLine( p[0], p[1] );
    double s3 = qwtSlopeLine( p[1], p[2] );

    for ( int i = 0; i < size - 3; i++ )
    {
        const double s4 = qwtSlopeLine( p[i+2],  p[i+3] );

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

template< class SplineStore >
static inline SplineStore qwtSplineLocal( 
    const QwtSplineLocal *spline, const QVector<QPointF> &points )
{   
    SplineStore store;

    const int size = points.size();
    if ( size <= 1 )
        return store;

    if ( size == 2 )
    {
        const double ts = 1.0 - spline->tension();

        const double s0 = qwtSlopeLine( points[0], points[1] );
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
            using namespace QwtSplineLocalP;
            store = qwtSplineL1<SplineStore, slopeCardinal>( spline, points );
            break;
        }
        case QwtSplineLocal::ParabolicBlending:
        {   
            using namespace QwtSplineLocalP;
            store = qwtSplineL1<SplineStore, slopeParabolicBlending>( spline, points );
            break;
        }
        case QwtSplineLocal::PChip:
        {   
            using namespace QwtSplineLocalP;
            store = qwtSplineL1<SplineStore, slopePChip>( spline, points );
            break;
        }
        case QwtSplineLocal::Akima:
        {   
            store = qwtSplineAkima<SplineStore>( spline, points );
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
    // Polynomial store -> TODO
    return QwtSplineC1::polynomials( points );
}

uint QwtSplineLocal::locality() const
{
    switch ( d_type )
    {
        case Akima:
        {
            // polynoms: 2 left, 2 right
            return 2;
        }
        case Cardinal:
        case ParabolicBlending:
        case PChip:
        {
            // polynoms: 1 left, 1 right
            return 1;
        }
    }

    return QwtSplineC1::locality(); 
}
