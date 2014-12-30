/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_OPENGL_PAINT_DEVICE_H
#define QWT_OPENGL_PAINT_DEVICE_H

#include "qwt_global.h"
#include "qwt_proxy_paintdevice.h"
#include <QImage>

class QWT_EXPORT QwtOpenGLPaintDevice: public QwtProxyPaintDevice
{
public:
    explicit QwtOpenGLPaintDevice(const QSize &size);
    virtual ~QwtOpenGLPaintDevice();

    QImage toImage() const;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
