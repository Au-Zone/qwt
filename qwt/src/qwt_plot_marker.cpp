/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#include <qpainter.h>
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_plot_marker.h"
#include "qwt_symbol.h"
#include "qwt_text.h"
#include "qwt_math.h"

static const int LabelDist = 2;

class QwtPlotMarker::PrivateData
{
public:
    PrivateData():
        align(Qt::AlignCenter),
        style(NoLine),
        xValue(0.0),
        yValue(0.0)
    {
        symbol = new QwtSymbol();
    }

    ~PrivateData()
    {
        delete symbol;
    }

    QwtText label;
#if QT_VERSION < 0x040000
    int align;
#else
    Qt::Alignment align;
#endif
    QPen pen;
    QwtSymbol *symbol;
    LineStyle style;

    double xValue;
    double yValue;
};

//! Sets alignment to Qt::AlignCenter, and style to NoLine
QwtPlotMarker::QwtPlotMarker():
    QwtPlotItem(QwtText("Marker"))
{
    d_data = new PrivateData;
    setZ(30.0);
}

//! Destructor
QwtPlotMarker::~QwtPlotMarker()
{
    delete d_data;
}

//! \return QwtPlotItem::Rtti_PlotMarker
int QwtPlotMarker::rtti() const
{
    return QwtPlotItem::Rtti_PlotMarker;
}

//! Return Value
QwtDoublePoint QwtPlotMarker::value() const
{
    return QwtDoublePoint(d_data->xValue, d_data->yValue);
}

//! Return x Value
double QwtPlotMarker::xValue() const 
{ 
    return d_data->xValue; 
}

//! Return y Value
double QwtPlotMarker::yValue() const 
{ 
    return d_data->yValue; 
}

//! Set Value
void QwtPlotMarker::setValue(const QwtDoublePoint& pos)
{
    setValue(pos.x(), pos.y());
}

//! Set Value
void QwtPlotMarker::setValue(double x, double y) 
{
    if ( x != d_data->xValue || y != d_data->yValue )
    {
        d_data->xValue = x; 
        d_data->yValue = y; 
        itemChanged(); 
    }
}

//! Set X Value
void QwtPlotMarker::setXValue(double x) 
{ 
    setValue(x, d_data->yValue);
}

//! Set Y Value
void QwtPlotMarker::setYValue(double y) 
{ 
    setValue(d_data->xValue, y);
}

/*!
  Draw the marker

  \param painter Painter
  \param xMap x Scale Map
  \param yMap y Scale Map
  \param canvasRect Contents rect of the canvas in painter coordinates
*/
void QwtPlotMarker::draw(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRect &canvasRect) const
{
    const int x = xMap.transform(d_data->xValue);
    const int y = yMap.transform(d_data->yValue);

    drawAt(painter, canvasRect, QPoint(x, y));
}

/*!
  Draw the marker at a specific position

  \param painter Painter
  \param canvasRect Contents rect of the canvas in painter coordinates
  \param pos Position of the marker in painter coordinates
*/
void QwtPlotMarker::drawAt(QPainter *painter, 
    const QRect &canvasRect, const QPoint &pos) const
{
    // draw lines
    if (d_data->style != NoLine)
    {
        painter->setPen(QwtPainter::scaledPen(d_data->pen));
        if ((d_data->style == HLine) || (d_data->style == Cross))
        {
            QwtPainter::drawLine(painter, canvasRect.left(), 
                pos.y(), canvasRect.right(), pos.y() );
        }
        if ((d_data->style == VLine) || (d_data->style == Cross))
        {
            QwtPainter::drawLine(painter, pos.x(), 
                canvasRect.top(), pos.x(), canvasRect.bottom());
        }
    }

    // draw symbol
    QSize symbolSize(0, 0);
    if (d_data->symbol->style() != QwtSymbol::NoSymbol)
    {
        symbolSize = d_data->symbol->size();
        d_data->symbol->draw(painter, pos.x(), pos.y());
    }

    // draw label
    if (!d_data->label.isEmpty())
    {
        int xlw = qwtMax(int(d_data->pen.width()), 1);
        int ylw = xlw;
        int xlw1;
        int ylw1;

        const int xLabelDist = 
            QwtPainter::metricsMap().screenToLayoutX(LabelDist);
        const int yLabelDist = 
            QwtPainter::metricsMap().screenToLayoutY(LabelDist);

        if ((d_data->style == VLine) || (d_data->style == HLine))
        {
            xlw1 = (xlw + 1) / 2 + xLabelDist;
            xlw = xlw / 2 + xLabelDist;
            ylw1 = (ylw + 1) / 2 + yLabelDist;
            ylw = ylw / 2 + yLabelDist;
        }
        else 
        {
            xlw1 = qwtMax((xlw + 1) / 2, (symbolSize.width() + 1) / 2) + xLabelDist;
            xlw = qwtMax(xlw / 2, (symbolSize.width() + 1) / 2) + xLabelDist;
            ylw1 = qwtMax((ylw + 1) / 2, (symbolSize.height() + 1) / 2) + yLabelDist;
            ylw = qwtMax(ylw / 2, (symbolSize. height() + 1) / 2) + yLabelDist;
        }

        QRect tr(QPoint(0, 0), d_data->label.textSize(painter->font()));
        tr.moveCenter(QPoint(0, 0));

        int dx = pos.x();
        int dy = pos.y();

        if (d_data->style == VLine)
        {
            if (d_data->align & (int) Qt::AlignTop)
                dy = canvasRect.top() + yLabelDist - tr.y();
            else if (d_data->align & (int) Qt::AlignBottom)
                dy = canvasRect.bottom() - yLabelDist + tr.y();
            else
                dy = canvasRect.top() + canvasRect.height() / 2;
        }
        else
        {
            if (d_data->align & (int) Qt::AlignTop)
                dy += tr.y() - ylw1;
            else if (d_data->align & (int) Qt::AlignBottom)
                dy -= tr.y() - ylw1;
        }


        if (d_data->style == HLine)
        {
            if (d_data->align & (int) Qt::AlignLeft)
                dx = canvasRect.left() + xLabelDist - tr.x();
            else if (d_data->align & (int) Qt::AlignRight)
                dx = canvasRect.right() - xLabelDist + tr.x();
            else
                dx = canvasRect.left() + canvasRect.width() / 2;
        }
        else
        {
            if (d_data->align & (int) Qt::AlignLeft)
                dx += tr.x() - xlw1;
            else if (d_data->align & (int) Qt::AlignRight)
                dx -= tr.x() - xlw1;
        }

#if QT_VERSION < 0x040000
        tr.moveBy(dx, dy);
#else
        tr.translate(dx, dy);
#endif
        d_data->label.draw(painter, tr);
    }
}

/*!
  \brief Set the line style
  \param st Line style. Can be one of QwtPlotMarker::NoLine,
    HLine, VLine or Cross
  \sa lineStyle()
*/
void QwtPlotMarker::setLineStyle(QwtPlotMarker::LineStyle st)
{
    if ( st != d_data->style )
    {
        d_data->style = st;
        itemChanged();
    }
}

/*!
  \return the line style
  \sa For a description of line styles, see QwtPlotMarker::setLineStyle()
*/
QwtPlotMarker::LineStyle QwtPlotMarker::lineStyle() const 
{ 
    return d_data->style; 
}

/*!
  \brief Assign a symbol
  \param s New symbol 
  \sa symbol()
*/
void QwtPlotMarker::setSymbol(const QwtSymbol &s)
{
    delete d_data->symbol;
    d_data->symbol = s.clone();
    itemChanged();
}

/*!
  \return the symbol
  \sa setSymbol(), QwtSymbol
*/
const QwtSymbol &QwtPlotMarker::symbol() const 
{ 
    return *d_data->symbol; 
}

/*!
  \brief Set the label
  \param label label text
  \sa label()
*/
void QwtPlotMarker::setLabel(const QwtText& label)
{
    if ( label != d_data->label )
    {
        d_data->label = label;
        itemChanged();
    }
}

/*!
  \return the label
  \sa setLabel()
*/
QwtText QwtPlotMarker::label() const 
{ 
    return d_data->label; 
}

/*!
  \brief Set the alignment of the label

  The alignment determines where the label is drawn relative to
  the marker's position.

  \param align Alignment. A combination of AlignTop, AlignBottom,
    AlignLeft, AlignRight, AlignCenter, AlgnHCenter,
    AlignVCenter.  
  \sa labelAlignment()
*/
#if QT_VERSION < 0x040000
void QwtPlotMarker::setLabelAlignment(int align)
#else
void QwtPlotMarker::setLabelAlignment(Qt::Alignment align)
#endif
{
    if ( align == d_data->align )
        return;
    
    d_data->align = align;
    itemChanged();
}

/*!
  \return the label alignment
  \sa setLabelAlignment()
*/
#if QT_VERSION < 0x040000
int QwtPlotMarker::labelAlignment() const 
#else
Qt::Alignment QwtPlotMarker::labelAlignment() const 
#endif
{ 
    return d_data->align; 
}

/*!
  Specify a pen for the line.

  The width of non cosmetic pens is scaled according to the resolution
  of the paint device.

  \param pen New pen
  \sa linePen(), QwtPainter::scaledPen()
*/
void QwtPlotMarker::setLinePen(const QPen &pen)
{
    if ( pen != d_data->pen )
    {
        d_data->pen = pen;
        itemChanged();
    }
}

/*!
  \return the line pen
  \sa setLinePen()
*/
const QPen &QwtPlotMarker::linePen() const 
{ 
    return d_data->pen; 
}

QwtDoubleRect QwtPlotMarker::boundingRect() const
{
    return QwtDoubleRect(d_data->xValue, d_data->yValue, 0.0, 0.0);
}
