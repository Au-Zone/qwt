/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GPL License, Version 2.0
 *****************************************************************************/

// vim: expandtab

#include <math.h>
#include "qwt_polar_plot.h"
#include "qwt_scale_div.h"
#include "qwt_polar_magnifier.h"

QwtPolarMagnifier::QwtPolarMagnifier(QwtPolarPlot *plot):
    QwtMagnifier(plot)
{
}

//! Destructor
QwtPolarMagnifier::~QwtPolarMagnifier()
{
}

QwtPolarPlot *QwtPolarMagnifier::plot()
{
    QObject *w = parent();
    if ( w && w->inherits("QwtPolarPlot") )
        return (QwtPolarPlot *)w;

    return NULL;
}

//! Return Observed plot canvas
const QwtPolarPlot *QwtPolarMagnifier::plot() const
{
    return ((QwtPolarMagnifier *)this)->plot();
}

void QwtPolarMagnifier::rescale(double factor)
{
    if ( factor == 1.0 || factor == 0.0 )
        return;

    QwtPolarPlot* plt = plot();

    const bool autoReplot = plt->autoReplot();
    plt->setAutoReplot(false);

    QwtPolarRect rect = plt->zoomRect();
    if ( rect.isEmpty() )
        rect = plt->scaleRect();

    rect.setWidth(rect.width() * factor);
    rect.setHeight(rect.height() * factor);

    plt->setZoomRect(rect);

    plt->setAutoReplot(autoReplot);
    plt->replot();
}
