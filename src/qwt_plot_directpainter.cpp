/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qpainter.h>
#include <qevent.h>
#include <qapplication.h>
#include "qwt_global.h"
#include "qwt_scale_map.h"
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_seriesitem.h"
#include "qwt_plot_directpainter.h"

class QwtPlotCurvePaintHelper: public QObject
{
public:
    QwtPlotCurvePaintHelper(QwtPlotDirectPainter *painter,
            QwtPlotAbstractSeriesItem *seriesItem, int from, int to):
        d_painter(painter),
        d_seriesItem(seriesItem),
        d_from(from),
        d_to(to)
    {
    }

    virtual bool eventFilter(QObject *, QEvent *event)
    {
        if ( event->type() == QEvent::Paint )
        {
            paintSeries();
            return true;
        }
        return false;
    }

private:
    inline void paintSeries()
    {
        QwtPlot *plot = d_seriesItem->plot();

        const QwtScaleMap xMap = plot->canvasMap(d_seriesItem->xAxis());
        const QwtScaleMap yMap = plot->canvasMap(d_seriesItem->yAxis());

        QPainter painter(plot->canvas());
#if QT_VERSION >= 0x040000
        painter.setRenderHint(QPainter::Antialiasing,
            d_seriesItem->testRenderHint(QwtPlotItem::RenderAntialiased) );
#endif
        painter.setClipping(true);
        painter.setClipRect(plot->canvas()->contentsRect());

        d_seriesItem->drawSeries(&painter, xMap, yMap,
            plot->canvas()->contentsRect(), d_from, d_to);
    }

    QwtPlotDirectPainter *d_painter;
    QwtPlotAbstractSeriesItem *d_seriesItem;
    int d_from;
    int d_to;
};

/*
    Creating and initializing a QPainter is an
    expensive operation. So we keep an painter
    open for situations, where we paint outside
    of paint events. This improves the performance
    of incremental painting like in the realtime
    example a lot.
    But it is not possible to have more than
    one QPainter open at the same time. So we
    need to close it before regular paint events
    are processed.
*/

class QwtGuardedPainter: public QObject
{
public:
    ~QwtGuardedPainter()
    {
        end();
    }

    QPainter *begin(QwtPlotCanvas *canvas)
    {
        if ( !(d_painter.isActive() && d_painter.device() == canvas) )
        {
            end();

            if ( canvas != NULL )
            {
                d_painter.begin(canvas);
                d_painter.setClipping(true);
                d_painter.setClipRect(canvas->contentsRect());

                canvas->installEventFilter(this);
            }
        }
        return &d_painter;
    }

    void end()
    {
        if ( d_painter.isActive() )
        {
            QwtPlotCanvas *canvas = (QwtPlotCanvas *)d_painter.device();
            if ( canvas )
                canvas->removeEventFilter(this);

            d_painter.end();
        }
    }

    virtual bool eventFilter(QObject *, QEvent *event)
    {
        if ( event->type() == QEvent::Paint )
            end();

        return false;
    }

private:
    QPainter d_painter;
};

class QwtPlotDirectPainter::PrivateData
{
public:
    QwtGuardedPainter guardedPainter;
};

QwtPlotDirectPainter::QwtPlotDirectPainter()
{
    d_data = new PrivateData;
}

QwtPlotDirectPainter::~QwtPlotDirectPainter()
{
    delete d_data;
}

/*!
  \brief Draw a set of points of a seriesItem.

  When observing an measurement while it is running, new points have to be
  added to an existing seriesItem. drawSeries can be used to display them avoiding
  a complete redraw of the canvas.

  Setting plot()->canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
  will result in faster painting, if the paint engine of the canvas widget
  supports this feature. 

  \param from Index of the first point to be painted
  \param to Index of the last point to be painted. If to < 0 the
         series will be painted to its last point.
*/
void QwtPlotDirectPainter::drawSeries(
    QwtPlotAbstractSeriesItem *seriesItem, int from, int to) 
{
    if ( seriesItem == NULL || seriesItem->plot() == NULL )
        return;

    QwtPlotCanvas *canvas = seriesItem->plot()->canvas();

    const QwtScaleMap xMap = seriesItem->plot()->canvasMap(seriesItem->xAxis());
    const QwtScaleMap yMap = seriesItem->plot()->canvasMap(seriesItem->yAxis());

    if ( canvas->testPaintAttribute(QwtPlotCanvas::PaintCached) &&
        canvas->paintCache() && !canvas->paintCache()->isNull() )
    {
        QPainter cachePainter((QPixmap *)canvas->paintCache());
        cachePainter.translate(-canvas->contentsRect().x(),
            -canvas->contentsRect().y());

        seriesItem->drawSeries(&cachePainter, xMap, yMap, 
            canvas->contentsRect(), from, to);
    }

    bool immediatePaint = true;
#if QT_VERSION >= 0x040000
    if ( !canvas->testAttribute(Qt::WA_WState_InPaintEvent) &&
        !canvas->testAttribute(Qt::WA_PaintOutsidePaintEvent) )
    {
        immediatePaint = false;
    }
#endif

    if ( !immediatePaint )
    {
        /*
          We save seriesItem and range in helper and call repaint.
          The helper filters the Paint event, to repeat
          the QwtPlotAbstractSeriesItem::draw, but now from inside the paint
          event.
         */

        QwtPlotCurvePaintHelper helper(this, seriesItem, from, to);
        canvas->installEventFilter(&helper);

        QPaintEvent event(canvas->contentsRect());
        QApplication::sendEvent(canvas, &event);
    }
    else
    {
        QPainter *painter = d_data->guardedPainter.begin(canvas);
#if QT_VERSION >= 0x040000
        painter->setRenderHint(QPainter::Antialiasing,
            seriesItem->testRenderHint(QwtPlotItem::RenderAntialiased) );
#endif
        seriesItem->drawSeries(painter, xMap, yMap,
            canvas->contentsRect(), from, to);
    }
}

void QwtPlotDirectPainter::reset()
{
    d_data->guardedPainter.end();
}
