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
#include <qimage.h>
#include <qpixmap.h>

class QwtPainterCommand;

class QWT_EXPORT QwtVectorGraphic: public QwtNullPaintDevice
{
public:
	enum RenderHint
	{
		RenderPensUnscaled = 0x1
	};

    //! Render hints
    typedef QFlags<RenderHint> RenderHints;

    QwtVectorGraphic();
    QwtVectorGraphic( const QwtVectorGraphic & );

    virtual ~QwtVectorGraphic();

    QwtVectorGraphic& operator=(const QwtVectorGraphic &p);

    void reset();

    bool isNull() const;
    bool isEmpty() const;

    void render( QPainter * ) const;
    void render( QPainter *, const QSizeF &, 
            Qt::AspectRatioMode = Qt::IgnoreAspectRatio  ) const;
    void render( QPainter *, const QRectF &, 
            Qt::AspectRatioMode = Qt::IgnoreAspectRatio  ) const;
    void render( QPainter *, const QPointF &,
        Qt::Alignment = Qt::AlignTop | Qt::AlignLeft ) const;

    QPixmap toPixmap() const; 
    QPixmap toPixmap( const QSize &, 
        Qt::AspectRatioMode = Qt::IgnoreAspectRatio  ) const;

    QImage toImage() const; 
    QImage toImage( const QSize &, 
        Qt::AspectRatioMode = Qt::IgnoreAspectRatio  ) const;

    QRectF boundingRect() const;
    QRectF pointRect() const;

    const QVector< QwtPainterCommand > &commands() const;
    void setCommands( QVector< QwtPainterCommand > & );

    void setDefaultSize( const QSizeF & );
    QSizeF defaultSize() const;
    
    void setRenderHint( RenderHint, bool on = true );
    bool testRenderHint( RenderHint ) const;

protected:
    virtual QSize sizeMetrics() const;

    virtual void drawPath(const QPainterPath &);

    virtual void drawPixmap(const QRectF &,
        const QPixmap &, const QRectF &);

    virtual void drawImage(const QRectF &,
        const QImage &, const QRectF &, Qt::ImageConversionFlags );

    virtual void updateState( const QPaintEngineState &state );

private:
    void updateBoundingRect( const QRectF & );
    void updatePointRect( const QRectF & );

    class PathInfo;

    class PrivateData;
    PrivateData *d_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtVectorGraphic::RenderHints )
Q_DECLARE_METATYPE( QwtVectorGraphic )

#endif
