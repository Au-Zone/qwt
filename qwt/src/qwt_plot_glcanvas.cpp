/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_glcanvas.h"
#include "qwt_plot.h"
#include <qevent.h>
#include <qpainter.h>

QwtPlotGLCanvas::QwtPlotGLCanvas( QwtPlot *plot ):
    QGLWidget( plot )
{
#ifndef QT_NO_CURSOR
    setCursor( Qt::CrossCursor );
#endif

    setAutoFillBackground( true );
}

QwtPlotGLCanvas::~QwtPlotGLCanvas()
{
}

bool QwtPlotGLCanvas::event( QEvent *event )
{
    return QGLWidget::event( event );
}

void QwtPlotGLCanvas::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
	painter.setClipRegion( event->region() );

	QwtPlot *plot = qobject_cast< QwtPlot *>( parent() );
	if ( plot )
    	plot->drawCanvas( &painter );
}

void QwtPlotGLCanvas::replot()
{
	repaint( contentsRect() );
}
