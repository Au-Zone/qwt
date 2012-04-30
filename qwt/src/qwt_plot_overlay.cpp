/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_overlay.h"
#include <qpainter.h>
#include <qpaintengine.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qevent.h>

#define USE_IMAGE 1
static const QImage::Format imageFormat = QImage::Format_ARGB32_Premultiplied;


static QRegion qwtAlphaMask( 
    const QImage& image, const QVector<QRect> rects )
{
    const int w = image.width();
    const int h = image.height();

    QRegion region;
    QRect rect;

    for ( int i = 0; i < rects.size(); i++ )
    {
        int x1, x2, y1, y2;
        rects[i].getCoords( &x1, &y1, &x2, &y2 );

        x1 = qMax( x1, 0 );
        x2 = qMin( x2, w - 1 );
        y1 = qMax( y1, 0 );
        y2 = qMin( y2, h - 1 );

        for ( int y = y1; y <= y2; ++y ) 
        {
            bool inRect = false;
            int rx0 = -1;

            const uint *line = 
                reinterpret_cast<const uint *> ( image.scanLine( y ) ) + x1;
            for ( int x = x1; x <= x2; x++ ) 
            {
                const bool on = ( *line++ != 0 );
                if ( on != inRect ) 
                {
                    if ( inRect  ) 
                    {
                        rect.setCoords( rx0, y, x - 1, y );
                        region += rect;
                    } 
                    else 
                    {
                        rx0 = x;
                    }

                    inRect = on;
                } 
            }

            if ( inRect ) 
            {
                rect.setCoords( rx0, y, x2, y );
                region = region.united( rect );
            }
        }
    }

    return region;
}

class QwtPlotOverlay::PrivateData
{
public:
    PrivateData():
		maskMode( QwtPlotOverlay::AlphaMask ),
		renderMode( QwtPlotOverlay::AutoRenderMode ),
		rgbaBuffer( NULL )
    {
    }

    ~PrivateData()
    {
    	if ( rgbaBuffer )
        	::free( rgbaBuffer );
    }

	MaskMode maskMode;
	RenderMode renderMode;
	uchar *rgbaBuffer;
};

QwtPlotOverlay::QwtPlotOverlay( QWidget* parent ):
    QWidget( parent ),
    m_rgbaBuffer( NULL )
{
	d_data = new PrivateData;

    setAttribute( Qt::WA_TransparentForMouseEvents );
    setAttribute( Qt::WA_NoSystemBackground );
    setFocusPolicy( Qt::NoFocus );

	if ( parent )
	{
		resize( parent->size() );
		parent->installEventFilter( this );
	}
}

QwtPlotOverlay::~QwtPlotOverlay()
{
	delete d_data;
}

void QwtPlotOverlay::updateOverlay()
{
	updateMask();
	update();
}

void QwtPlotOverlay::updateMask()
{
	if ( d_data->rgbaBuffer )
	{
		::free( d_data->rgbaBuffer );
		d_data->rgbaBuffer = NULL;
	}

	QRegion mask;

	if ( d_data->maskMode == QwtPlotOverlay::Mask )
	{
		mask = maskHint();
	}
	else if ( d_data->maskMode == QwtPlotOverlay::AlphaMask )
	{
    	// A fresh buffer from calloc() is usually faster
    	// than reinitializing an existing one with
    	// QImage::fill( 0 ) or memset()

    	d_data->rgbaBuffer = ( uchar* )::calloc( width() * height(), 4 );

    	QImage image( d_data->rgbaBuffer, width(), height(), imageFormat );

    	QPainter painter( &image );
    	draw( &painter );
		painter.end();

		QRegion hint = maskHint();
		if ( hint.isEmpty() )
			hint += QRect( 0, 0, width(), height() );

		mask = qwtAlphaMask( image, hint.rects() );

	}

	// A bug in Qt initiates a full repaint of the canvas
	// when we change the mask, while we are visible !

	setVisible( false );

	if ( mask.isEmpty() )
		clearMask();
	else
		setMask( mask );

	setVisible( true );
}

void QwtPlotOverlay::paintEvent( QPaintEvent* event )
{
    const QRegion clipRegion = event->region();

    QPainter painter( this );

    bool useRgbaBuffer = false;
	if ( d_data->renderMode == QwtPlotOverlay::CopyAlphaMask )
	{
		useRgbaBuffer = true;
	}
	else if ( d_data->renderMode == QwtPlotOverlay::AutoRenderMode )
	{
		if ( painter.paintEngine()->type() == QPaintEngine::Raster )
			useRgbaBuffer = true;
	}

    if ( d_data->rgbaBuffer && useRgbaBuffer )
    {
        const QImage image( d_data->rgbaBuffer, 
			width(), height(), imageFormat );

        QVector<QRect> rects;
        if ( clipRegion.rects().size() > 20 )
        {
			// the region is to complex
            painter.setClipRegion( clipRegion );
            rects += clipRegion.boundingRect();
        }
        else
        {
            rects = clipRegion.rects();
        }

        for ( int i = 0; i < rects.size(); i++ )
        {
            const QRect r = rects[i];
            painter.drawImage( r.topLeft(), image, r );
        }
    }
    else
    {
        painter.setClipRegion( clipRegion );
        draw( &painter );
    }
}

void QwtPlotOverlay::resizeEvent( QResizeEvent* )
{
    if ( m_rgbaBuffer )
    {
        ::free( m_rgbaBuffer );
        m_rgbaBuffer = 0;
    }
}

void QwtPlotOverlay::draw( QPainter *painter ) const
{
	QWidget *canvas = const_cast< QWidget *>( parentWidget() );
    if ( canvas )
    {
        painter->setClipRect( parentWidget()->contentsRect() );

        QPainterPath clipPath;

        ( void )QMetaObject::invokeMethod(
            canvas, "borderPath", Qt::DirectConnection,
            Q_RETURN_ARG( QPainterPath, clipPath ), Q_ARG( QRect, rect() ) );

        if (!clipPath.isEmpty())
        {
            painter->setClipPath( clipPath, Qt::IntersectClip );
        }
    }

    drawOverlay( painter );
}

QRegion QwtPlotOverlay::maskHint() const
{
    return QRegion();
}

bool QwtPlotOverlay::eventFilter( QObject* object, QEvent* event )
{
	if ( object == parent() && event->type() == QEvent::Resize )
	{
		QResizeEvent *resizeEvent = static_cast<QResizeEvent *>( event );
		resize( resizeEvent->size() );
	}

	return QObject::eventFilter( object, event );
}
