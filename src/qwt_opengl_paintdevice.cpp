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

#else // QT_VERSION < 0x050000

#include <qglframebufferobject.h>

#endif

class QwtOpenGLPaintDeviceFBOData
{
public:
    QwtOpenGLPaintDeviceFBOData( const QSize &size )
    {
        surface = initSurface();

#if QT_VERSION >= 0x050000
        context = new QOpenGLContext();
        context->create();
        context->makeCurrent(surface);
#else
        surface->makeCurrent();
#endif

#if QT_VERSION >= 0x050000
        QOpenGLFramebufferObjectFormat fboFormat;
        fboFormat.setSamples(16);
        fboFormat.setAttachment( QOpenGLFramebufferObject::CombinedDepthStencil );

        fbo = new QOpenGLFramebufferObject(size, fboFormat);
#else
        fbo = new QGLFramebufferObject( size, QGLFramebufferObject::CombinedDepthStencil );
#endif
        fbo->bind();

#if QT_VERSION >= 0x050000
        device = new QOpenGLPaintDevice(size);
#else
        device = fbo;
#endif
    }

    ~QwtOpenGLPaintDeviceFBOData()
    {
#if QT_VERSION >= 0x050000
        delete device;
#endif
        fbo->release();
        delete fbo;

#if QT_VERSION >= 0x050000
        delete context;
#endif
        delete surface;
    }

private:

#if QT_VERSION >= 0x050000
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
#else
    QGLWidget* initSurface() const
    {
        QGLFormat format = QGLFormat::defaultFormat();
        format.setSampleBuffers( true );
        format.setSamples(4);

        return new QGLWidget( format );
    }
#endif

public:
#if QT_VERSION >= 0x050000
    QSurface *surface;
    QOpenGLContext *context;
    QOpenGLFramebufferObject *fbo;
    QOpenGLPaintDevice *device;
#else
    QGLWidget *surface;
    QPaintDevice *device;
    QGLFramebufferObject *fbo;
#endif
};

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

    QwtOpenGLPaintDeviceFBOData *fboData;
    QwtOpenGLPaintDevicePBOData *pboData;
};

QwtOpenGLPaintDevice::QwtOpenGLPaintDevice(const QSize &size)
{
#if QT_VERSION >= 0x050000
    d_data = new PrivateData( PrivateData::FBO, size );
#else
    d_data = new PrivateData( PrivateData::PBO, size );
#endif
    if ( d_data->fboData )
        setBaseDevice( d_data->fboData->device );
    else
        setBaseDevice( d_data->pboData->device );
}

QwtOpenGLPaintDevice::~QwtOpenGLPaintDevice()
{
    setBaseDevice( NULL );
    delete d_data;
}

QImage QwtOpenGLPaintDevice::toImage() const
{
    if ( d_data->fboData )
        return d_data->fboData->fbo->toImage();
    else
        return d_data->pboData->device->toImage();
}
