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

/*!
  \brief An alternative canvas for a QwtPlot derived from QGLWidget
  
  QwtPlotGLCanvas implements the very basics to act as canvas
  inside of a QwtPlot widget. It might be extended to a full
  featured alternative to QwtPlotCanvas in a future version of Qwt.

  \sa QwtPlot::setCanvas(), QwtPlotCanvas

  \note You might want to use the QPaintEngine::OpenGL paint engine
        ( see QGL::setPreferredPaintEngine() ). On a Linux test system 
        QPaintEngine::OpenGL2 shows very basic problems ( wrong
        geometries of rectangles ) but also more advanced stuff
        like antialiasing doesn't work.

  \note Another way to introduce OpenGL rendering to Qwt
        is to use QGLPixelBuffer or QGLFramebufferObject. Both
        type of buffers can be converted into a QImage and 
        used in combination with a regular QwtPlotCanvas.
*/
class QWT_EXPORT QwtPlotGLCanvas : public QGLWidget
{
    Q_OBJECT

public:
    explicit QwtPlotGLCanvas( QwtPlot * = NULL );
    virtual ~QwtPlotGLCanvas();

    Q_INVOKABLE QPainterPath borderPath( const QRect & ) const;

public Q_SLOTS:
    void replot();

protected:
    virtual void paintEvent( QPaintEvent * );
};

#endif
