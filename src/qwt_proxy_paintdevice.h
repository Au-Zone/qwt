/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PROXY_PAINT_DEVICE_H
#define QWT_PROXY_PAINT_DEVICE_H

#include "qwt_global.h"
#include <qpaintdevice.h>

class QWT_EXPORT QwtProxyPaintDevice: public QPaintDevice
{
public:
    explicit QwtProxyPaintDevice();
    virtual ~QwtProxyPaintDevice();

    QPaintDevice *baseDevice();
    const QPaintDevice *baseDevice() const;

    virtual QPaintEngine *paintEngine() const;
    virtual int metric(PaintDeviceMetric metric) const;

protected:
    void setBaseDevice( QPaintDevice * );

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
