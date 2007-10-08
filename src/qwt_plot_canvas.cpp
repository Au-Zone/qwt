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
#include <qstyle.h>
#if QT_VERSION >= 0x040000
#include <qstyleoption.h>
#include <qpaintengine.h>
#endif
#include <qevent.h>
#include "qwt_painter.h"
#include "qwt_math.h"
#include "qwt_plot.h"
#include "qwt_paint_cache.h"
#include "qwt_plot_canvas.h"

static void setSystemBackground(QWidget *w, bool on)
{
#if QT_VERSION < 0x040000
    if ( w->backgroundMode() == Qt::NoBackground )
    {
        if ( on )
            w->setBackgroundMode(Qt::PaletteBackground);
    }
    else
    {
        if ( !on )
            w->setBackgroundMode(Qt::NoBackground);
    }
#else
    if ( w->testAttribute(Qt::WA_NoSystemBackground) == on )
        w->setAttribute(Qt::WA_NoSystemBackground, !on);
#endif
}

class QwtPlotCanvasPainter::PrivateData
{
public:
    PrivateData(QWidget *w, QwtPaintCache *cache):
        focusIndicator(NoFocusIndicator),
        paintAttributes(0),
        paintCache(cache),
        canvas(w)
    {
    }

    ~PrivateData()
    {
        delete paintCache;
    }

    FocusIndicator focusIndicator;
    int paintAttributes;
    
    QwtPaintCache *paintCache;
    QWidget *canvas;
};

QwtPlotCanvasPainter::QwtPlotCanvasPainter(
    QWidget *canvas, QwtPaintCache *paintCache)
{
    d_data = new PrivateData(canvas, paintCache);

    setPaintAttribute(PaintCached, true);
    setPaintAttribute(PaintPacked, true);
}

QwtPlotCanvasPainter::~QwtPlotCanvasPainter()
{
    delete d_data;
}

QwtPaintCache *QwtPlotCanvasPainter::paintCache()
{
    return d_data->paintCache;
}

const QwtPaintCache *QwtPlotCanvasPainter::paintCache() const
{
    return d_data->paintCache;
}

void QwtPlotCanvasPainter::setPaintAttribute(
    QwtPlotCanvas::PaintAttribute attribute, bool on)
{
    if ( bool(d_data->paintAttributes & attribute) == on )
        return;

    if ( on )
        d_data->paintAttributes |= attribute;
    else
        d_data->paintAttributes &= ~attribute;

    switch(attribute)
    {
        case PaintCached:
        {
            d_data->paintCache->setEnabled(on);
            if ( on )
            {
                // fill the cache with the canvas content
                d_data->paintCache->sync(true); 
            }
            break;
        }
        case PaintPacked:
        {
            /*
              If not visible, changing of the background mode
              is delayed until it becomes visible. This tries to avoid 
              looking through the canvas when the canvas is shown the first 
              time.
             */

            if ( on == false || d_data->canvas->isVisible() )
                ::setSystemBackground(d_data->canvas, !on);

            break;
        }
    }
}

bool QwtPlotCanvasPainter::testPaintAttribute(
    QwtPlotCanvas::PaintAttribute attribute) const
{
    return (d_data->paintAttributes & attribute) != 0;
}

/*!
  Set the focus indicator

  \sa FocusIndicator, focusIndicator
*/
void QwtPlotCanvasPainter::setFocusIndicator(FocusIndicator focusIndicator)
{
    d_data->focusIndicator = focusIndicator;
}

/*!
  \return Focus indicator
  
  \sa FocusIndicator, setFocusIndicator
*/
QwtPlotCanvas::FocusIndicator QwtPlotCanvasPainter::focusIndicator() const
{
    return d_data->focusIndicator;
}

void QwtPlotCanvasPainter::drawFocusIndicator(QPainter *painter)
{
    const int margin = 1;

    QRect focusRect = d_data->canvas->rect();
    focusRect.setRect(focusRect.x() + margin, focusRect.y() + margin,
        focusRect.width() - 2 * margin, focusRect.height() - 2 * margin);

    QwtPainter::drawFocusRect(painter, d_data->canvas, focusRect);
}

void QwtPlotCanvasPainter::drawCanvas(QPainter *painter)
{
    if ( !d_data->canvas->rect().isValid() )
        return;

    QBrush bgBrush;
#if QT_VERSION >= 0x040000
        bgBrush = d_data->canvas->palette().brush(d_data->canvas->backgroundRole());
#else
    QColorGroup::ColorRole role = 
        QPalette::backgroundRoleFromMode( d_data->canvas->backgroundMode() );
    bgBrush = d_data->canvas->colorGroup().brush( role );
#endif

    if ( testPaintAttribute(PaintCached) && paintCache() )
    {
        d_data->paintCache->sync(false);

        QPainter cachePainter(d_data->paintCache->buffer());

        cachePainter.save();
        cachePainter.setPen(Qt::NoPen);
        cachePainter.setBrush(bgBrush);

        cachePainter.drawRect(cachePainter.window());
        cachePainter.restore();

        ((QwtPlot *)d_data->canvas->parent())->drawCanvas(&cachePainter);
        cachePainter.end();

        d_data->paintCache->flush(painter);
    }
    else
    {
#if QT_VERSION >= 0x040000
        if ( testPaintAttribute(QwtPlotCanvas::PaintPacked) )
#endif
        {
            painter->save();

            painter->setPen(Qt::NoPen);
            painter->setBrush(bgBrush);
            painter->drawRect(d_data->canvas->rect());

            painter->restore();
        }

        ((QwtPlot *)d_data->canvas->parent())->drawCanvas(painter);
    }
}

void QwtPlotCanvasPainter::drawContents(QPainter *painter)
{
    if ( testPaintAttribute(PaintCached) && 
        d_data->paintCache->isValid() )
    {
        d_data->paintCache->flush(painter);
    }
    else
    {
        QwtPlot *plot = ((QwtPlot *)d_data->canvas->parent());
        const bool doAutoReplot = plot->autoReplot();
            plot->setAutoReplot(false);

        drawCanvas(painter);

        plot->setAutoReplot(doAutoReplot);
    }

    if ( d_data->canvas->hasFocus() && 
        d_data->focusIndicator == CanvasFocusIndicator )
    {
        drawFocusIndicator(painter);
    }
}

void QwtPlotCanvasPainter::replot()
{
    d_data->paintCache->invalidate();

    /*
      In case of cached or packed painting the canvas
      is repainted completely and doesn't need to be erased.
     */
    const bool erase =
        !testPaintAttribute(QwtPlotCanvas::PaintPacked)
        && !testPaintAttribute(PaintCached);

    QWidget *w = d_data->canvas;

#if QT_VERSION >= 0x040000
    const bool noBackgroundMode = 
        w->testAttribute(Qt::WA_NoBackground);
    if ( !erase && !noBackgroundMode )
        w->setAttribute(Qt::WA_NoBackground, true);

    w->repaint(d_data->canvas->rect());

    if ( !erase && !noBackgroundMode )
        w->setAttribute(Qt::WA_NoBackground, false);
#else
    w->repaint(w->rect(), erase);
#endif
}

//! Sets a cross cursor, enables QwtPlotCanvasPainter::PaintCached

QwtPlotCanvas::QwtPlotCanvas(QwtPlot *plot):
#if QWT_GLCANVAS
    QGLWidget(plot),
#else
    QWidget(plot),
#endif
    QwtPlotCanvasPainter(this, new QwtPixmapPaintCache(this) )
{
#if QT_VERSION >= 0x040100
    setAutoFillBackground(true);
#endif

#if QT_VERSION < 0x040000
    setWFlags(Qt::WNoAutoErase);
#ifndef QT_NO_CURSOR
    setCursor(Qt::crossCursor);
#endif
#else
#ifndef QT_NO_CURSOR
    setCursor(Qt::CrossCursor);
#endif
#endif // >= 0x040000
}

//! Destructor
QwtPlotCanvas::~QwtPlotCanvas()
{
}

void QwtPlotCanvas::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);

    if ( testPaintAttribute(PaintPacked) )
    {
        // enable system background to avoid the "looking through
        // the canvas" effect, for the next show

        ::setSystemBackground(this, true);
    }
}

void QwtPlotCanvas::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setClipRegion(event->region());

    drawContents( &painter );

    if ( testPaintAttribute(PaintPacked) )
        ::setSystemBackground(this, false);
}
