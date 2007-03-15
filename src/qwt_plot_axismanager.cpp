/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#include <qevent.h>
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"
#include "qwt_scale_div.h"
#include "qwt_plot_axismanager.h"
#if 1
#include <iostream>
using namespace std;
#endif

class QwtPlotAxisManager::PrivateData
{
public:
    PrivateData():
        xAxis(QwtPlot::xBottom),
        yAxis(QwtPlot::yLeft),
        mode(QwtPlotAxisManager::KeepAspectRatio)
    {
    }

    int xAxis;
    int yAxis;
    AspectRatioMode mode;
    QRect rectOfInterest;
};

QwtPlotAxisManager::QwtPlotAxisManager(QwtPlotCanvas *canvas):
    QObject(canvas)
{
    d_data = new PrivateData;
    canvas->installEventFilter(this);
}

QwtPlotAxisManager::~QwtPlotAxisManager()
{
    delete d_data;
}

void QwtPlotAxisManager::setAxis(int xAxis, int yAxis)
{
    d_data->xAxis = xAxis;
    d_data->yAxis = yAxis;
}

int QwtPlotAxisManager::xAxis() const
{
    return d_data->xAxis;
}

int QwtPlotAxisManager::yAxis() const
{
    return d_data->yAxis;
}

void QwtPlotAxisManager::setAspectRatioMode(AspectRatioMode mode)
{
    d_data->mode = mode;
}

QwtPlotAxisManager::AspectRatioMode 
QwtPlotAxisManager::aspectRatioMode() const
{
    return d_data->mode;
}

QwtPlotCanvas *QwtPlotAxisManager::canvas()
{
    QObject *o = parent();
    if ( o && o->inherits("QwtPlotCanvas") )
        return (QwtPlotCanvas *)o;

    return NULL;
}

const QwtPlotCanvas *QwtPlotAxisManager::canvas() const
{
    return ((QwtPlotAxisManager *)this)->canvas();
}

QwtPlot *QwtPlotAxisManager::plot()
{
    QObject *w = canvas();
    if ( w )
    {
        w = w->parent();
        if ( w && w->inherits("QwtPlot") )
            return (QwtPlot *)w;
    }

    return NULL;
}

const QwtPlot *QwtPlotAxisManager::plot() const
{
    return ((QwtPlotAxisManager *)this)->plot();
}

bool QwtPlotAxisManager::eventFilter(QObject *o, QEvent *e)
{
    if ( o && o == canvas() )
    {
        if ( e->type() == QEvent::Resize )
            canvasResizeEvent((QResizeEvent *)e);
    }

    return false;
}

void QwtPlotAxisManager::canvasResizeEvent(QResizeEvent* e)
{
    if ( !e->oldSize().isValid() )
        return;

    QwtPlot *plt = plot();
    double x1 = plt->axisScaleDiv(xAxis())->lBound();
    double x2 = plt->axisScaleDiv(xAxis())->hBound();
    double y1 = plt->axisScaleDiv(yAxis())->lBound();
    double y2 = plt->axisScaleDiv(yAxis())->hBound();

    const int fw = 2 * plt->canvas()->frameWidth();

    const QSize newSize = e->size() - QSize(fw, fw);
    const QSize oldSize = e->oldSize() - QSize(fw, fw);

    const QwtDoubleRect rect(x1, y1, x2 - x1, y2 - y1);

    const QwtDoubleRect scaledRect = 
        rescale(oldSize, rect, newSize);

    if ( rect == scaledRect )
        return;

    x1 = scaledRect.x();
    x2 = scaledRect.x() + scaledRect.width();
    y1 = scaledRect.y();
    y2 = scaledRect.y() + scaledRect.height();

    if ( plt->axisScaleDiv(xAxis())->lBound() >
         plt->axisScaleDiv(xAxis())->hBound() )
    {
        qSwap(x1, x2);
    }

    if ( plt->axisScaleDiv(yAxis())->lBound() >
         plt->axisScaleDiv(yAxis())->hBound() )
    {
        qSwap(y1, y2);
    }

    const bool doReplot = plt->autoReplot();
    plt->setAutoReplot(false);

    plt->setAxisScale(xAxis(), x1, x2);
    plt->setAxisScale(yAxis(), y1, y2);

    plt->setAutoReplot(doReplot);
    plt->replot();
}

QwtDoubleRect QwtPlotAxisManager::rescale( const QSize &oldSize, 
    const QwtDoubleRect &rect, const QSize &newSize) const
{
    if ( !rect.isValid() || !oldSize.isValid() || !newSize.isValid() )
        return rect;

    int mode = 2;

    QwtDoubleRect scaledRect = rect;

    switch(mode)
    {
        case 1:
        {
            const double xRatio = rect.width() / oldSize.width();
            const double yRatio = rect.height() / oldSize.height();

            scaledRect.setWidth(newSize.width() * xRatio);
            scaledRect.setHeight(newSize.height() * yRatio);
            break;
        }
        case 2:
        {
            const double xRatio = double(newSize.width()) / oldSize.width();
            const double yRatio = double(newSize.height()) / oldSize.height();

            scaledRect.setWidth(scaledRect.width() * xRatio);
            scaledRect.setHeight(scaledRect.height() * yRatio);
            break;
        }
    }

    return scaledRect;
}
