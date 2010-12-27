/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_clipper.h"
#include "qwt_math.h"
#include <qrect.h>

#if QT_VERSION < 0x040601
#define qAtan(x) ::atan(x)
#endif

enum Edge
{
    Left,
    Top,
    Right,
    Bottom,
    NEdges
};

template< class Point, class Rect, class Polygon >
class QwtPolygonClipper: public Rect
{
public:
    QwtPolygonClipper( const Rect &r );
    Polygon clipPolygon( const Polygon & ) const;

private:
    void clipEdge( Edge, const Polygon &, Polygon & ) const;
    bool insideEdge( const Point &, Edge edge ) const;
    Point intersectEdge( const Point &p1,
        const Point &p2, Edge edge ) const;

    void addPoint( Polygon &, uint pos, const Point &point ) const;

    const Rect d_rect;
};

class QwtCircleClipper: public QRectF
{
public:
    QwtCircleClipper( const QRectF &r );
    QVector<QwtInterval> clipCircle( const QPointF &, double radius ) const;

private:
    QList<QPointF> cuttingPoints(
        Edge, const QPointF &pos, double radius ) const;
    double toAngle( const QPointF &, const QPointF & ) const;
};

template< class Point, class Rect, class Polygon >
QwtPolygonClipper<Point, Rect, Polygon>::QwtPolygonClipper( const Rect &r ):
    d_rect( r )
{
}

template< class Point, class Rect, class Polygon >
inline void QwtPolygonClipper<Point, Rect, Polygon>::addPoint( 
    Polygon &pa, uint pos, const Point &point ) const
{
    if ( uint( pa.size() ) <= pos )
        pa.resize( pos + 5 );

    pa.data()[pos] = point;
}

template< class Point, class Rect, class Polygon >
Polygon QwtPolygonClipper<Point, Rect, Polygon>::clipPolygon( const Polygon &pa ) const
{
    if ( contains( pa.boundingRect() ) )
        return pa;

    Polygon cpa( pa.size() );

    clipEdge( ( Edge )0, pa, cpa );

    for ( uint edge = 1; edge < NEdges; edge++ )
    {
        const Polygon rpa = cpa;
        clipEdge( ( Edge )edge, rpa, cpa );
    }

    return cpa;
}

template< class Point, class Rect, class Polygon >
bool QwtPolygonClipper<Point, Rect, Polygon>::insideEdge( 
    const Point &p, Edge edge ) const
{
    switch ( edge )
    {
        case Left:
            return p.x() > d_rect.left();
        case Top:
            return p.y() > d_rect.top();
        case Right:
            return p.x() < d_rect.right();
        case Bottom:
            return p.y() < d_rect.bottom();
        default:
            break;
    }

    return false;
}

template< class Point, class Rect, class Polygon >
Point QwtPolygonClipper<Point, Rect, Polygon>::intersectEdge( 
    const Point &p1, const Point &p2, Edge edge ) const
{
    double x = 0.0, y = 0.0;
    double m = 0;

    const double dy = p2.y() - p1.y();
    const double dx = p2.x() - p1.x();

    switch ( edge )
    {
        case Left:
        {
            x = d_rect.left();
            m = double( qAbs( p1.x() - x ) ) / qAbs( dx );
            y = p1.y() + int( dy * m );
            break;
        }
        case Top:
        {
            y = d_rect.top();
            m = double( qAbs( p1.y() - y ) ) / qAbs( dy );
            x = p1.x() + int( dx * m );
            break;
        }
        case Right:
        {
            x = d_rect.right();
            m = double( qAbs( p1.x() - x ) ) / qAbs( dx );
            y = p1.y() + int( dy * m );
            break;
        }
        case Bottom:
        {
            y = d_rect.bottom();
            m = double( qAbs( p1.y() - y ) ) / qAbs( dy );
            x = p1.x() + int( dx * m );
            break;
        }
        default:
            break;
    }

    return Point( x, y );
}

template< class Point, class Rect, class Polygon >
void QwtPolygonClipper<Point, Rect, Polygon>::clipEdge( Edge edge,
    const Polygon &pa, Polygon &cpa ) const
{
    if ( pa.count() == 0 )
    {
        cpa.resize( 0 );
        return;
    }

    unsigned int count = 0;

    Point p1 = pa[0];
    if ( insideEdge( p1, edge ) )
        addPoint( cpa, count++, p1 );

    const uint nPoints = pa.size();
    for ( uint i = 1; i < nPoints; i++ )
    {
        const Point p2 = pa[( int )i];
        if ( insideEdge( p2, edge ) )
        {
            if ( insideEdge( p1, edge ) )
                addPoint( cpa, count++, p2 );
            else
            {
                addPoint( cpa, count++, intersectEdge( p1, p2, edge ) );
                addPoint( cpa, count++, p2 );
            }
        }
        else
        {
            if ( insideEdge( p1, edge ) )
                addPoint( cpa, count++, intersectEdge( p1, p2, edge ) );
        }
        p1 = p2;
    }
    cpa.resize( count );
}

QwtCircleClipper::QwtCircleClipper( const QRectF &r ):
    QRectF( r )
{
}

QVector<QwtInterval> QwtCircleClipper::clipCircle(
    const QPointF &pos, double radius ) const
{
    QList<QPointF> points;
    for ( int edge = 0; edge < NEdges; edge++ )
        points += cuttingPoints( ( Edge )edge, pos, radius );

    QVector<QwtInterval> intv;
    if ( points.size() <= 0 )
    {
        QRectF cRect( 0, 0, 2 * radius, 2* radius );
        cRect.moveCenter( pos );
        if ( contains( cRect ) )
            intv += QwtInterval( 0.0, 2 * M_PI );
    }
    else
    {
        QList<double> angles;
        for ( int i = 0; i < points.size(); i++ )
            angles += toAngle( pos, points[i] );
        qSort( angles );

        const int in = contains( qwtPolar2Pos( pos, radius,
            angles[0] + ( angles[1] - angles[0] ) / 2 ) );
        if ( in )
        {
            for ( int i = 0; i < angles.size() - 1; i += 2 )
                intv += QwtInterval( angles[i], angles[i+1] );
        }
        else
        {
            for ( int i = 1; i < angles.size() - 1; i += 2 )
                intv += QwtInterval( angles[i], angles[i+1] );
            intv += QwtInterval( angles.last(), angles.first() );
        }
    }

    return intv;
}

double QwtCircleClipper::toAngle(
    const QPointF &from, const QPointF &to ) const
{
    if ( from.x() == to.x() )
        return from.y() <= to.y() ? M_PI / 2.0 : 3 * M_PI / 2.0;

    const double m = qAbs( ( to.y() - from.y() ) / ( to.x() - from.x() ) );

    double angle = qAtan( m );
    if ( to.x() > from.x() )
    {
        if ( to.y() > from.y() )
            angle = 2 * M_PI - angle;
    }
    else
    {
        if ( to.y() > from.y() )
            angle = M_PI + angle;
        else
            angle = M_PI - angle;
    }

    return angle;
}

QList<QPointF> QwtCircleClipper::cuttingPoints(
    Edge edge, const QPointF &pos, double radius ) const
{
    QList<QPointF> points;

    if ( edge == Left || edge == Right )
    {
        const double x = ( edge == Left ) ? left() : right();
        if ( qAbs( pos.x() - x ) < radius )
        {
            const double off = qSqrt( qwtSqr( radius ) - qwtSqr( pos.x() - x ) );
            const double y1 = pos.y() + off;
            if ( y1 >= top() && y1 <= bottom() )
                points += QPointF( x, y1 );
            const double y2 = pos.y() - off;
            if ( y2 >= top() && y2 <= bottom() )
                points += QPointF( x, y2 );
        }
    }
    else
    {
        const double y = ( edge == Top ) ? top() : bottom();
        if ( qAbs( pos.y() - y ) < radius )
        {
            const double off = qSqrt( qwtSqr( radius ) - qwtSqr( pos.y() - y ) );
            const double x1 = pos.x() + off;
            if ( x1 >= left() && x1 <= right() )
                points += QPointF( x1, y );
            const double x2 = pos.x() - off;
            if ( x2 >= left() && x2 <= right() )
                points += QPointF( x2, y );
        }
    }
    return points;
}

/*!
   Sutherland-Hodgman polygon clipping

   \param clipRect Clip rectangle
   \param polygon Polygon

   \return Clipped polygon
*/
QPolygon QwtClipper::clipPolygon(
    const QRect &clipRect, const QPolygon &polygon )
{
    QwtPolygonClipper<QPoint, QRect, QPolygon> clipper( clipRect );
    return clipper.clipPolygon( polygon );
}

/*!
   Sutherland-Hodgman polygon clipping

   \param clipRect Clip rectangle
   \param polygon Polygon

   \return Clipped polygon
*/
QPolygonF QwtClipper::clipPolygonF(
    const QRectF &clipRect, const QPolygonF &polygon )
{
    QwtPolygonClipper<QPointF, QRectF, QPolygonF> clipper( clipRect );
    return clipper.clipPolygon( polygon );
}

/*!
   Circle clipping

   clipCircle() devides a circle into intervals of angles representing arcs
   of the circle. When the circle is completely inside the clip rectangle
   an interval [0.0, 2 * M_PI] is returned.

   \param clipRect Clip rectangle
   \param center Center of the circle
   \param radius Radius of the circle

   \return Arcs of the circle
*/
QVector<QwtInterval> QwtClipper::clipCircle( const QRectF &clipRect,
    const QPointF &center, double radius )
{
    QwtCircleClipper clipper( clipRect );
    return clipper.clipCircle( center, radius );
}
