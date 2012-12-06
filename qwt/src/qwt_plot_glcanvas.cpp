/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_glcanvas.h"
#include "qwt_plot.h"
#include <qevent.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include "qwt_painter.h"

static void qwtUpdateContentsRect( QwtPlotGLCanvas *canvas )
{
    const int fw = canvas->frameWidth();
    canvas->setContentsMargins( fw, fw, fw, fw );
}

class QwtPlotGLCanvas::PrivateData
{
public:
    PrivateData():
        frameStyle( QFrame::Panel | QFrame::Sunken),
        lineWidth( 2 ),
        midLineWidth( 0 ),
        borderRadius( 0 )
    {
    }

    int frameStyle;
    int lineWidth;
    int midLineWidth;
    double borderRadius;
};

/*! 
  \brief Constructor

  \param plot Parent plot widget
  \sa QwtPlot::setCanvas()
*/
QwtPlotGLCanvas::QwtPlotGLCanvas( QwtPlot *plot ):
    QGLWidget( plot )
{
    d_data = new PrivateData;

#ifndef QT_NO_CURSOR
    setCursor( Qt::CrossCursor );
#endif

    setAutoFillBackground( true );
    qwtUpdateContentsRect( this );
}

//! Destructor
QwtPlotGLCanvas::~QwtPlotGLCanvas()
{
    delete d_data;
}

void QwtPlotGLCanvas::setFrameStyle( int style )
{
    if ( style != d_data->frameStyle )
    {
        d_data->frameStyle = style;
        qwtUpdateContentsRect( this );

        update();
    }
}

int QwtPlotGLCanvas::frameStyle() const
{
    return d_data->frameStyle;
}

void QwtPlotGLCanvas::setFrameShadow( Shadow shadow )
{
    setFrameStyle(( d_data->frameStyle & QFrame::Shape_Mask ) | shadow );
}

QwtPlotGLCanvas::Shadow QwtPlotGLCanvas::frameShadow() const
{
    return (Shadow) ( d_data->frameStyle & QFrame::Shadow_Mask );
}

void QwtPlotGLCanvas::setFrameShape( Shape shape )
{
    setFrameStyle( ( d_data->frameStyle & QFrame::Shadow_Mask ) | shape );
}

QwtPlotGLCanvas::Shape QwtPlotGLCanvas::frameShape() const
{
    return (Shape) ( d_data->frameStyle & QFrame::Shape_Mask );
}

void QwtPlotGLCanvas::setLineWidth( int width )
{
    width = qMax( width, 0 );
    if ( width != d_data->lineWidth )
    {
        d_data->lineWidth = qMax( width, 0 );
        qwtUpdateContentsRect( this );
        update();
    }
}

int QwtPlotGLCanvas::lineWidth() const
{
    return d_data->lineWidth;
}

void QwtPlotGLCanvas::setMidLineWidth( int width )
{
    width = qMax( width, 0 );
    if ( width != d_data->midLineWidth )
    {
        d_data->midLineWidth = width;
        qwtUpdateContentsRect( this );
        update();
    }
}

int QwtPlotGLCanvas::midLineWidth() const
{
    return d_data->midLineWidth;
}

int QwtPlotGLCanvas::frameWidth() const
{
    return ( frameStyle() != NoFrame ) ? d_data->lineWidth : 0;
}

void QwtPlotGLCanvas::setBorderRadius( double radius )
{
    radius = qMax( 0.0, radius );
    if ( radius != d_data->borderRadius )
    {
        d_data->borderRadius = qMax( 0.0, radius );
        update();
    }
}

double QwtPlotGLCanvas::borderRadius() const
{
    return d_data->borderRadius;
}

/*!
  Paint event

  \param event Paint event
  \sa QwtPlot::drawCanvas()
*/
void QwtPlotGLCanvas::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    QwtPlot *plot = qobject_cast< QwtPlot *>( parent() );
    if ( plot )
        plot->drawCanvas( &painter );

    if ( !testAttribute(Qt::WA_StyledBackground ) )
    {
        if ( frameWidth() > 0 )
            drawBorder( &painter );
    }
}

void QwtPlotGLCanvas::drawBorder( QPainter *painter )
{
    const int fw = frameWidth();

    if ( fw <= 0 )
        return;

    const QRect frameRect = contentsRect().adjusted( -fw, -( fw + 0), fw + 0, fw );

    if ( d_data->borderRadius > 0 )
    {
        QwtPainter::drawRoundedFrame( painter, frameRect,
            d_data->borderRadius, d_data->borderRadius,
            palette(), fw, frameStyle() );
    }
    else
    {
        if ( frameShadow() == Plain )
        {
            qDrawPlainRect( painter, frameRect, 
                palette().shadow().color(), lineWidth() );
        }
        else
        {
            if ( frameShape() == Box )
            {
                qDrawShadeRect( painter, frameRect, palette(),
                    frameShadow() == Sunken, lineWidth(), midLineWidth() );
            }
            else
            {
                qDrawShadePanel( painter, frameRect, palette(), 
                    frameShadow() == Sunken, lineWidth() );
            }
        }
    }
}

//! Calls repaint()
void QwtPlotGLCanvas::replot()
{
    repaint( contentsRect() );
}

/*!
   \return Empty path
*/
QPainterPath QwtPlotGLCanvas::borderPath( const QRect &rect ) const
{
    Q_UNUSED( rect );
    return QPainterPath();
}
