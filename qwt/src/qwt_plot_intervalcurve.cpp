/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qpainter.h>
#include "qwt_polygon.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include "qwt_plot_intervalcurve.h"

class QwtPlotIntervalCurve::PrivateData
{
public:
    PrivateData():
        curveStyle(Tube),
        symbolStyle(NoSymbol),
        pen(Qt::black, 0)
    {
    }

    CurveStyle curveStyle;
    SymbolStyle symbolStyle;

    QPen pen;
    QBrush brush;
};

/*!
  \brief Ctor
  \param title title of the curve   
*/
QwtPlotIntervalCurve::QwtPlotIntervalCurve(const QwtText &title):
    QwtPlotSeriesItem<QwtIntervalSample>(title)
{
    init();
}

/*!
  \brief Ctor
  \param title title of the curve   
*/
QwtPlotIntervalCurve::QwtPlotIntervalCurve(const QString &title):
    QwtPlotSeriesItem<QwtIntervalSample>(QwtText(title))
{
    init();
}

//! Dtor
QwtPlotIntervalCurve::~QwtPlotIntervalCurve()
{
    delete d_data;
}

/*!
  \brief Initialize data members
*/
void QwtPlotIntervalCurve::init()
{
    setItemAttribute(QwtPlotItem::Legend);
    setItemAttribute(QwtPlotItem::AutoScale);

    d_data = new PrivateData;
    d_series = new QwtIntervalSeriesData();

    setZ(19.0);
}

int QwtPlotIntervalCurve::rtti() const
{
    return QwtPlotIntervalCurve::Rtti_PlotIntervalCurve;
}

void QwtPlotIntervalCurve::setData(
    const QwtArray<QwtIntervalSample> &data)
{
    QwtPlotSeriesItem<QwtIntervalSample>::setData(
        QwtIntervalSeriesData(data));
}

void QwtPlotIntervalCurve::setData(
    const QwtSeriesData<QwtIntervalSample> &data)
{
    QwtPlotSeriesItem<QwtIntervalSample>::setData(data);
}

void QwtPlotIntervalCurve::setCurveStyle(CurveStyle style)
{
    if ( style != d_data->curveStyle )
    {
        d_data->curveStyle = style;
        itemChanged();
    }
}

/*!
    \brief Return the current style
    \sa setStyle
*/
QwtPlotIntervalCurve::CurveStyle QwtPlotIntervalCurve::curveStyle() const 
{ 
    return d_data->curveStyle; 
}

void QwtPlotIntervalCurve::setSymbolStyle(SymbolStyle style)
{
    if ( style != d_data->symbolStyle )
    {
        d_data->symbolStyle = style;
        itemChanged();
    }
}

const QwtPlotIntervalCurve::SymbolStyle &
QwtPlotIntervalCurve::symbolStyle() const 
{ 
    return d_data->symbolStyle; 
}

/*!
  \brief Assign a pen
  \param p New pen
  \sa pen(), brush()
*/
void QwtPlotIntervalCurve::setPen(const QPen &p)
{
    if ( p != d_data->pen )
    {
        d_data->pen = p;
        itemChanged();
    }
}

/*!
    \brief Return the pen used to draw the lines
    \sa setPen(), brush()
*/
const QPen& QwtPlotIntervalCurve::pen() const 
{ 
    return d_data->pen; 
}

void QwtPlotIntervalCurve::setBrush(const QBrush &brush)
{
    if ( brush != d_data->brush )
    {
        d_data->brush = brush;
        itemChanged();
    }
}

const QBrush& QwtPlotIntervalCurve::brush() const 
{
    return d_data->brush;
}

/*!
  \brief Draw the complete curve

  \param painter Painter
  \param xMap Maps x-values into pixel coordinates.
  \param yMap Maps y-values into pixel coordinates.

  \sa drawCurve(), drawSymbols()
*/
void QwtPlotIntervalCurve::draw(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRect &) const
{
    draw(painter, xMap, yMap, 0, -1);
}

void QwtPlotIntervalCurve::draw(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    int from, int to) const
{
    if (to < 0)
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from >= to )
        return;
        
    switch(d_data->curveStyle)
    {
        case Tube:
            drawTube(painter, xMap, yMap, from, to);
            break;
        case NoCurve:
        default:
            break;
    }

    if ( d_data->symbolStyle != NoSymbol )
    {
        drawSymbols(painter, xMap, yMap, from, to);
    }
}

void QwtPlotIntervalCurve::drawTube(QPainter *painter, 
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    int from, int to) const
{
    painter->save();

    const size_t size = dataSize();
    QwtPolygon minValues(size);
    QwtPolygon maxValues(size);

    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample intervalSample = sample(i);
        QPoint &minValue = minValues[i];
        QPoint &maxValue = maxValues[to - i];

        if ( orientation() == Qt::Vertical )
        {
            const int x = xMap.transform(intervalSample.value);
            const int y1 = yMap.transform(intervalSample.interval.minValue());
            const int y2 = yMap.transform(intervalSample.interval.maxValue());

            minValue.setX(x);
            minValue.setY(y1);
            maxValue.setX(x);
            maxValue.setY(y2);
        }
        else
        {
            const int y = yMap.transform(intervalSample.value);
            const int x1 = xMap.transform(intervalSample.interval.minValue());
            const int x2 = xMap.transform(intervalSample.interval.maxValue());

            minValue.setX(x1);
            minValue.setY(y);
            maxValue.setX(x2);
            maxValue.setY(y);
        }
    }

    painter->setPen(d_data->pen);
    painter->setBrush(Qt::NoBrush);

    QwtPainter::drawPolyline(painter, minValues);
    QwtPainter::drawPolyline(painter, maxValues);

#if QT_VERSION < 0x040000
    minValues.resize(minValues.size() + maxValues.size());
    minValues.putPoints(minValues.size(), maxValues.size(), maxValues);
#else
    minValues += maxValues;
    maxValues.clear();
#endif

    painter->setPen(Qt::NoPen);
    painter->setBrush(d_data->brush);

    QwtPainter::drawPolygon(painter, minValues);

    painter->restore();
}

void QwtPlotIntervalCurve::drawSymbols(
    QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    int from, int to) const
{
    painter->save();

    painter->setPen(d_data->pen);

    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample intervalSample = sample(i);

        QPoint p[2];

        if ( orientation() == Qt::Vertical )
        {
            const int x = xMap.transform(intervalSample.value);
            const int y1 = yMap.transform(intervalSample.interval.minValue());
            const int y2 = yMap.transform(intervalSample.interval.maxValue());

            p[0].setX(x);
            p[0].setY(y1);
            p[1].setX(x);
            p[1].setY(y2);
        }
        else
        {
            const int y = yMap.transform(intervalSample.value);
            const int x1 = xMap.transform(intervalSample.interval.minValue());
            const int x2 = xMap.transform(intervalSample.interval.maxValue());

            p[0].setX(x1);
            p[0].setY(y);
            p[1].setX(x2);
            p[1].setY(y);
        }

        switch(d_data->symbolStyle)
        {
            case Bar:
                drawBar(painter, i, p[0], p[1]);
                break;
            default:;
        }
    }

    painter->restore();
}

void QwtPlotIntervalCurve::drawBar(QPainter *painter, int,
    const QPoint& from, const QPoint& to ) const
{
    const int capWidth = 3;

    QwtPainter::drawLine(painter, from, to);
    if ( orientation() == Qt::Vertical )
    {
        const int x1 = from.x() - capWidth;
        const int x2 = from.x() + capWidth;

        QwtPainter::drawLine(painter, x1, from.y(), x2, from.y());
        QwtPainter::drawLine(painter, x1, to.y(), x2, to.y());
    }
    else
    {
        const int y1 = from.y() - capWidth;
        const int y2 = from.y() + capWidth;

        QwtPainter::drawLine(painter, from.x(), y1, from.x(), y1);
        QwtPainter::drawLine(painter, from.x(), y2, from.x(), y2);
    }
}

void QwtPlotIntervalCurve::updateLegend(QwtLegend *) const
{
}
