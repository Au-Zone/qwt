/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/


#include <qpainter.h>
#include <qwidget.h>
#if QT_VERSION >= 0x040000
#ifdef Q_WS_X11
#include <qx11info_x11.h>
#endif
#endif
#include "qwt_paint_cache.h"

QwtPaintCache::QwtPaintCache(QWidget *widget):
    d_buffer(NULL),
    d_widget(widget)
{
}

QwtPaintCache::~QwtPaintCache()
{
    delete d_buffer;
}

QPaintDevice *QwtPaintCache::buffer()
{
    return d_buffer;
}

const QPaintDevice *QwtPaintCache::buffer() const
{
    return d_widget;
}

QWidget *QwtPaintCache::widget()
{
    return d_widget;
}

const QWidget *QwtPaintCache::widget() const
{
    return d_widget;
}

void QwtPaintCache::setBuffer(QPaintDevice *buffer)
{
    delete d_buffer;
    d_buffer = buffer;
}

bool QwtPaintCache::isValid() const
{
    return d_buffer && d_buffer->width() == d_widget->width() 
        && d_buffer->height() == d_widget->height();
}

QwtPixmapPaintCache::QwtPixmapPaintCache(QWidget *widget):
    QwtPaintCache(widget)
{
}

QwtPixmapPaintCache::~QwtPixmapPaintCache()
{
}

void QwtPixmapPaintCache::setEnabled(bool on)
{
    if ( on )
    {
        if ( buffer() == NULL )
            setBuffer(initPixmap());
    }
    else
        setBuffer(NULL);
}

void QwtPixmapPaintCache::sync(bool doFill)
{
    QPixmap *pm = (QPixmap *)buffer();
    QWidget *w = widget();

    if ( doFill )
    {

        const QRect cr = w->rect();
        *pm = QPixmap::grabWidget(w,
            cr.x(), cr.y(), cr.width(), cr.height() );
    }
    else
        *pm = QPixmap(w->size());

}

void QwtPixmapPaintCache::invalidate()
{
    QPixmap *pm = (QPixmap *)buffer();
    if ( pm )
        *pm = QPixmap();
}

void QwtPixmapPaintCache::flush(QPainter *painter)
{
    QPixmap *pm = (QPixmap *)buffer();
    painter->drawPixmap(0, 0, *pm);
}

QPixmap *QwtPixmapPaintCache::initPixmap() const
{
    const QWidget *w = widget();
    QPixmap *pm = new QPixmap();
#ifdef Q_WS_X11
#if QT_VERSION >= 0x040000
    if ( pm->x11Info().screen() != w->x11Info().screen() )
        pm->x11SetScreen(w->x11Info().screen());
#else
    if ( pm->x11Screen() != w->x11Screen() )
        pm->x11SetScreen(w->x11Screen());
#endif
#endif

    return pm;
}
