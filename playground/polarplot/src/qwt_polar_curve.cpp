/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GPL License, Version 2.0
 *****************************************************************************/

#include <qpainter.h>
#include "qwt_global.h"
#include "qwt_scale_map.h"
#include "qwt_double_rect.h"
#include "qwt_math.h"
#include "qwt_polygon.h"
#include "qwt_symbol.h"
#include "qwt_legend.h"
#include "qwt_legend_item.h"
#include "qwt_polar_curve.h"

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

class QwtPolarCurve::PrivateData
{
public:
    PrivateData():
        style(QwtPolarCurve::Lines)
    {
        symbol = new QwtSymbol();
        pen = QPen(Qt::black);
    }

    ~PrivateData()
    {
        delete symbol;
    }

    QwtPolarCurve::CurveStyle style;
    QwtSymbol *symbol;
    QPen pen;
};

//! Constructor
QwtPolarCurve::QwtPolarCurve():
    QwtPolarItem(QwtText())
{
    init();
}

/*!
  Constructor
  \param title title of the curve
*/
QwtPolarCurve::QwtPolarCurve(const QwtText &title):
    QwtPolarItem(title)
{
    init();
}

/*!
  Constructor
  \param title title of the curve
*/
QwtPolarCurve::QwtPolarCurve(const QString &title):
    QwtPolarItem(QwtText(title))
{
    init();
}

//! Destructor
QwtPolarCurve::~QwtPolarCurve()
{
    delete d_points;
    delete d_data;
}

//! Initialize data members
void QwtPolarCurve::init()
{
    setItemAttribute(QwtPolarItem::AutoScale);
    setItemAttribute(QwtPolarItem::Legend);

    d_data = new PrivateData;
    d_points = new QwtPolygonFData(QwtArray<QwtDoublePoint>());

    setZ(20.0);
#if QT_VERSION >= 0x040000
    setRenderHint(RenderAntialiased, true);
#endif
}

//! \return QwtPolarCurve::Rtti_PolarCurve
int QwtPolarCurve::rtti() const
{
    return QwtPolarItem::Rtti_PolarCurve;
}

/*!
  \brief Set the curve's drawing style

  Valid styles are:
  <dl>
  <dt>NoCurve</dt>
  <dd>Don't draw a curve. Note: This doesn't affect the symbol.</dd>
  <dt>Lines</dt>
  <dd>Connect the points with straight lines.</dd>
  <dt>UserCurve ...</dt>
  <dd>Styles >= UserCurve are reserved for derived
      classes of QwtPolarCurve that overload drawCurve() with
      additional application specific curve types.</dd>
  </dl>
  \sa style()
*/
void QwtPolarCurve::setStyle(CurveStyle style)
{
    if ( style != d_data->style )
    {
        d_data->style = style;
        itemChanged();
    }
}

/*!
    \brief Return the current style
    \sa setStyle()
*/  
QwtPolarCurve::CurveStyle QwtPolarCurve::style() const 
{ 
    return d_data->style; 
}

/*!
  \brief Assign a symbol
  \param s symbol
  \sa symbol()
*/
void QwtPolarCurve::setSymbol(const QwtSymbol &s )
{
    delete d_data->symbol;
    d_data->symbol = s.clone();
    itemChanged();
}

/*!
    \brief Return the current symbol
    \sa setSymbol()
*/
const QwtSymbol &QwtPolarCurve::symbol() const 
{ 
    return *d_data->symbol; 
}

/*!
  \brief Assign a pen
  \param pen New pen
  \sa pen()
*/
void QwtPolarCurve::setPen(const QPen &p)
{
    if ( p != d_data->pen )
    {
        d_data->pen = p;
        itemChanged();
    }
}

/*!
    \brief Return the pen used to draw the lines
    \sa setPen()
*/
const QPen& QwtPolarCurve::pen() const 
{ 
    return d_data->pen; 
}

/*!
  Initialize data with a pointer to QwtData.

  The x-values of the data object represent the azimuth,
  the y-value respresent the radius.

  \param data Data
  \sa QwtData::copy()
*/
void QwtPolarCurve::setData(const QwtData &data)
{
    delete d_points;
    d_points = data.copy();
    itemChanged();
}

/*!
  Draw the curve

  \param painter Painter
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param radius Radius of the complete plot area in painter coordinates
  \param canvasRect Contents rect of the canvas in painter coordinates
*/
void QwtPolarCurve::draw(QPainter *painter,
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QwtDoublePoint &pole, double /*radius*/,
    const QwtDoubleRect &) const
{
    draw(painter, azimuthMap, radialMap, pole, 0, -1);
}

/*!
  \brief Draw an interval of the curve
  \param painter Painter
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param from index of the first point to be painted
  \param to index of the last point to be painted. If to < 0 the
         curve will be painted to its last point.

  \sa drawCurve(), drawSymbols(),
*/
void QwtPolarCurve::draw(QPainter *painter,
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QwtDoublePoint &pole, int from, int to) const
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
            azimuthMap, radialMap, pole, from, to);
        painter->restore();

        if (d_data->symbol->style() != QwtSymbol::NoSymbol)
        {
            painter->save();
            drawSymbols(painter, *d_data->symbol, 
                azimuthMap, radialMap, pole, from, to);
            painter->restore();
        }
    }
}

/*!
  Draw the line part (without symbols) of a curve interval.

  \param painter Painter
  \param style curve style, see QwtPolarCurve::CurveStyle
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param from index of the first point to be painted
  \param to index of the last point to be painted. 
  \sa draw(), drawLines()
*/
void QwtPolarCurve::drawCurve(QPainter *painter, int style, 
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QwtDoublePoint &pole, int from, int to) const
{
    switch (style)
    {
        case Lines:
            drawLines(painter, azimuthMap, radialMap, pole, from, to);
            break;
        case NoCurve:
        default:
            break;
    }
}

/*!
  Draw lines

  \param painter Painter
  \param style curve style, see QwtPolarCurve::CurveStyle
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param from index of the first point to be painted
  \param to index of the last point to be painted.
  \sa draw(), drawLines()
*/
void QwtPolarCurve::drawLines(QPainter *painter,
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QwtDoublePoint &pole, int from, int to) const
{
    int size = to - from + 1;
    if ( size <= 0 )
        return;

    QwtPolygon polyline(size);
    for (int i = from; i <= to; i++)
    {
        double r = radialMap.xTransform(radius(i));
        const double a = azimuthMap.xTransform(azimuth(i));
        polyline.setPoint(i - from, qwtPolar2Pos(pole, r, a).toPoint() );
    }
    painter->drawPolyline(polyline);
}

/*!
  Draw symbols

  \param painter Painter
  \param symbol Curve symbol
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param from index of the first point to be painted
  \param to index of the last point to be painted.
  \sa setSymbol(), draw(), drawCurve()
*/
void QwtPolarCurve::drawSymbols(QPainter *painter, const QwtSymbol &symbol, 
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QwtDoublePoint &pole, int from, int to) const
{
    painter->setBrush(symbol.brush());
    painter->setPen(symbol.pen());

    QRect rect(QPoint(0, 0), symbol.size());

    for (int i = from; i <= to; i++)
    {
        const double r = radialMap.xTransform(radius(i));
        const double a = azimuthMap.xTransform(azimuth(i));

        const QPoint pos = qwtPolar2Pos(pole, r, a).toPoint();

        rect.moveCenter(pos);
        symbol.draw(painter, rect);
    }

}

/*!
  Return the size of the data arrays
  \sa setData()
*/
int QwtPolarCurve::dataSize() const
{
    return d_points->size();
}

//!  Update the widget that represents the curve on the legend
void QwtPolarCurve::updateLegend(QwtLegend *legend) const
{
    if ( !legend )
        return;
        
    QwtPolarItem::updateLegend(legend);
    
    QWidget *widget = legend->find(this);
    if ( !widget || !widget->inherits("QwtLegendItem") )
        return;
        
    QwtLegendItem *legendItem = (QwtLegendItem *)widget;
    
#if QT_VERSION < 0x040000
    const bool doUpdate = legendItem->isUpdatesEnabled();
#else
    const bool doUpdate = legendItem->updatesEnabled();
#endif
    legendItem->setUpdatesEnabled(false);
    
    const int policy = legend->displayPolicy();
    
    if (policy == QwtLegend::FixedIdentifier)
    {
        int mode = legend->identifierMode();

        if (mode & QwtLegendItem::ShowLine)
            legendItem->setCurvePen(pen());

        if (mode & QwtLegendItem::ShowSymbol)
            legendItem->setSymbol(symbol());

        if (mode & QwtLegendItem::ShowText)
            legendItem->setText(title());
        else
            legendItem->setText(QwtText());

        legendItem->setIdentifierMode(mode);
    }
    else if (policy == QwtLegend::AutoIdentifier)
    {
        int mode = 0;

        if (QwtPolarCurve::NoCurve != style())
        {
            legendItem->setCurvePen(pen());
            mode |= QwtLegendItem::ShowLine;
        }
        if (QwtSymbol::NoSymbol != symbol().style())
        {
            legendItem->setSymbol(symbol());
            mode |= QwtLegendItem::ShowSymbol;
        }
        if ( !title().isEmpty() )
        {
            legendItem->setText(title());
            mode |= QwtLegendItem::ShowText;
        }
        else
        {
            legendItem->setText(QwtText());
        }
        legendItem->setIdentifierMode(mode);
    }

    legendItem->setUpdatesEnabled(doUpdate);
    legendItem->update();
}
