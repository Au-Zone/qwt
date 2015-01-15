/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_abstract_canvas.h"
#include "qwt_plot.h"
#include <qpainter.h>
#include <qdrawutil.h>
#include <qstyle.h>
#include <qstyleoption.h>

static QWidget *qwtBGWidget( QWidget *widget )
{
    QWidget *w = widget;

    for ( ; w->parentWidget() != NULL; w = w->parentWidget() )
    {
        if ( w->autoFillBackground() || 
            w->testAttribute( Qt::WA_StyledBackground ) )
        {
            return w;
        }
    }

    return w;
}

static void qwtUpdateContentsRect( int fw, QWidget *canvas )
{
    canvas->setContentsMargins( fw, fw, fw, fw );
}

class QwtPlotAbstractCanvas::PrivateData
{
public:
    PrivateData():
        frameStyle( QFrame::Panel | QFrame::Sunken),
        lineWidth( 2 ),
        midLineWidth( 0 )
    {
    }

    ~PrivateData()
    {
    }

    QwtPlotAbstractCanvas::PaintAttributes paintAttributes;

    int frameStyle;
    int lineWidth;
    int midLineWidth;

    QWidget *canvasWidget;
};

QwtPlotAbstractCanvas::QwtPlotAbstractCanvas( QWidget *canvasWidget )
{
    d_data = new PrivateData;
    d_data->canvasWidget = canvasWidget;

    init();
}

QwtPlotAbstractCanvas::~QwtPlotAbstractCanvas()
{
    delete d_data;
}

void QwtPlotAbstractCanvas::init()
{
#ifndef QT_NO_CURSOR
    d_data->canvasWidget->setCursor( Qt::CrossCursor );
#endif

    d_data->canvasWidget->setAutoFillBackground( true );
    qwtUpdateContentsRect( frameWidth(), d_data->canvasWidget );

    d_data->paintAttributes = QwtPlotAbstractCanvas::BackingStore;
}

/*!
  \brief Changing the paint attributes

  \param attribute Paint attribute
  \param on On/Off

  \sa testPaintAttribute()
*/
void QwtPlotAbstractCanvas::setPaintAttribute( PaintAttribute attribute, bool on )
{   
    if ( bool( d_data->paintAttributes & attribute ) == on )
        return;
    
    if ( on )
        d_data->paintAttributes |= attribute;
    else
        d_data->paintAttributes &= ~attribute;

    if ( attribute == BackingStore )
        invalidateBackingStore();
}

/*!
  Test whether a paint attribute is enabled

  \param attribute Paint attribute
  \return true, when attribute is enabled
  \sa setPaintAttribute()
*/  
bool QwtPlotAbstractCanvas::testPaintAttribute( PaintAttribute attribute ) const
{   
    return d_data->paintAttributes & attribute;
}

/*!
  Set the frame style

  \param style The bitwise OR between a shape and a shadow. 
  
  \sa frameStyle(), QFrame::setFrameStyle(), 
      setFrameShadow(), setFrameShape()
 */
void QwtPlotAbstractCanvas::setFrameStyle( int style )
{
    if ( style != d_data->frameStyle )
    {
        d_data->frameStyle = style;
        qwtUpdateContentsRect( frameWidth(), d_data->canvasWidget );

        d_data->canvasWidget->update();
    }
}

/*!
  \return The bitwise OR between a frameShape() and a frameShadow()
  \sa setFrameStyle(), QFrame::frameStyle()
 */
int QwtPlotAbstractCanvas::frameStyle() const
{
    return d_data->frameStyle;
}

/*!
  Set the frame shadow

  \param shadow Frame shadow
  \sa frameShadow(), setFrameShape(), QFrame::setFrameShadow()
 */
void QwtPlotAbstractCanvas::setFrameShadow( Shadow shadow )
{
    setFrameStyle(( d_data->frameStyle & QFrame::Shape_Mask ) | shadow );
}

/*!
  \return Frame shadow
  \sa setFrameShadow(), QFrame::setFrameShadow()
 */
QwtPlotAbstractCanvas::Shadow QwtPlotAbstractCanvas::frameShadow() const
{
    return (Shadow) ( d_data->frameStyle & QFrame::Shadow_Mask );
}

/*!
  Set the frame shape

  \param shape Frame shape
  \sa frameShape(), setFrameShadow(), QFrame::frameShape()
 */
void QwtPlotAbstractCanvas::setFrameShape( Shape shape )
{
    setFrameStyle( ( d_data->frameStyle & QFrame::Shadow_Mask ) | shape );
}

/*!
  \return Frame shape
  \sa setFrameShape(), QFrame::frameShape()
 */
QwtPlotAbstractCanvas::Shape QwtPlotAbstractCanvas::frameShape() const
{
    return (Shape) ( d_data->frameStyle & QFrame::Shape_Mask );
}

/*!
   Set the frame line width

   The default line width is 2 pixels.

   \param width Line width of the frame
   \sa lineWidth(), setMidLineWidth()
*/
void QwtPlotAbstractCanvas::setLineWidth( int width )
{
    width = qMax( width, 0 );
    if ( width != d_data->lineWidth )
    {
        d_data->lineWidth = qMax( width, 0 );
        qwtUpdateContentsRect( frameWidth(), d_data->canvasWidget );
        d_data->canvasWidget->update();
    }
}

/*!
  \return Line width of the frame
  \sa setLineWidth(), midLineWidth()
 */
int QwtPlotAbstractCanvas::lineWidth() const
{
    return d_data->lineWidth;
}

/*!
   Set the frame mid line width

   The default midline width is 0 pixels.

   \param width Midline width of the frame
   \sa midLineWidth(), setLineWidth()
*/
void QwtPlotAbstractCanvas::setMidLineWidth( int width )
{
    width = qMax( width, 0 );
    if ( width != d_data->midLineWidth )
    {
        d_data->midLineWidth = width;
        qwtUpdateContentsRect( frameWidth(), d_data->canvasWidget );
        d_data->canvasWidget->update();
    }
}

/*!
  \return Midline width of the frame
  \sa setMidLineWidth(), lineWidth()
 */ 
int QwtPlotAbstractCanvas::midLineWidth() const
{
    return d_data->midLineWidth;
}

/*!
  \return Frame width depending on the style, line width and midline width.
 */
int QwtPlotAbstractCanvas::frameWidth() const
{
    return ( frameStyle() != NoFrame ) ? d_data->lineWidth : 0;
}

void QwtPlotAbstractCanvas::replot()
{
    invalidateBackingStore();
    
    QWidget *w = d_data->canvasWidget;
    if ( testPaintAttribute( QwtPlotAbstractCanvas::ImmediatePaint ) )
        w->repaint( w->contentsRect() );
    else
        w->update( w->contentsRect() );
}

/*!
  Draw the plot items
  \param painter Painter

  \sa QwtPlot::drawCanvas()
*/  
void QwtPlotAbstractCanvas::drawItems( QPainter *painter )
{
    QwtPlot *plot = qobject_cast< QwtPlot *>( d_data->canvasWidget->parent() );
    if ( plot )
    {
        painter->save();
        painter->setClipRect( d_data->canvasWidget->contentsRect(), Qt::IntersectClip );

        plot->drawCanvas( painter );

        painter->restore();
    }
}

/*!
  Draw the background of the canvas
  \param painter Painter
*/ 
void QwtPlotAbstractCanvas::drawBackground( QPainter *painter )
{
    painter->save();

    QWidget *w = qwtBGWidget( d_data->canvasWidget );

    const QPoint off = d_data->canvasWidget->mapTo( w, QPoint() );
    painter->translate( -off );

    const QRect fillRect = d_data->canvasWidget->rect().translated( off );

    if ( w->testAttribute( Qt::WA_StyledBackground ) )
    {
        painter->setClipRect( fillRect );

        QStyleOption opt;
        opt.initFrom( w );
        w->style()->drawPrimitive( QStyle::PE_Widget, &opt, painter, w);
    }
    else 
    {
#if 0
        if ( !autoFillBackground() )
#endif
        {
            painter->fillRect( fillRect,
                w->palette().brush( w->backgroundRole() ) );
        }
    }

    painter->restore();
}

/*!
  Draw the border of the canvas
  \param painter Painter
*/
void QwtPlotAbstractCanvas::drawBorder( QPainter *painter )
{
    const int fw = frameWidth();
    if ( fw <= 0 )
        return;

    if ( frameShadow() == QwtPlotAbstractCanvas::Plain )
    {
        qDrawPlainRect( painter, frameRect(), 
            d_data->canvasWidget->palette().shadow().color(), lineWidth() );
    }
    else
    {
        if ( frameShape() == QwtPlotAbstractCanvas::Box )
        {
            qDrawShadeRect( painter, frameRect(), d_data->canvasWidget->palette(),
                frameShadow() == Sunken, lineWidth(), midLineWidth() );
        }
        else
        {
            qDrawShadePanel( painter, frameRect(), d_data->canvasWidget->palette(), 
                frameShadow() == Sunken, lineWidth() );
        }
    }
}

/*!
   \return Empty path
*/
QPainterPath QwtPlotAbstractCanvas::borderPath( const QRect &rect ) const
{
    Q_UNUSED( rect );
    return QPainterPath();
}

//! \return The rectangle where the frame is drawn in.
QRect QwtPlotAbstractCanvas::frameRect() const
{
    const int fw = frameWidth();
    return d_data->canvasWidget->contentsRect().adjusted( -fw, -fw, fw, fw );
}

void QwtPlotAbstractCanvas::draw( QPainter *painter )
{
#if FIX_GL_TRANSLATION
    if ( painter->paintEngine()->type() == QPaintEngine::OpenGL2 )
    {
        // work around a translation bug of QPaintEngine::OpenGL2
        painter->translate( 1, 1 );
    }
#endif

    drawBackground( painter );
    drawItems( painter );

    if ( !d_data->canvasWidget->testAttribute( Qt::WA_StyledBackground ) )
    { 
        if ( frameWidth() > 0 )
            drawBorder( painter );
    }
}
