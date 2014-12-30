/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_opengl_paintdevice.h"
#include <qglpixelbuffer.h>

#if QT_VERSION >= 0x050000

#include <qopenglcontext.h>
#include <qopenglframebufferobject.h>
#include <qopenglpaintdevice.h>

#if QT_VERSION >= 0x050100
#include <qoffscreensurface.h>
#else
#include <qwindow.h>
#endif

class QwtOpenGLPaintDeviceFBOData
{
public:
    QwtOpenGLPaintDeviceFBOData( const QSize &size )
    {
        surface = initSurface();

        context = new QOpenGLContext();
        context->create();
        context->makeCurrent(surface);

        QOpenGLFramebufferObjectFormat fboFormat;
        fboFormat.setSamples(16);
        fboFormat.setAttachment( QOpenGLFramebufferObject::CombinedDepthStencil );

        fbo = new QOpenGLFramebufferObject(size, fboFormat);
        fbo->bind();

        device = new QOpenGLPaintDevice(size);
    }

    ~QwtOpenGLPaintDeviceFBOData()
    {
        delete device;

        fbo->release();
        delete fbo;
        delete context;
        delete surface;
    }
private:
    QSurface* initSurface() const
    {
#if QT_VERSION >= 0x050100
        QOffscreenSurface* surface = new QOffscreenSurface();
#else       
        QWindow *surface = new QWindow();
        surface->setSurfaceType(QWindow::OpenGLSurface);
#endif
        surface->create();
        return surface;
    }

public:
    QSurface* surface;
    QOpenGLContext* context;
    QOpenGLFramebufferObject *fbo;
    QOpenGLPaintDevice *device;
};

#endif

class QwtOpenGLPaintDevicePBOData
{
public:
    QwtOpenGLPaintDevicePBOData( const QSize &size )
    {
        QGLFormat format = QGLFormat::defaultFormat();
#if QT_VERSION < 0x050000
        format.setSampleBuffers( true );
        format.setSamples(4);
#endif
        device = new QGLPixelBuffer( size, format );
    }

    ~QwtOpenGLPaintDevicePBOData()
    {
        delete device;
    }

    QGLPixelBuffer *device;
};

class QwtOpenGLPaintDevice::PrivateData
{
public:
    enum Mode
    {
        PBO,
        FBO
    };

    PrivateData( Mode m, const QSize &size ):
        fboData( NULL ),
        pboData( NULL )
    {
        if ( m == PBO )
            pboData = new QwtOpenGLPaintDevicePBOData( size );
        else
            fboData = new QwtOpenGLPaintDeviceFBOData( size );
    }

    ~PrivateData()
    {
        delete fboData;
        delete pboData;
    }

#if QT_VERSION >= 0x050000
    QwtOpenGLPaintDeviceFBOData *fboData;
#endif
    QwtOpenGLPaintDevicePBOData *pboData;
};

QwtOpenGLPaintDevice::QwtOpenGLPaintDevice(const QSize &size)
{
#if QT_VERSION >= 0x050000
    d_data = new PrivateData( PrivateData::FBO, size );
    if ( d_data->fboData )
        setBaseDevice( d_data->fboData->device );
    else
        setBaseDevice( d_data->pboData->device );
#else
    d_data = new PrivateData( PrivateData::PBO, size );
    setBaseDevice( d_data->pboData->device );
#endif

}

QwtOpenGLPaintDevice::~QwtOpenGLPaintDevice()
{
    setBaseDevice( NULL );
    delete d_data;
}

QImage QwtOpenGLPaintDevice::toImage() const
{
#if QT_VERSION >= 0x050000
    if ( d_data->fboData )
        return d_data->fboData->fbo->toImage();
    else
        return d_data->pboData->device->toImage();
#else
    return d_data->pboData->device->toImage();
#endif
}
