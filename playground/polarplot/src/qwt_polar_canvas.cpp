/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GPL License, Version 2.0
 *****************************************************************************/

#include <qpainter.h>
#include <qevent.h>
#include "qwt_polar_canvas.h"
#include "qwt_polar_plot.h"

QwtPolarCanvas::QwtPolarCanvas(QwtPolarPlot *plot):
    QFrame(plot)
{
#if QT_VERSION >= 0x040100
    setAutoFillBackground(true);
#endif

#if QT_VERSION < 0x040000
#ifndef QT_NO_CURSOR
    setCursor(Qt::crossCursor);
#endif
#else
#ifndef QT_NO_CURSOR
    setCursor(Qt::CrossCursor);
#endif
#endif // >= 0x040000
}

QwtPolarCanvas::~QwtPolarCanvas()
{
}

QwtPolarPlot *QwtPolarCanvas::plot()
{
    QWidget *w = parentWidget();
    if ( w && w->inherits("QwtPolarPlot") )
        return (QwtPolarPlot *)w;

    return NULL;
}

const QwtPolarPlot *QwtPolarCanvas::plot() const
{
    const QWidget *w = parentWidget();
    if ( w && w->inherits("QwtPolarPlot") )
        return (QwtPolarPlot *)w;

    return NULL;
}

void QwtPolarCanvas::paintEvent(QPaintEvent *event)
{
#if QT_VERSION >= 0x040000
    QPainter painter(this);

    if ( !contentsRect().contains( event->rect() ) )
    {
        painter.save();
        painter.setClipRegion( event->region() & frameRect() );
        drawFrame( &painter );
        painter.restore();
    }

    painter.setClipRegion(event->region() & contentsRect());

    drawContents( &painter );
#else // QT_VERSION < 0x040000
    QFrame::paintEvent(event);
#endif
}

void QwtPolarCanvas::drawContents(QPainter *painter)
{
    QwtPolarPlot *plt = plot();
    if ( plt )
    {
        const bool doAutoReplot = plt->autoReplot();
        plt->setAutoReplot(false);
        plt->drawCanvas(painter, QwtDoubleRect(contentsRect()) );
        plt->setAutoReplot(doAutoReplot);
    }
}

