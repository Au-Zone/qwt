/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qpainter.h>
#include "qwt_global.h"
#include "qwt_scale_map.h"
#include "qwt_double_rect.h"
#include "qwt_math.h"
#include "qwt_polygon.h"
#include "qwt_symbol.h"
#include "qwt_radial_plot_curve.h"
#include <QDebug>

static int verifyRange(int size, int &i1, int &i2)
{
    if (size < 1)
        return 0;

    i1 = qwtLim(i1, 0, size-1);
    i2 = qwtLim(i2, 0, size-1);

    if ( i1 > i2 )
        qSwap(i1, i2);

    return (i2 - i1 + 1);
}

class QwtRadialPlotCurve::PrivateData
{
public:
    PrivateData():
        style(QwtRadialPlotCurve::Lines)
    {
        symbol = new QwtSymbol();
        pen = QPen(Qt::black);
    }

    ~PrivateData()
    {
        delete symbol;
    }

    QwtRadialPlotCurve::CurveStyle style;
    QwtSymbol *symbol;
    QPen pen;
};

QwtRadialPlotCurve::QwtRadialPlotCurve():
    QwtRadialPlotItem(QwtText())
{
    init();
}

QwtRadialPlotCurve::QwtRadialPlotCurve(const QwtText &title):
    QwtRadialPlotItem(title)
{
    init();
}

QwtRadialPlotCurve::QwtRadialPlotCurve(const QString &title):
    QwtRadialPlotItem(QwtText(title))
{
    init();
}

QwtRadialPlotCurve::~QwtRadialPlotCurve()
{
    delete d_points;
    delete d_data;
}

void QwtRadialPlotCurve::init()
{
    setItemAttribute(QwtRadialPlotItem::AutoScale);

    d_data = new PrivateData;
    d_points = new QwtPolygonFData(QwtArray<QwtDoublePoint>());

    setZ(20.0);
#if QT_VERSION >= 0x040000
    setRenderHint(RenderAntialiased, true);
#endif
}

//! \return QwtRadialPlotCurve::Rtti_RadialPlotCurve
int QwtRadialPlotCurve::rtti() const
{
    return QwtRadialPlotItem::Rtti_RadialPlotCurve;
}

void QwtRadialPlotCurve::setStyle(CurveStyle style)
{
    if ( style != d_data->style )
    {
        d_data->style = style;
        itemChanged();
    }
}

QwtRadialPlotCurve::CurveStyle QwtRadialPlotCurve::style() const 
{ 
    return d_data->style; 
}

void QwtRadialPlotCurve::setSymbol(const QwtSymbol &s )
{
    delete d_data->symbol;
    d_data->symbol = s.clone();
    itemChanged();
}

const QwtSymbol &QwtRadialPlotCurve::symbol() const 
{ 
    return *d_data->symbol; 
}

void QwtRadialPlotCurve::setPen(const QPen &p)
{
    if ( p != d_data->pen )
    {
        d_data->pen = p;
        itemChanged();
    }
}

const QPen& QwtRadialPlotCurve::pen() const 
{ 
    return d_data->pen; 
}

void QwtRadialPlotCurve::setData(const QwtData &data)
{
    delete d_points;
    d_points = data.copy();
    itemChanged();
}

void QwtRadialPlotCurve::draw(QPainter *painter,
    const QwtScaleMap &distanceMap, const QwtScaleMap &angleMap,
    const QRect &canvasRect) const
{
    draw(painter, canvasRect.center(), distanceMap, angleMap, 0, -1);
}

void QwtRadialPlotCurve::draw(QPainter *painter,
    const QPoint &center,
    const QwtScaleMap &distanceMap, const QwtScaleMap &angleMap, 
    int from, int to) const
{
    if ( !painter || dataSize() <= 0 )
        return;

    if (to < 0)
        to = dataSize() - 1;

    if ( verifyRange(dataSize(), from, to) > 0 )
    {
        painter->save();
        painter->setPen(d_data->pen);

        drawCurve(painter, d_data->style, 
            center, distanceMap, angleMap, from, to);
        painter->restore();

        if (d_data->symbol->style() != QwtSymbol::NoSymbol)
        {
            painter->save();
            drawSymbols(painter, *d_data->symbol, 
                center, distanceMap, angleMap, from, to);
            painter->restore();
        }
    }
}

void QwtRadialPlotCurve::drawCurve(QPainter *painter, 
    int style, const QPoint &center,
    const QwtScaleMap &distanceMap, const QwtScaleMap &angleMap, 
    int from, int to) const
{
    switch (style)
    {
        case Lines:
            drawLines(painter, center, distanceMap, angleMap, from, to);
            break;
        case NoCurve:
        default:
            break;
    }
}

void QwtRadialPlotCurve::drawLines(QPainter *painter,
    const QPoint &center,
    const QwtScaleMap &distanceMap, const QwtScaleMap &angleMap, 
    int from, int to) const
{
    int size = to - from + 1;
    if ( size <= 0 )
        return;

    QwtPolygon polyline(size);
    for (int i = from; i <= to; i++)
    {
        double radius = distanceMap.xTransform(distance(i));
#if 1
        radius -= center.x();
#endif
        const double a = angleMap.xTransform(angle(i));
        polyline.setPoint(i - from, 
            qwtPolar2Pos(center, radius, a) );
    }
    painter->drawPolyline(polyline);
}

void QwtRadialPlotCurve::drawSymbols(QPainter *painter, 
    const QwtSymbol &symbol, const QPoint &center,
    const QwtScaleMap &distanceMap, const QwtScaleMap &angleMap, 
    int from, int to) const
{
    painter->setBrush(symbol.brush());
    painter->setPen(symbol.pen());

    QRect rect(QPoint(0, 0), symbol.size());

    for (int i = from; i <= to; i++)
    {
        double radius = distanceMap.xTransform(distance(i));
#if 1
        radius -= center.x();
#endif
        const double a = angleMap.xTransform(angle(i));

        const QPoint pos = qwtPolar2Pos(center, radius, a);

        rect.moveCenter(pos);
        symbol.draw(painter, rect);
    }

}

int QwtRadialPlotCurve::dataSize() const
{
    return d_points->size();
}
