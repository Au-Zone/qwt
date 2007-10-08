/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PAINT_CACHE_H
#define QWT_PAINT_CACHE_H 1

#include "qwt_global.h"

class QPainter;
class QPixmap;
class QWidget;

/*!
  \brief Abstract buffer for paint operations

  QwtPaintCache provides an common API for using a QPixmap or
  QGLPixelBuffer as paint cache. It is intended to assist the 
  implementation of a OpenGL and "regular" plot canvas, but could
  be useful for similar situations.

  \sa QwtPlotCanvas, QwtPixmapPaintCache
*/

class QWT_EXPORT QwtPaintCache
{
public:
    virtual ~QwtPaintCache();

    virtual void setEnabled(bool on) = 0;

    virtual void sync(bool doFill) = 0;
    virtual void flush(QPainter *) = 0;

    virtual void invalidate() = 0;
    bool isValid() const;

    QPaintDevice *buffer();
    const QPaintDevice *buffer() const;

    QWidget *widget();
    const QWidget *widget() const;

protected:
    explicit QwtPaintCache(QWidget *widget);
    void setBuffer(QPaintDevice *buffer);

private:
    QPaintDevice *d_buffer;
    QWidget *d_widget;
};

class QWT_EXPORT QwtPixmapPaintCache: public QwtPaintCache
{
public:
    QwtPixmapPaintCache(QWidget *);
    virtual ~QwtPixmapPaintCache();

    virtual void setEnabled(bool on);

    virtual void sync(bool doFill);
    virtual void flush(QPainter *);

    virtual void invalidate();

private:
    QPixmap *initPixmap() const;
};

#endif
