/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qrect.h>
#include "qwt_math.h"
#include "qwt_clipper.h"

class QwtPolygonClipper: public QRect
{
public:
    QwtPolygonClipper(const QRect &r);

    QwtPolygon clipPolygon(const QwtPolygon &) const;

private:
    enum Edge { Left, Top, Right, Bottom, NEdges };

    void clipEdge(Edge, const QwtPolygon &, QwtPolygon &) const;
    bool insideEdge(const QPoint &, Edge edge) const;
    QPoint intersectEdge(const QPoint &p1,
        const QPoint &p2, Edge edge) const;

    void addPoint(QwtPolygon &, uint pos, const QPoint &point) const;
};

class QwtPolygonClipperF: public QwtDoubleRect
{
public:
    QwtPolygonClipperF(const QwtDoubleRect &r);

    QwtPolygonF clipPolygon(const QwtPolygonF &) const;

private:
    enum Edge { Left, Top, Right, Bottom, NEdges };

    void clipEdge(Edge, const QwtPolygonF &, QwtPolygonF &) const;
    bool insideEdge(const QwtDoublePoint &, Edge edge) const;
    QwtDoublePoint intersectEdge(const QwtDoublePoint &p1,
        const QwtDoublePoint &p2, Edge edge) const;

    void addPoint(QwtPolygonF &, uint pos, const QwtDoublePoint &point) const;
};

QwtPolygonClipper::QwtPolygonClipper(const QRect &r): 
    QRect(r) 
{
}

inline void QwtPolygonClipper::addPoint(
    QwtPolygon &pa, uint pos, const QPoint &point) const
{
    if ( uint(pa.size()) <= pos ) 
        pa.resize(pos + 5);

    pa.setPoint(pos, point);
}

//! Sutherland-Hodgman polygon clipping
QwtPolygon QwtPolygonClipper::clipPolygon(const QwtPolygon &pa) const
{
    if ( contains( pa.boundingRect() ) )
        return pa;

    QwtPolygon cpa(pa.size());

    clipEdge((Edge)0, pa, cpa);

    for ( uint edge = 1; edge < NEdges; edge++ ) 
    {
        const QwtPolygon rpa = cpa;
#if QT_VERSION < 0x040000
        cpa.detach();
#endif
        clipEdge((Edge)edge, rpa, cpa);
    }

    return cpa;
}

bool QwtPolygonClipper::insideEdge(const QPoint &p, Edge edge) const
{
    switch(edge) 
    {
        case Left:
            return p.x() > left();
        case Top:
            return p.y() > top();
        case Right:
            return p.x() < right();
        case Bottom:
            return p.y() < bottom();
        default:
            break;
    }

    return false;
}

QPoint QwtPolygonClipper::intersectEdge(const QPoint &p1, 
    const QPoint &p2, Edge edge ) const
{
    int x=0, y=0;
    double m = 0;

    const double dy = p2.y() - p1.y();
    const double dx = p2.x() - p1.x();

    switch ( edge ) 
    {
        case Left:
            x = left();
            m = double(qwtAbs(p1.x() - x)) / qwtAbs(dx);
            y = p1.y() + int(dy * m);
            break;
        case Top:
            y = top();
            m = double(qwtAbs(p1.y() - y)) / qwtAbs(dy);
            x = p1.x() + int(dx * m);
            break;
        case Right:
            x = right();
            m = double(qwtAbs(p1.x() - x)) / qwtAbs(dx);
            y = p1.y() + int(dy * m);
            break;
        case Bottom:
            y = bottom();
            m = double(qwtAbs(p1.y() - y)) / qwtAbs(dy);
            x = p1.x() + int(dx * m);
            break;
        default:
            break;
    }

    return QPoint(x,y);
}

void QwtPolygonClipper::clipEdge(Edge edge, 
    const QwtPolygon &pa, QwtPolygon &cpa) const
{
    if ( pa.count() == 0 )
    {
        cpa.resize(0);
        return;
    }

    unsigned int count = 0;

    QPoint p1 = pa.point(0);
    if ( insideEdge(p1, edge) )
        addPoint(cpa, count++, p1);

    const uint nPoints = pa.size();
    for ( uint i = 1; i < nPoints; i++ )
    {
        const QPoint p2 = pa.point(i);
        if ( insideEdge(p2, edge) )
        {
            if ( insideEdge(p1, edge) )
                addPoint(cpa, count++, p2);
            else
            {
                addPoint(cpa, count++, intersectEdge(p1, p2, edge));
                addPoint(cpa, count++, p2);
            }
        }
        else
        {
            if ( insideEdge(p1, edge) )
                addPoint(cpa, count++, intersectEdge(p1, p2, edge));
        }
        p1 = p2;
    }
    cpa.resize(count);
}

QwtPolygonClipperF::QwtPolygonClipperF(const QwtDoubleRect &r): 
    QwtDoubleRect(r) 
{
}

inline void QwtPolygonClipperF::addPoint(QwtPolygonF &pa, uint pos, const QwtDoublePoint &point) const
{
    if ( uint(pa.size()) <= pos ) 
        pa.resize(pos + 5);

    pa[pos] = point;
}

//! Sutherland-Hodgman polygon clipping
QwtPolygonF QwtPolygonClipperF::clipPolygon(const QwtPolygonF &pa) const
{
    if ( contains( pa.boundingRect() ) )
        return pa;

    QwtPolygonF cpa(pa.size());

    clipEdge((Edge)0, pa, cpa);

    for ( uint edge = 1; edge < NEdges; edge++ ) 
    {
        const QwtPolygonF rpa = cpa;
#if QT_VERSION < 0x040000
        cpa.detach();
#endif
        clipEdge((Edge)edge, rpa, cpa);
    }

    return cpa;
}

bool QwtPolygonClipperF::insideEdge(const QwtDoublePoint &p, Edge edge) const
{
    switch(edge) 
    {
        case Left:
            return p.x() > left();
        case Top:
            return p.y() > top();
        case Right:
            return p.x() < right();
        case Bottom:
            return p.y() < bottom();
        default:
            break;
    }

    return false;
}

QwtDoublePoint QwtPolygonClipperF::intersectEdge(const QwtDoublePoint &p1, 
    const QwtDoublePoint &p2, Edge edge ) const
{
    double x=0.0, y=0.0;
    double m = 0;

    const double dy = p2.y() - p1.y();
    const double dx = p2.x() - p1.x();

    switch ( edge ) 
    {
        case Left:
            x = left();
            m = double(qwtAbs(p1.x() - x)) / qwtAbs(dx);
            y = p1.y() + int(dy * m);
            break;
        case Top:
            y = top();
            m = double(qwtAbs(p1.y() - y)) / qwtAbs(dy);
            x = p1.x() + int(dx * m);
            break;
        case Right:
            x = right();
            m = double(qwtAbs(p1.x() - x)) / qwtAbs(dx);
            y = p1.y() + int(dy * m);
            break;
        case Bottom:
            y = bottom();
            m = double(qwtAbs(p1.y() - y)) / qwtAbs(dy);
            x = p1.x() + int(dx * m);
            break;
        default:
            break;
    }

    return QwtDoublePoint(x,y);
}

void QwtPolygonClipperF::clipEdge(Edge edge, 
    const QwtPolygonF &pa, QwtPolygonF &cpa) const
{
    if ( pa.count() == 0 )
    {
        cpa.resize(0);
        return;
    }

    unsigned int count = 0;

    QwtDoublePoint p1 = pa[0];
    if ( insideEdge(p1, edge) )
        addPoint(cpa, count++, p1);

    const uint nPoints = pa.size();
    for ( uint i = 1; i < nPoints; i++ )
    {
        const QwtDoublePoint p2 = pa[i];
        if ( insideEdge(p2, edge) )
        {
            if ( insideEdge(p1, edge) )
                addPoint(cpa, count++, p2);
            else
            {
                addPoint(cpa, count++, intersectEdge(p1, p2, edge));
                addPoint(cpa, count++, p2);
            }
        }
        else
        {
            if ( insideEdge(p1, edge) )
                addPoint(cpa, count++, intersectEdge(p1, p2, edge));
        }
        p1 = p2;
    }
    cpa.resize(count);
}


QwtPolygon QwtClipper::clipPolygon(
    const QRect &clipRect, const QwtPolygon &polygon)
{
    QwtPolygonClipper clipper(clipRect);
    return clipper.clipPolygon(polygon);
}

QwtPolygonF QwtClipper::clipPolygonF(
    const QwtDoubleRect &clipRect, const QwtPolygonF &polygon)
{
    QwtPolygonClipperF clipper(clipRect);
    return clipper.clipPolygon(polygon);
}
