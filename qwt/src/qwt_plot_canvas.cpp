/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_canvas.h"
#include "qwt_painter.h"
#include "qwt_math.h"
#include "qwt_plot.h"
#include <qpainter.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qpaintengine.h>
#ifdef Q_WS_X11
#include <qx11info_x11.h>
#endif
#include <qevent.h>

class QwtPlotCanvas::PrivateData
{
public:
    PrivateData():
        focusIndicator( NoFocusIndicator ),
        paintAttributes( 0 )
    {
        cache.isValid = false;
    }

    FocusIndicator focusIndicator;
    int paintAttributes;

    struct
    {
        QPixmap pixmap;
        bool isValid;

    } cache;
};

//! Sets a cross cursor, enables QwtPlotCanvas::PaintCached

QwtPlotCanvas::QwtPlotCanvas( QwtPlot *plot ):
    QFrame( plot )
{
    d_data = new PrivateData;

    setAutoFillBackground( true );

#ifndef QT_NO_CURSOR
    setCursor( Qt::CrossCursor );
#endif

    setPaintAttribute( PaintCached, true );
    setPaintAttribute( PaintPacked, true );
}

//! Destructor
QwtPlotCanvas::~QwtPlotCanvas()
{
    delete d_data;
}

//! Return parent plot widget
QwtPlot *QwtPlotCanvas::plot()
{
    return qobject_cast<QwtPlot *>( parentWidget() );
}

//! Return parent plot widget
const QwtPlot *QwtPlotCanvas::plot() const
{
    return qobject_cast<const QwtPlot *>( parentWidget() );
}

/*!
  \brief Changing the paint attributes

  \param attribute Paint attribute
  \param on On/Off

  The default setting enables PaintCached and PaintPacked

  \sa testPaintAttribute(), drawCanvas(), drawContents(), paintCache()
*/
void QwtPlotCanvas::setPaintAttribute( PaintAttribute attribute, bool on )
{
    if ( bool( d_data->paintAttributes & attribute ) == on )
        return;

    if ( on )
        d_data->paintAttributes |= attribute;
    else
        d_data->paintAttributes &= ~attribute;

    switch ( attribute )
    {
        case PaintCached:
        {
            if ( on )
            {
                if ( isVisible() )
                {
                    const QRect cr = contentsRect();
                    d_data->cache.pixmap = QPixmap::grabWidget( this,
                        cr.x(), cr.y(), cr.width(), cr.height() );
                    d_data->cache.isValid = true;
                }
            }
            else
            {
                d_data->cache.isValid = false;
            }
            break;
        }
        case PaintPacked:
        {
            /*
              If not visible, changing of the background mode
              is delayed until it becomes visible. This tries to avoid
              looking through the canvas when the canvas is shown the first
              time.
             */

            if ( on == false || isVisible() )
                QwtPlotCanvas::setSystemBackground( !on );

            break;
        }
    }
}

/*!
  Test wether a paint attribute is enabled

  \param attribute Paint attribute
  \return true if the attribute is enabled
  \sa setPaintAttribute()
*/
bool QwtPlotCanvas::testPaintAttribute( PaintAttribute attribute ) const
{
    return ( d_data->paintAttributes & attribute ) != 0;
}

//! Return the paint cache, might be null
QPixmap *QwtPlotCanvas::paintCache()
{
    if ( !( d_data->paintAttributes & PaintCached ) )
        return NULL;

    return &d_data->cache.pixmap;
}

//! Return the paint cache, might be null
const QPixmap *QwtPlotCanvas::paintCache() const
{
    if ( !( d_data->paintAttributes & PaintCached ) )
        return NULL;

    return &d_data->cache.pixmap;
}

//! Invalidate the internal paint cache
void QwtPlotCanvas::invalidatePaintCache()
{
    d_data->cache.isValid = false;
}

/*!
  Set the focus indicator

  \sa FocusIndicator, focusIndicator()
*/
void QwtPlotCanvas::setFocusIndicator( FocusIndicator focusIndicator )
{
    d_data->focusIndicator = focusIndicator;
}

/*!
  \return Focus indicator

  \sa FocusIndicator, setFocusIndicator()
*/
QwtPlotCanvas::FocusIndicator QwtPlotCanvas::focusIndicator() const
{
    return d_data->focusIndicator;
}

/*!
  Hide event
  \param event Hide event
*/
void QwtPlotCanvas::hideEvent( QHideEvent *event )
{
    QFrame::hideEvent( event );

    if ( d_data->paintAttributes & PaintPacked )
    {
        // enable system background to avoid the "looking through
        // the canvas" effect, for the next show

        setSystemBackground( true );
    }
}

/*!
  Paint event
  \param event Paint event
*/
void QwtPlotCanvas::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );

    if ( !contentsRect().contains( event->rect() ) )
    {
        painter.save();
        painter.setClipRegion( event->region() & frameRect() );
        drawFrame( &painter );
        painter.restore();
    }

    painter.setClipRegion( event->region() & contentsRect() );

    drawContents( &painter );

    if ( d_data->paintAttributes & PaintPacked )
        setSystemBackground( false );
}

/*!
  Redraw the canvas, and focus rect
  \param painter Painter
*/
void QwtPlotCanvas::drawContents( QPainter *painter )
{
    if ( d_data->paintAttributes & PaintCached && d_data->cache.isValid
        && d_data->cache.pixmap.size() == contentsRect().size() )
    {
        painter->drawPixmap( contentsRect().topLeft(), d_data->cache.pixmap );
    }
    else
    {
        QwtPlot *plot = ( ( QwtPlot * )parent() );
        const bool doAutoReplot = plot->autoReplot();
        plot->setAutoReplot( false );

        drawCanvas( painter );

        plot->setAutoReplot( doAutoReplot );
    }

    if ( hasFocus() && focusIndicator() == CanvasFocusIndicator )
        drawFocusIndicator( painter );
}

/*!
  Draw the the canvas

  Paints all plot items to the contentsRect(), using QwtPlot::drawCanvas
  and updates the paint cache.

  \param painter Painter

  \sa QwtPlot::drawCanvas(), setPaintAttributes(), testPaintAttributes()
*/
void QwtPlotCanvas::drawCanvas( QPainter *painter )
{
    if ( !contentsRect().isValid() )
        return;

    QBrush bgBrush = palette().brush( backgroundRole() );

    if ( d_data->paintAttributes & PaintCached )
    {
        if ( d_data->cache.pixmap.size() != contentsRect().size() )
        {
            d_data->cache.pixmap = QPixmap( contentsRect().size() );

#ifdef Q_WS_X11
            if ( d_data->cache.pixmap.x11Info().screen() != x11Info().screen() )
                d_data->cache.pixmap.x11SetScreen( x11Info().screen() );
#endif
        }

        if ( testAttribute(Qt::WA_StyledBackground) ) 
        {
            QPainter bgPainter( &d_data->cache.pixmap );
            bgPainter.translate( -contentsRect().topLeft() );
            
            QStyleOption opt;
            opt.initFrom(this);
            style()->drawPrimitive(QStyle::PE_Widget, &opt, &bgPainter, this);
            bgPainter.end();
        }
        else
        {
            if ( d_data->paintAttributes & PaintPacked )
            {
                QPainter bgPainter( &d_data->cache.pixmap );
                bgPainter.setPen( Qt::NoPen );

                bgPainter.setBrush( bgBrush );
                bgPainter.drawRect( d_data->cache.pixmap.rect() );
            }
            else
            {
                d_data->cache.pixmap.fill( this, 
                    d_data->cache.pixmap.rect().topLeft() );
            }
        }

        QPainter cachePainter( &d_data->cache.pixmap );
        cachePainter.translate( -contentsRect().x(),
            -contentsRect().y() );

        ( ( QwtPlot * )parent() )->drawCanvas( &cachePainter );

        cachePainter.end();

        d_data->cache.isValid = true;
        painter->drawPixmap( contentsRect(), d_data->cache.pixmap );
    }
    else
    {
        if ( d_data->paintAttributes & PaintPacked )
        {
            painter->save();

            if ( testAttribute(Qt::WA_StyledBackground) )
            {
                QStyleOption opt;
                opt.initFrom(this);
                style()->drawPrimitive(QStyle::PE_Widget, &opt, painter, this);
            }
            else
            {
                painter->setPen( Qt::NoPen );
                painter->setBrush( bgBrush );
                painter->drawRect( contentsRect() );
            }

            painter->restore();
        }

        ( ( QwtPlot * )parent() )->drawCanvas( painter );
    }
}

/*!
  Draw the focus indication
  \param painter Painter
*/
void QwtPlotCanvas::drawFocusIndicator( QPainter *painter )
{
    const int margin = 1;

    QRect focusRect = contentsRect();
    focusRect.setRect( focusRect.x() + margin, focusRect.y() + margin,
        focusRect.width() - 2 * margin, focusRect.height() - 2 * margin );

    QwtPainter::drawFocusRect( painter, this, focusRect );
}

void QwtPlotCanvas::setSystemBackground( bool on )
{
    if ( testAttribute( Qt::WA_NoSystemBackground ) == on )
        setAttribute( Qt::WA_NoSystemBackground, !on );
}

/*!
   Invalidate the paint cache and repaint the canvas
   \sa invalidatePaintCache()
*/
void QwtPlotCanvas::replot()
{
    invalidatePaintCache();

    /*
      In case of cached or packed painting the canvas
      is repainted completely and doesn't need to be erased.
     */
    const bool erase =
        !testPaintAttribute( QwtPlotCanvas::PaintPacked )
        && !testPaintAttribute( QwtPlotCanvas::PaintCached );

    const bool noBackgroundMode = testAttribute( Qt::WA_NoBackground );
    if ( !erase && !noBackgroundMode )
        setAttribute( Qt::WA_NoBackground, true );

    repaint( contentsRect() );

    if ( !erase && !noBackgroundMode )
        setAttribute( Qt::WA_NoBackground, false );
}
