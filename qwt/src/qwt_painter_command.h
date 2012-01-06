/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PAINTER_COMMAND_H
#define QWT_PAINTER_COMMAND_H

#include "qwt_global.h"
#include <qpaintengine.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpolygon.h>

class QPainterPath;

class QwtPainterCommand
{
public:
    enum Type
    {
        Invalid = -1,

        Path,
        Polygon,
        PolygonF,
        Pixmap,
        Image,
        State
    };

    struct PolygonData
    {
        QPolygon polygon;
        QPaintEngine::PolygonDrawMode mode;
    };

    struct PolygonFData
    {
        QPolygonF polygonF;
        QPaintEngine::PolygonDrawMode mode;
    };

    struct PixmapData
    {
        QRectF rect;
        QPixmap pixmap;
        QRectF subRect;
    };

    struct ImageData
    {
        QRectF rect;
        QImage image;
        QRectF subRect;
        Qt::ImageConversionFlags flags;
    };

    struct StateData
    {
        QPaintEngine::DirtyFlags flags;

        QPen pen;
        QBrush brush;
        QPointF brushOrigin;
        QBrush backgroundBrush;
        Qt::BGMode backgroundMode;
        QFont font;
        QMatrix matrix;
        QTransform transform;

        Qt::ClipOperation clipOperation;
        QRegion clipRegion;
        QPainterPath clipPath;
        bool isClipEnabled;

        QPainter::RenderHints renderHints;
        QPainter::CompositionMode compositionMode;
        qreal opacity;
    };

    QwtPainterCommand();
    QwtPainterCommand(const QwtPainterCommand &);

    QwtPainterCommand( const QPainterPath & );
    QwtPainterCommand( const QPolygon &, QPaintEngine::PolygonDrawMode );

    QwtPainterCommand( const QPolygonF &, QPaintEngine::PolygonDrawMode );
    QwtPainterCommand( const QRectF &rect,
            const QPixmap &, const QRectF& subRect );
    QwtPainterCommand( const QRectF &rect,
            const QImage &, const QRectF& subRect,
            Qt::ImageConversionFlags );
    QwtPainterCommand( const QPaintEngineState & );

    ~QwtPainterCommand();

    QwtPainterCommand &operator=(const QwtPainterCommand & );

    Type type() const;

    QPainterPath *path();
    const QPainterPath *path() const;

    PolygonData* polygonData();
    const PolygonData* polygonData() const;

    PolygonFData* polygonFData();
    const PolygonFData* polygonFData() const;

    PixmapData* pixmapData();
    const PixmapData* pixmapData() const;

    ImageData* imageData();
    const ImageData* imageData() const;

    StateData* stateData();
    const StateData* stateData() const;

    void render( QPainter *painter ) const;

private:
    void copy( const QwtPainterCommand & );
    void reset();

    Type d_type;

    union
    {
        QPainterPath *d_path;
        PolygonData *d_polygonData;
        PolygonFData *d_polygonFData;
        PixmapData *d_pixmapData;
        ImageData *d_imageData;
        StateData *d_stateData;
    };
};

#endif
