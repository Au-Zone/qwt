/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_GLCANVAS_H
#define QWT_PLOT_GLCANVAS_H

#include "qwt_global.h"
#include <qgl.h>

class QwtPlot;

class QWT_EXPORT QwtPlotGLCanvas : public QGLWidget
{
    Q_OBJECT

public:
    explicit QwtPlotGLCanvas( QwtPlot * = NULL );
    virtual ~QwtPlotGLCanvas();

	virtual bool event( QEvent * );

public Q_SLOTS:
    void replot();

protected:
	virtual void paintEvent( QPaintEvent * );
};

#endif
