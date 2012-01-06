/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_VECTOR_GRAPHIC_H
#define QWT_VECTOR_GRAPHIC_H

#include "qwt_global.h"
#include "qwt_null_paintdevice.h"
#include <qmetatype.h>

class QwtPainterCommand;

class QWT_EXPORT QwtVectorGraphic: public QwtNullPaintDevice
{
public:
    QwtVectorGraphic();
    QwtVectorGraphic( const QwtVectorGraphic & );

    virtual ~QwtVectorGraphic();

    QwtVectorGraphic& operator=(const QwtVectorGraphic &p);

    void reset();
    bool isNull() const;

    void render( QPainter * ) const;

    QRectF boundingRect() const;
    QRectF pointRect() const;

    const QVector< QwtPainterCommand > &commands() const;
    void setCommands( QVector< QwtPainterCommand > & );

protected:
    virtual QSize sizeMetrics() const;

    virtual void drawPath(const QPainterPath &);

    virtual void drawPolygon(
        const QPointF *, int , QPaintEngine::PolygonDrawMode );

    virtual void drawPolygon(
        const QPoint *, int , QPaintEngine::PolygonDrawMode );

    virtual void drawPixmap(const QRectF &,
        const QPixmap &, const QRectF &);

    virtual void drawImage(const QRectF &,
        const QImage &, const QRectF &, Qt::ImageConversionFlags );

    virtual void updateState( const QPaintEngineState &state );

private:
    void updateRects( const QRectF &, bool addPen );

    class PrivateData;
    PrivateData *d_data;
};

Q_DECLARE_METATYPE( QwtVectorGraphic )

#endif
