/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_pleasing.h"
#include "qwt_spline_parametrization.h"

namespace QwtSplinePleasingP
{
    class Tension
    {
    public:
        Tension():
          t1( 0.0 ),
          t2( 0.0 )
        {
        }

        Tension( double s1, double s2 ):
            t1( s1 ),
            t2( s2 )
        {
        }

        double t1;
        double t2;
    };

    class PathStore
    {
    public:
        inline void init( int )
        {
        }

        inline void start( const QPointF &p0 )
        {
            path.moveTo( p0 );
        }

        inline void addCubic( const QPointF &cp1,
            const QPointF &cp2, const QPointF &p2 )
        {
            path.cubicTo( cp1, cp2, p2 );
        }

        QPainterPath path;
    };

    class ControlPointsStore
    {
    public:
        inline ControlPointsStore():
            d_cp( NULL )
        {
        }

        inline void init( int size )
        {
            controlPoints.resize( size );
            d_cp = controlPoints.data();
        }

        inline void start( const QPointF & )
        {
        }

        inline void addCubic( const QPointF &cp1,
            const QPointF &cp2, const QPointF & )
        {
            QLineF &l = *d_cp++;
            l.setPoints( cp1, cp2 );
        }

        QVector<QLineF> controlPoints;

    private:
        QLineF* d_cp;
    };

    inline double lengthChordal(
        const QPointF &point1, const QPointF &point2 )
    {
        const double dx = point1.x() - point2.x();
        const double dy = point1.y() - point2.y();

        return qSqrt( dx * dx + dy * dy );
    }

    inline double lengthManhattan(
        const QPointF &point1, const QPointF &point2 )
    {
        return qAbs( point2.x() - point1.x() ) + qAbs( point2.y() - point1.y() );
    }

}

static inline QwtSplinePleasingP::Tension qwtTensionPleasing(
    double d13, double d23, double d24,
    const QPointF &p1, const QPointF &p2,
    const QPointF &p3, const QPointF &p4 )
{
    const bool b1 = ( d13 / 3.0 ) < d23;
    const bool b2 = ( d24 / 3.0 ) < d23;

    QwtSplinePleasingP::Tension tension;
#if 0
    if ( b1 && b2 )
    {
        tension.t1 = ( p1 != p2 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
        tension.t2 = ( p3 != p4 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
    }
    else
    {
        tension.t1 = d23 / ( b1 ? d24 : d13 );
        tension.t2 = d23 / ( b2 ? d13 : d24 );
    }
#else
    if ( b1 )
    {
        if ( b2 )
        {
            tension.t1 = ( p1 != p2 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
            tension.t2 = ( p3 != p4 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
        }
        else
        {
            tension.t1 = tension.t2 = d23 / d24;
        }
    }
    else
    {
        if ( b2 )
        {
            tension.t1 = tension.t2 = d23 / d13;
        }
        else
        {
            tension.t1 = d23 / d13;
            tension.t2 = d23 / d24;
        }
    }
#endif

    return tension;
}

template< class SplineStore, double length(const QPointF &, const QPointF &) >
static SplineStore qwtSplinePathPleasing( const QPolygonF &points, bool isClosed )
{
    using namespace QwtSplinePleasingP;

    const int size = points.size();

    const QPointF *p = points.constData();

    SplineStore store;
    store.init( isClosed ? size : size - 1 );
    store.start( p[0] );

    const QPointF &p0 = isClosed ? p[size-1] : p[0];
    double d13 = length(p[0], p[2]);

    const Tension t0 = qwtTensionPleasing( 
        length(p0, p[1]), length(p[0], p[1]), d13, p0, p[0], p[1], p[2] );

    const QPointF vec0 = ( p[1] - p0 ) * 0.5;
    QPointF vec1 = ( p[2] - p[0] ) * 0.5;

    store.addCubic( p[0] + vec0 * t0.t1, p[1] - vec1 * t0.t2, p[1] );

    for ( int i = 1; i < size - 2; i++ )
    {
        const double d23 = length( p[i], p[i+1] );
        const double d24 = length( p[i], p[i+2] );
        const QPointF vec2 = ( p[i+2] - p[i] ) * 0.5;

        const Tension t =
            qwtTensionPleasing( d13, d23, d24, p[i-1], p[i], p[i+1], p[i+2] );

        store.addCubic( p[i] + vec1 * t.t1, p[i+1] - vec2 * t.t2, p[i+1] );

        d13 = d24;
        vec1 = vec2;
    }

    const QPointF &pn = isClosed ? p[0] : p[size-1];
    const double d24 = length( p[size-2], pn );

    const Tension tn = qwtTensionPleasing( 
        d13, length( p[size-2], p[size-1] ), d24, p[size-3], p[size-2], p[size-1], pn );

    const QPointF vec2 = 0.5 * ( pn - p[size-2] );
    store.addCubic( p[size-2] + vec1 * tn.t1, p[size-1] - vec2 * tn.t2, p[size-1] );

    if ( isClosed )
    {
        const double d34 = length( p[size-1], p[0] );
        const double d35 = length( p[size-1], p[1] );

        const QPointF vec3 = 0.5 * ( p[1] - p[size-1] );

        const Tension tn = qwtTensionPleasing( d24, d34, d35, p[size-2], p[size-1], p[0], p[1] );
        store.addCubic( p[size-1] + vec2 * tn.t1, p[0] - vec3 * tn.t2, p[0] );
    }

    return store;
}

QwtSplinePleasing::QwtSplinePleasing()
{
}

QwtSplinePleasing::~QwtSplinePleasing()
{
}

uint QwtSplinePleasing::locality() const
{
    return 2;
}

void QwtSplinePleasing::setLengthType( LengthType lengthType )
{
    d_lengthType = lengthType;
}

QwtSplinePleasing::LengthType QwtSplinePleasing::lengthType() const
{
    return d_lengthType;
}

/*! 
  \brief Interpolate a curve with Bezier curves

  Interpolates a polygon piecewise with cubic Bezier curves
  and returns them as QPainterPath.
    
  \param points Control points
  \return QPainterPath Painter path, that can be rendered by QPainter
 */     
QPainterPath QwtSplinePleasing::painterPath( const QPolygonF &points ) const
{
    const int size = points.size();
    if ( size <= 2 )
        return QwtSplineG1::painterPath( points );

    using namespace QwtSplinePleasingP;

    PathStore store;
    if ( lengthType() == ManhattanLength )
    {
        store = qwtSplinePathPleasing<PathStore, lengthManhattan>( 
            points, isClosing() );
    }
    else
    {
        store = qwtSplinePathPleasing<PathStore, lengthChordal>(
            points, isClosing() );
    }

    if ( isClosing() )
        store.path.closeSubpath();

    return store.path;
}

QVector<QLineF> QwtSplinePleasing::bezierControlLines( 
    const QPolygonF &points ) const
{
    const int size = points.size();
    if ( size <= 2 )
        return QVector<QLineF>();

    using namespace QwtSplinePleasingP;

    ControlPointsStore store;
    if ( lengthType() == ManhattanLength )
    {
        store = qwtSplinePathPleasing<ControlPointsStore, lengthManhattan>(
            points, isClosing() );
    }
    else
    {
        store = qwtSplinePathPleasing<ControlPointsStore, lengthChordal>(
            points, isClosing() );
    }

    return store.controlPoints;
}

