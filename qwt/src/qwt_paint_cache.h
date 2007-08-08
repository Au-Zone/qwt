/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PAINT_BUFFER_H
#define QWT_PAINT_BUFFER_H 1

#include "qwt_global.h"

class QPainter;
class QPixmap;
class QWidget;

class QWT_EXPORT QwtPaintCache
{
public:
    virtual ~QwtPaintCache();

    virtual void init() = 0;
    virtual void reset() = 0;
    virtual void fill() = 0;
    virtual void flush(QPainter *) = 0;

    virtual void invalidate() = 0;

    QPaintDevice *buffer();
    const QPaintDevice *buffer() const;

    QWidget *widget();
    const QWidget *widget() const;

    void clear();
    bool isValid() const;

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

    virtual void init();
    virtual void reset();
    virtual void fill();
    virtual void flush(QPainter *);

    virtual void invalidate();
};

#endif
