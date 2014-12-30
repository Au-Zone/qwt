/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_proxy_paintdevice.h"
#include <qpaintengine.h>

class QwtProxyPaintEngine: public QPaintEngine
{
public:
    QwtProxyPaintEngine( QwtProxyPaintDevice *paintDevice ):
        QPaintEngine( QPaintEngine::AllFeatures ),
        d_device( paintDevice )
    {
    }

    virtual ~QwtProxyPaintEngine()
    {
    }

    virtual bool begin( QPaintDevice * )
    {
        return d_painter.begin( d_device->baseDevice() );
    }

    virtual bool end()
    {
        return d_painter.end();
    }

    virtual Type type () const
    {
        return d_device->baseDevice()->paintEngine()->type();
    }

    virtual void updateState(const QPaintEngineState &state)
    {
        const QPaintEngine::DirtyFlags flags = state.state();

        if ( flags & QPaintEngine::DirtyPen )
            d_painter.setPen( state.pen() );

        if ( flags & QPaintEngine::DirtyBrush )
            d_painter.setBrush( state.brush() );

        if ( flags & QPaintEngine::DirtyBrushOrigin )
            d_painter.setBrushOrigin( state.brushOrigin() );

        if ( flags & QPaintEngine::DirtyFont )
            d_painter.setFont( state.font() );

        if ( flags & QPaintEngine::DirtyBackground )
            d_painter.setBackground( state.backgroundBrush() );

        if ( flags & QPaintEngine::DirtyBackgroundMode )
            d_painter.setBackgroundMode( state.backgroundMode() );

        if ( flags & QPaintEngine::DirtyTransform )
            d_painter.setTransform( state.transform() );

        if ( flags & QPaintEngine::DirtyClipRegion )
            d_painter.setClipRegion( state.clipRegion() );

        if ( flags & QPaintEngine::DirtyClipPath )
            d_painter.setClipPath( state.clipPath() );

        if ( flags & QPaintEngine::DirtyHints )
            d_painter.setRenderHints( state.renderHints() );

        if ( flags & QPaintEngine::DirtyCompositionMode )
            d_painter.setCompositionMode( state.compositionMode() );

        if ( flags & QPaintEngine::DirtyClipEnabled )
            d_painter.setClipping( state.isClipEnabled() );

        if ( flags & QPaintEngine::DirtyOpacity )
            d_painter.setOpacity( state.opacity() );
    }

    virtual void drawRects( const QRect *rects, int rectCount )
    {
        d_painter.drawRects( rects, rectCount );
    }

    virtual void drawRects(const QRectF *rects, int rectCount )
    {
        d_painter.drawRects( rects, rectCount );
    }

    virtual void drawLines(const QLine *lines, int lineCount )
    {
        d_painter.drawLines( lines, lineCount );
    }

    virtual void drawLines(const QLineF *lines, int lineCount )
    {
        d_painter.drawLines( lines, lineCount );
    }

    virtual void drawEllipse(const QRectF &rect)
    {
        d_painter.drawEllipse( rect );
    }

    virtual void drawEllipse(const QRect &rect)
    {
        d_painter.drawEllipse( rect );
    }

    virtual void drawPath(const QPainterPath &path)
    {
        d_painter.drawPath( path );
    }

    virtual void drawPoints(const QPointF *points, int pointCount )
    {
        d_painter.drawPoints( points, pointCount );
    }

    virtual void drawPoints(const QPoint *points, int pointCount )
    {
        d_painter.drawPoints( points, pointCount );
    }

    virtual void drawPolygon(const QPointF *points, int pointCount , PolygonDrawMode mode )
    {
        switch( mode )
        {
            case QPaintEngine::OddEvenMode:
            {
                d_painter.drawPolygon( points, pointCount, Qt::OddEvenFill );
                break;
            }
            case QPaintEngine::WindingMode:
            {
                d_painter.drawPolygon( points, pointCount, Qt::WindingFill );
                break;
            }
            case QPaintEngine::PolylineMode:
            {
                d_painter.drawPolyline( points, pointCount );
                break;
            }
            case QPaintEngine::ConvexMode:
            default:
                // should never happen as ConvexMode is only a fallback,
                // when QPaintEngine::PainterPaths is not available
                break;
        }
    }

    virtual void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode )
    {
        switch( mode )
        {
            case QPaintEngine::OddEvenMode:
            {
                d_painter.drawPolygon( points, pointCount, Qt::OddEvenFill );
                break;
            }
            case QPaintEngine::WindingMode:
            {
                d_painter.drawPolygon( points, pointCount, Qt::WindingFill );
                break;
            }
            case QPaintEngine::PolylineMode:
            {
                d_painter.drawPolyline( points, pointCount );
                break;
            }
            case QPaintEngine::ConvexMode:
            default:
                // should never happen as ConvexMode is only a fallback,
                // when QPaintEngine::PainterPaths is not available
                break;
        }
    }

    virtual void drawPixmap(const QRectF &rect,
        const QPixmap &pm, const QRectF &subRect)
    {
        d_painter.drawPixmap( rect, pm, subRect );
    }

    virtual void drawTextItem(const QPointF &pos, const QTextItem &textItem)
    {
        d_painter.drawTextItem( pos, textItem );
    }

    virtual void drawTiledPixmap(const QRectF &rect,
        const QPixmap &pixmap, const QPointF &subRect)
    {
        d_painter.drawTiledPixmap( rect, pixmap, subRect );
    }

    virtual void drawImage(const QRectF &rect,
        const QImage &image, const QRectF &subRect, Qt::ImageConversionFlags flags )
    {
        d_painter.drawImage( rect, image, subRect, flags );
    }

private:
    QPainter d_painter;
    QwtProxyPaintDevice* d_device;
};

class QwtProxyPaintDevice::PrivateData
{
public:
    PrivateData():
        engine( NULL )
    {
    }

    ~PrivateData()
    {
        delete engine;
    }

public:
    QwtProxyPaintEngine* engine;
    QPaintDevice* baseDevice;
};

QwtProxyPaintDevice::QwtProxyPaintDevice()
{
    d_data = new PrivateData();
}

QwtProxyPaintDevice::~QwtProxyPaintDevice()
{
    delete d_data;
}

void QwtProxyPaintDevice::setBaseDevice( QPaintDevice *device )
{
    d_data->baseDevice = device;
}
    
QPaintDevice *QwtProxyPaintDevice::baseDevice()
{
    return d_data->baseDevice;
}

const QPaintDevice *QwtProxyPaintDevice::baseDevice() const
{
    return d_data->baseDevice;
}

QPaintEngine *QwtProxyPaintDevice::paintEngine() const
{
    if ( d_data->baseDevice == NULL || d_data->baseDevice->paintEngine() == NULL )
        return NULL;

    if ( d_data->engine == NULL )
    {
        QwtProxyPaintDevice *that = const_cast<QwtProxyPaintDevice*>( this );
        d_data->engine = new QwtProxyPaintEngine( that );
    }

    return d_data->engine;
}

int QwtProxyPaintDevice::metric(PaintDeviceMetric metric) const
{
    if ( d_data->baseDevice )
    {
        // work around protected
        struct PD: QPaintDevice { using QPaintDevice::metric; };
        return (d_data->baseDevice->*&PD::metric)( metric );
    }

    return 0;
}
