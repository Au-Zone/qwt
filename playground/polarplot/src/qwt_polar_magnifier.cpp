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
#include "qwt_polar_canvas.h"
#include "qwt_scale_div.h"
#include "qwt_polar_magnifier.h"

/*!
   Constructor
   \param canvas Plot canvas to be magnified
*/
QwtPolarMagnifier::QwtPolarMagnifier(QwtPolarCanvas *canvas):
    QwtMagnifier(canvas)
{
}

//! Destructor
QwtPolarMagnifier::~QwtPolarMagnifier()
{
}

//! Return observed plot canvas
QwtPolarCanvas *QwtPolarMagnifier::canvas()
{
    QWidget *w = parentWidget();
    if ( w && w->inherits("QwtPolarCanvas") )
        return (QwtPolarCanvas *)w;

    return NULL;
}

//! Return observed plot canvas
const QwtPolarCanvas *QwtPolarMagnifier::canvas() const
{
    return ((QwtPolarMagnifier *)this)->canvas();
}

//! Return observed plot
QwtPolarPlot *QwtPolarMagnifier::plot()
{
    QwtPolarCanvas *c = canvas();
    if ( c )
        return c->plot();

    return NULL;
}

//! Return observed plot
const QwtPolarPlot *QwtPolarMagnifier::plot() const
{
    return ((QwtPolarMagnifier *)this)->plot();
}

/*! 
   Zoom in/out the zoomed area
   \param factor A value < 1.0 zooms in, a value > 1.0 zooms out.
*/
void QwtPolarMagnifier::rescale(double factor)
{
    factor = qwtAbs(factor);
    if ( factor == 1.0 || factor == 0.0 )
        return;

    QwtPolarPlot* plt = plot();

    const bool autoReplot = plt->autoReplot();
    plt->setAutoReplot(false);

    plt->zoom(plt->zoomPos(), plt->zoomFactor() * factor);

    plt->setAutoReplot(autoReplot);
    plt->replot();
}
