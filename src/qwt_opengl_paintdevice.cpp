/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_opengl_paintdevice.h"

#if QT_VERSION < 0x050000
#define FBO_OPENGL 0
#else
#define FBO_OPENGL 1
#endif

#if FBO_OPENGL

#include <qopenglcontext.h>
#include <qopenglframebufferobject.h>
#include <qopenglpaintdevice.h>

#if QT_VERSION >= 0x050100
#include <qoffscreensurface.h>
#else
#include <qwindow.h>
#endif

#else // FBO_OPENGL

#include <qglframebufferobject.h>

#endif

class QwtOpenGLPaintDeviceFBOData
{
public:
    QwtOpenGLPaintDeviceFBOData( const QSize &size )
    {
        surface = initSurface();

#if FBO_OPENGL
        context = new QOpenGLContext();
        context->create();
        context->makeCurrent(surface);
#else
        surface->makeCurrent();
#endif

        const int numSamples = 16;

#if FBO_OPENGL
        QOpenGLFramebufferObjectFormat fboFormat;
        fboFormat.setSamples(numSamples);
#if 0
        fboFormat.setAttachment( QOpenGLFramebufferObject::CombinedDepthStencil );
#endif

        fbo = new QOpenGLFramebufferObject(size, fboFormat);
#else
        QGLFramebufferObjectFormat format;
        format.setSamples(numSamples);
#if 0
        format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);
#endif

        fbo = new QGLFramebufferObject( size, format );
#endif
#if 0
        fbo->bind();
#endif

#if FBO_OPENGL
        device = new QOpenGLPaintDevice(size);
#else
        device = fbo;
#endif
    }

    ~QwtOpenGLPaintDeviceFBOData()
    {
#if FBO_OPENGL
        delete device;
#endif
        fbo->release();
        delete fbo;

#if FBO_OPENGL
        delete context;
#endif
        delete surface;
    }

private:

#if FBO_OPENGL
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
        format.setSamples(16);

        return new QGLWidget( format );
    }
#endif

public:
#if FBO_OPENGL
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

class QwtOpenGLPaintDevice::PrivateData
{
public:
    PrivateData( const QSize &size ):
        fboData( NULL )
    {
        fboData = new QwtOpenGLPaintDeviceFBOData( size );
    }

    ~PrivateData()
    {
        delete fboData;
    }

    QwtOpenGLPaintDeviceFBOData *fboData;
};

QwtOpenGLPaintDevice::QwtOpenGLPaintDevice(const QSize &size)
{
    d_data = new PrivateData( size );
    setBaseDevice( d_data->fboData->device );
}

QwtOpenGLPaintDevice::~QwtOpenGLPaintDevice()
{
    setBaseDevice( NULL );
    delete d_data;
}

QImage QwtOpenGLPaintDevice::toImage() const
{
    return d_data->fboData->fbo->toImage();
}
