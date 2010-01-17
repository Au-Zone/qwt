/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_curve_3d.h"
#include "qwt_symbol.h"
#include "qwt_color_map.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include <qpainter.h>

typedef QVector<QRgb> QwtColorTable;

class QwtPlotCurve3D::PrivateData
{
public:
    PrivateData():
        style(QwtPlotCurve3D::Dots),
        colorRange(0.0, 1000.0),
        paintAttributes(QwtPlotCurve3D::ClipPoints)
    {
        symbol = new QwtSymbol();
        symbol->setStyle(QwtSymbol::XCross);
        symbol->setSize(5);

        colorMap = new QwtLinearColorMap();
    }

    ~PrivateData()
    {
        delete symbol;
        delete colorMap;
    }

    QwtPlotCurve3D::CurveStyle style;
    QwtSymbol *symbol;
    QwtColorMap *colorMap;
    QwtDoubleInterval colorRange;
    QwtColorTable colorTable;
    int paintAttributes;
};

/*!
  Constructor
  \param title Title of the curve   
*/
QwtPlotCurve3D::QwtPlotCurve3D(const QwtText &title):
    QwtPlotSeriesItem<QwtDoublePoint3D>(title)
{
    init();
}

/*!
  Constructor
  \param title Title of the curve   
*/
QwtPlotCurve3D::QwtPlotCurve3D(const QString &title):
    QwtPlotSeriesItem<QwtDoublePoint3D>(QwtText(title))
{
    init();
}

//! Destructor
QwtPlotCurve3D::~QwtPlotCurve3D()
{
    delete d_data;
}

/*!
  \brief Initialize data members
*/
void QwtPlotCurve3D::init()
{
    setItemAttribute(QwtPlotItem::Legend);
    setItemAttribute(QwtPlotItem::AutoScale);

    d_data = new PrivateData;
    d_series = new QwtPoint3DSeriesData();

    setZ(20.0);
}

//! \return QwtPlotItem::Rtti_PlotCurve
int QwtPlotCurve3D::rtti() const
{
    return QwtPlotItem::Rtti_PlotCurve3D;
}

/*!
  Specify an attribute how to draw the curve

  \param attribute Paint attribute
  \param on On/Off
  /sa PaintAttribute, testPaintAttribute()
*/
void QwtPlotCurve3D::setPaintAttribute(PaintAttribute attribute, bool on)
{
    if ( on )
        d_data->paintAttributes |= attribute;
    else
        d_data->paintAttributes &= ~attribute;
}

/*!
    \brief Return the current paint attributes
    \sa PaintAttribute, setPaintAttribute()
*/
bool QwtPlotCurve3D::testPaintAttribute(PaintAttribute attribute) const
{
    return (d_data->paintAttributes & attribute);
}

void QwtPlotCurve3D::setSamples(const QVector<QwtDoublePoint3D> &data)
{
    delete d_series;
    d_series = new QwtPoint3DSeriesData(data);
    itemChanged();
}

/*!
  Set the curve's drawing style

  \param style Curve style
  \sa CurveStyle, style()
*/
void QwtPlotCurve3D::setStyle(CurveStyle style)
{
    if ( style != d_data->style )
    {
        d_data->style = style;
        itemChanged();
    }
}

/*!
    Return the current style
    \sa CurveStyle, setStyle()
*/
QwtPlotCurve3D::CurveStyle QwtPlotCurve3D::style() const
{
    return d_data->style;
}

/*!
  Change the color map

  Often it is useful to display the mapping between intensities and
  colors as an additional plot axis, showing a color bar.

  \param colorMap Color Map

  \sa colorMap(), QwtScaleWidget::setColorBarEnabled(),
      QwtScaleWidget::setColorMap()
*/
void QwtPlotCurve3D::setColorMap(const QwtColorMap &colorMap)
{
    delete d_data->colorMap;
    d_data->colorMap = colorMap.copy();

    itemChanged();
}

/*!
   \return Color Map used for mapping the intensity values to colors
   \sa setColorMap()
*/
const QwtColorMap &QwtPlotCurve3D::colorMap() const
{
    return *d_data->colorMap;
}

void QwtPlotCurve3D::setColorRange(const QwtDoubleInterval &interval)
{
    if ( interval != d_data->colorRange )
    {
        d_data->colorRange = interval;
        itemChanged();
    }
}

QwtDoubleInterval &QwtPlotCurve3D::colorRange() const
{
    return d_data->colorRange;
}

void QwtPlotCurve3D::drawSeries(QPainter *painter, 
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, int from, int to) const
{
    if ( !painter || dataSize() <= 0 )
        return;

    if (to < 0)
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from >= to )
        return;

    switch(d_data->style)
    {
        case QwtPlotCurve3D::Symbols:
            drawSymbols(painter, xMap, yMap, canvasRect, from, to);
            break;

        case QwtPlotCurve3D::Dots:
        default:
            drawDots(painter, xMap, yMap, canvasRect, from, to);
            break;
    }
}

void QwtPlotCurve3D::drawDots(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, int from, int to) const
{
    if ( !d_data->colorRange.isValid() )
        return;

    const QwtColorMap::Format format = d_data->colorMap->format();
    if ( format == QwtColorMap::Indexed )
        d_data->colorTable = d_data->colorMap->colorTable(d_data->colorRange);

    for (int i = from; i <= to; i++)
    {   
        const QwtDoublePoint3D sample = d_series->sample(i);
    
        const double xi = xMap.xTransform(sample.x());
        const double yi = yMap.xTransform(sample.y());

        if ( d_data->paintAttributes & QwtPlotCurve3D::ClipPoints )
        {
            if ( !canvasRect.contains(xi, yi ) )
                continue;
        } 

        if ( format == QwtColorMap::RGB )
        {
            const QRgb rgb = d_data->colorMap->rgb(
                d_data->colorRange, sample.z());

            painter->setPen(QPen(QColor(rgb)));
        }
        else
        {
            const unsigned char index = d_data->colorMap->colorIndex(
                d_data->colorRange, sample.z());

            painter->setPen(QPen(d_data->colorTable[index]));
        }

        QwtPainter::drawPoint(painter, QPointF(xi, yi));
    }
    
    d_data->colorTable.clear();
}

void QwtPlotCurve3D::drawSymbols(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, int from, int to) const
{
    if ( !d_data->colorRange.isValid() )
        return;

    if ( d_data->colorMap->format() == QwtColorMap::Indexed )
        d_data->colorTable = d_data->colorMap->colorTable(d_data->colorRange);

    for (int i = from; i <= to; i++)
    {
        const QwtDoublePoint3D sample = d_series->sample(i);

        const double xi = xMap.xTransform(sample.x());
        const double yi = yMap.xTransform(sample.y());

        if ( d_data->paintAttributes & QwtPlotCurve3D::ClipPoints )
        {
            if ( !canvasRect.contains(xi, yi ) )
                continue;
        }

        const QwtSymbol *symbol = valueSymbol(sample);
        if ( symbol )
        {
            symbol->draw(painter, QPointF(xi, yi));
            delete symbol;
        }
    }

    d_data->colorTable.clear();
}

QwtSymbol *QwtPlotCurve3D::valueSymbol(const QwtDoublePoint3D &sample) const
{
    QwtSymbol *symbol = new QwtSymbol();
    symbol->setSize(5, 5);
    symbol->setStyle(QwtSymbol::XCross);

    QRgb rgb;
    if ( d_data->colorMap->format() == QwtColorMap::RGB )
    {
        rgb = d_data->colorMap->rgb(d_data->colorRange, sample.z());
    }
    else
    {
        const unsigned char index = 
            d_data->colorMap->colorIndex(d_data->colorRange, sample.z());
        rgb = d_data->colorTable[index];
    }

    symbol->setPen(QColor(rgb));
    symbol->setBrush(QColor(rgb));
        
    return symbol;
}
