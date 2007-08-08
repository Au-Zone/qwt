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

class QwtPlotCanvas::CanvasPainter
{
public:
    CanvasPainter(QwtPlotCanvas *canvas);
    ~CanvasPainter();

    void setPaintAttribute(QwtPlotCanvas::PaintAttribute, bool on);
    bool testPaintAttribute(QwtPlotCanvas::PaintAttribute) const;

    QPaintDevice *paintCache();
    const QPaintDevice *paintCache() const;

    void drawCanvas(QPainter *);
    void drawContents(QPainter *);
    void drawFocusIndicator(QPainter *);

    void invalidatePaintCache();

    void replot();

    FocusIndicator focusIndicator;

private:
    int d_paintAttributes;
    
    QwtPaintCache *d_paintCache;
    QWidget *d_canvas;
};


QwtPlotCanvas::CanvasPainter::CanvasPainter(QwtPlotCanvas *canvas):
    focusIndicator(NoFocusIndicator),
    d_paintAttributes(0),
    d_canvas(canvas)
{
    d_paintCache = new QwtPixmapPaintCache(canvas);

    setPaintAttribute(PaintCached, true);
    setPaintAttribute(PaintPacked, true);
}

QwtPlotCanvas::CanvasPainter::~CanvasPainter()
{
    delete d_paintCache;
}

QPaintDevice *QwtPlotCanvas::CanvasPainter::paintCache()
{
    return d_paintCache->buffer();
}

//! Return the paint cache, might be null
const QPaintDevice *QwtPlotCanvas::CanvasPainter::paintCache() const
{
    return d_paintCache->buffer();
}

void QwtPlotCanvas::CanvasPainter::setPaintAttribute(
    QwtPlotCanvas::PaintAttribute attribute, bool on)
{
    if ( bool(d_paintAttributes & attribute) == on )
        return;

    if ( on )
        d_paintAttributes |= attribute;
    else
        d_paintAttributes &= ~attribute;

    switch(attribute)
    {
        case PaintCached:
        {
            if ( on )
                d_paintCache->init();
            else
                d_paintCache->clear();
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

            if ( on == false || d_canvas->isVisible() )
                ::setSystemBackground(d_canvas, !on);

            break;
        }
    }
}

bool QwtPlotCanvas::CanvasPainter::testPaintAttribute(
    QwtPlotCanvas::PaintAttribute attribute) const
{
    return (d_paintAttributes & attribute) != 0;
}


void QwtPlotCanvas::CanvasPainter::drawFocusIndicator(QPainter *painter)
{
    const int margin = 1;

    QRect focusRect = d_canvas->rect();
    focusRect.setRect(focusRect.x() + margin, focusRect.y() + margin,
        focusRect.width() - 2 * margin, focusRect.height() - 2 * margin);

    QwtPainter::drawFocusRect(painter, d_canvas, focusRect);
}

void QwtPlotCanvas::CanvasPainter::drawCanvas(QPainter *painter)
{
    if ( !d_canvas->rect().isValid() )
        return;

    QBrush bgBrush;
#if QT_VERSION >= 0x040000
        bgBrush = d_canvas->palette().brush(d_canvas->backgroundRole());
#else
    QColorGroup::ColorRole role = 
        QPalette::backgroundRoleFromMode( d_canvas->backgroundMode() );
    bgBrush = d_canvas->colorGroup().brush( role );
#endif

    if ( testPaintAttribute(QwtPlotCanvas::PaintCached) && paintCache() )
    {
        d_paintCache->reset();

        if ( testPaintAttribute(QwtPlotCanvas::PaintPacked) )
        {
            QPainter bgPainter(paintCache());
            bgPainter.setPen(Qt::NoPen);

            bgPainter.setBrush(bgBrush);

            const QRect rect(0, 0, 
                paintCache()->width(), paintCache()->height());
            bgPainter.drawRect(rect);
        }
        else
            d_paintCache->fill();

        QPainter cachePainter(d_paintCache->buffer());
        ((QwtPlot *)d_canvas->parent())->drawCanvas(&cachePainter);
        cachePainter.end();

        d_paintCache->flush(painter);
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
            painter->drawRect(d_canvas->rect());

            painter->restore();
        }

        ((QwtPlot *)d_canvas->parent())->drawCanvas(painter);
    }
}

void QwtPlotCanvas::CanvasPainter::drawContents(QPainter *painter)
{
    if ( testPaintAttribute(QwtPlotCanvas::PaintCached) && 
        d_paintCache->isValid() )
    {
        d_paintCache->flush(painter);
    }
    else
    {
        QwtPlot *plot = ((QwtPlot *)d_canvas->parent());
        const bool doAutoReplot = plot->autoReplot();
            plot->setAutoReplot(false);

        drawCanvas(painter);

        plot->setAutoReplot(doAutoReplot);
    }

    if ( d_canvas->hasFocus() && 
        focusIndicator == QwtPlotCanvas::CanvasFocusIndicator )
    {
        drawFocusIndicator(painter);
    }
}

void QwtPlotCanvas::CanvasPainter::replot()
{
    d_paintCache->invalidate();

    /*
      In case of cached or packed painting the canvas
      is repainted completely and doesn't need to be erased.
     */
    const bool erase =
        !testPaintAttribute(QwtPlotCanvas::PaintPacked)
        && !testPaintAttribute(QwtPlotCanvas::PaintCached);

#if QT_VERSION >= 0x040000
    const bool noBackgroundMode = d_canvas->testAttribute(Qt::WA_NoBackground);
    if ( !erase && !noBackgroundMode )
        d_canvas->setAttribute(Qt::WA_NoBackground, true);

    d_canvas->repaint(d_canvas->rect());

    if ( !erase && !noBackgroundMode )
        d_canvas->setAttribute(Qt::WA_NoBackground, false);
#else
    d_canvas->repaint(d_canvas->rect(), erase);
#endif
}

void QwtPlotCanvas::CanvasPainter::invalidatePaintCache()
{
    d_paintCache->invalidate();
}

//! Sets a cross cursor, enables QwtPlotCanvas::PaintCached

QwtPlotCanvas::QwtPlotCanvas(QwtPlot *plot):
    QWidget(plot)
{
    d_canvasPainter = new CanvasPainter(this);

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
    delete d_canvasPainter;
}

/*!
  \brief Changing the paint attributes

  \param attribute Paint attribute
  \param on On/Off

  The default setting enables PaintCached and PaintPacked

  \sa testPaintAttribute(), drawCanvas(), drawContents(), paintCache()
*/
void QwtPlotCanvas::setPaintAttribute(PaintAttribute attribute, bool on)
{
    d_canvasPainter->setPaintAttribute(attribute, on);
}

/*!
  Test wether a paint attribute is enabled

  \param attribute Paint attribute
  \return true if the attribute is enabled
*/
bool QwtPlotCanvas::testPaintAttribute(PaintAttribute attribute) const
{
    return d_canvasPainter->testPaintAttribute(attribute);
}

//! Return the paint cache, might be null
QPaintDevice *QwtPlotCanvas::paintCache()
{
    return d_canvasPainter->paintCache();
}

//! Return the paint cache, might be null
const QPaintDevice *QwtPlotCanvas::paintCache() const
{
    return d_canvasPainter->paintCache();
}

//! Invalidate the internal paint cache
void QwtPlotCanvas::invalidatePaintCache()
{
    d_canvasPainter->invalidatePaintCache();
}

/*!
  Set the focus indicator

  \sa FocusIndicator, focusIndicator
*/
void QwtPlotCanvas::setFocusIndicator(FocusIndicator focusIndicator)
{
    d_canvasPainter->focusIndicator = focusIndicator;
}

/*!
  \return Focus indicator
  
  \sa FocusIndicator, setFocusIndicator
*/
QwtPlotCanvas::FocusIndicator QwtPlotCanvas::focusIndicator() const
{
    return d_canvasPainter->focusIndicator;
}

void QwtPlotCanvas::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);

    if ( d_canvasPainter->testPaintAttribute(PaintPacked) )
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

    d_canvasPainter->drawContents( &painter );

    if ( d_canvasPainter->testPaintAttribute(PaintPacked) )
        ::setSystemBackground(this, false);
}

void QwtPlotCanvas::replot()
{
    d_canvasPainter->replot();
}
