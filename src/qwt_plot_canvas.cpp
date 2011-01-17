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
#include "qwt_null_paintdevice.h"
#include "qwt_math.h"
#include "qwt_plot.h"
#include <qpainter.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qpaintengine.h>
#include <qevent.h>
#ifdef Q_WS_X11
#include <qx11info_x11.h>
#endif

#define DEBUG_BACKGROUND 0
#if DEBUG_BACKGROUND
#include <qdatetime.h>
#endif

class QwtClipLogger: public QwtNullPaintDevice
{
public:
    QwtClipLogger( const QSize &size ):
        QwtNullPaintDevice( QPaintEngine::AllFeatures )
    {
        setSize( size );
        d_pathCount = 0;
    }

    virtual void drawPath( const QPainterPath &path )
    {
        if ( d_pathCount == 0 )
        {
            setCornerRects( path );

            const QRectF rect( QPointF( 0.0, 0.0 ) , size() );

            for ( int i = 0; i < clipRects.size(); i++ )
            {
                QRectF &r = clipRects[i];
                if ( r.center().x() < rect.center().x() )
                    r.setLeft( rect.left() );
                else
                    r.setRight( rect.right() );

                if ( r.center().y() < rect.center().y() )
                    r.setTop( rect.top() );
                else
                    r.setBottom( rect.bottom() );
            }

        }

        d_pathCount++;
    }

    void setCornerRects( const QPainterPath &path )
    {
        QPointF pos( 0.0, 0.0 );

        for (int i = 0; i < path.elementCount(); i++ )
        {
            QPainterPath::Element el = path.elementAt(i); 
            switch( el.type )
            {
                case QPainterPath::MoveToElement:
                case QPainterPath::LineToElement:
                {
                    pos.setX( el.x );
                    pos.setY( el.y );
                    break;
                }
                case QPainterPath::CurveToElement:
                {
                    QRectF r( pos, QPointF( el.x, el.y ) );
                    clipRects += r.normalized();

                    pos.setX( el.x );
                    pos.setY( el.y );

                    break;
                }
                case QPainterPath::CurveToDataElement:
                {
                    if ( clipRects.size() > 0 )
                    {
                        QRectF r = clipRects.last();
                        r.setCoords( 
                            qMin( r.left(), el.x ),
                            qMin( r.top(), el.y ),
                            qMax( r.right(), el.x ),
                            qMax( r.bottom(), el.y )
                        );
                        clipRects.last() = r.normalized();
                    }
                    break;
                }
            }
        }
    }

public:
    QVector<QRectF> clipRects;

private:
    bool d_pathCount;
};

static inline void qwtDrawStyledBackground( 
    QWidget *w, QPainter *painter )
{
    QStyleOption opt;
    opt.initFrom(w);
    w->style()->drawPrimitive( QStyle::PE_Widget, &opt, painter, w);
}

static QWidget *qwtBackgroundWidget( QWidget *w )
{
    if ( w->parentWidget() == NULL )
        return w;

    if ( w->autoFillBackground() )
    {
        const QBrush brush = w->palette().brush( w->backgroundRole() );
        if ( brush.color().alpha() > 0 )
            return w;
    }

    if ( w->testAttribute( Qt::WA_StyledBackground ) )
    {
        QImage image( 1, 1, QImage::Format_ARGB32 );
        image.fill( Qt::transparent );

        QPainter painter( &image );
        painter.translate( -w->rect().center() );
        qwtDrawStyledBackground( w, &painter );
        painter.end();

        if ( qAlpha( image.pixel( 0, 0 ) ) != 0 )
            return w;
    }

    return qwtBackgroundWidget( w->parentWidget() );
}

class QwtPlotCanvas::PrivateData
{
public:
    PrivateData():
        focusIndicator( NoFocusIndicator ),
        paintAttributes( 0 ),
        cache( NULL )
    {
    }

    ~PrivateData()
    {
        delete cache;
    }

    FocusIndicator focusIndicator;
    int paintAttributes;
    QPixmap *cache;
};

//! Sets a cross cursor, enables QwtPlotCanvas::PaintCached

QwtPlotCanvas::QwtPlotCanvas( QwtPlot *plot ):
    QFrame( plot )
{
    d_data = new PrivateData;

    setAutoFillBackground( true );

    // Otherwise we have a lot of perfomance issues with
    // styled backgrounds
    setAttribute( Qt::WA_OpaquePaintEvent, true );

#ifndef QT_NO_CURSOR
    setCursor( Qt::CrossCursor );
#endif

    setPaintAttribute( PaintCached, true );
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

  The default setting enables PaintCached

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
                if ( d_data->cache == NULL )
                    d_data->cache = new QPixmap();

                if ( isVisible() )
                {
                    *d_data->cache = 
                        QPixmap::grabWidget( this, contentsRect() );
                }
            }
            else
            {
                delete d_data->cache;
                d_data->cache = NULL;
            }
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
    return d_data->cache;
}

//! Return the paint cache, might be null
const QPixmap *QwtPlotCanvas::paintCache() const
{
    return d_data->cache;
}

//! Invalidate the internal paint cache
void QwtPlotCanvas::invalidatePaintCache()
{
    if ( d_data->cache )
        *d_data->cache = QPixmap();
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

bool QwtPlotCanvas::event( QEvent *event )
{
    if ( event->type() == QEvent::PolishRequest ) 
    {
        if ( testAttribute( Qt::WA_StyledBackground ) )
        {
            // Setting a style sheet changes the 
            // Qt::WA_OpaquePaintEvent attribute, but we insist
            // on painting the background - simply because
            // we can do this much faster because of clipping
            
            setAttribute( Qt::WA_OpaquePaintEvent, true );
        }
    }

    return QFrame::event( event );
}

/*!
  Paint event
  \param event Paint event
*/
void QwtPlotCanvas::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    if ( testAttribute( Qt::WA_OpaquePaintEvent ) )
    {
        painter.save();
        drawBackground( &painter );
        painter.restore();
    }
    
    if ( frameWidth() > 0 && !contentsRect().contains( event->rect() ) )
    {
        painter.save();
        drawFrame( &painter );
        painter.restore();
    }
    
    painter.setClipRegion( contentsRect(), Qt::IntersectClip );

    drawContents( &painter );
}

void QwtPlotCanvas::drawBackground( QPainter *painter )
{
    if ( testAttribute(Qt::WA_StyledBackground ) )
    {
#if DEBUG_BACKGROUND
        QTime time;
        time.start();

        int d1 = 0;
        int d2 = 0;
#endif
        
        QwtClipLogger clipLogger( size() );

        QPainter p( &clipLogger );
        qwtDrawStyledBackground( this, &p );
        p.end();

#if DEBUG_BACKGROUND
        d1 = time.elapsed();
#endif

        QRegion clipRegion;
        if ( painter->hasClipping() )
            clipRegion = painter->transform().map( painter->clipRegion() );
        else
            clipRegion = contentsRect();

        QWidget *bgWidget = NULL;

        for ( int i = 0; i < clipLogger.clipRects.size(); i++ )
        {
            const QRect rect = clipLogger.clipRects[i].toAlignedRect();
            if ( clipRegion.intersects( rect ) )
            {
                if ( bgWidget == NULL )
                {
                    // Try to find out which widget fills
                    // the unfilled areas of the styled background

                    bgWidget = qwtBackgroundWidget( parentWidget() );
                }

                QPixmap pm( rect.size() );
                pm.fill( bgWidget, mapTo( bgWidget, rect.topLeft() ) );
                painter->drawPixmap( rect, pm );
            }
        }

#if DEBUG_BACKGROUND
        d2 = time.elapsed();
#endif
    
        qwtDrawStyledBackground( this, painter );

#if DEBUG_BACKGROUND
        int d3 = time.elapsed();
        qDebug() << d2 - d1 << d3 - d2 << " -> " << d3;
#endif

    }
    else if ( autoFillBackground() )
    {
        const int fw = frameWidth();
        
        QRegion clipRegion = rect().adjusted( fw, fw, -fw, -fw );
        if ( d_data->paintAttributes & PaintCached )
            clipRegion -= contentsRect();

        if ( painter->hasClipping() )
            clipRegion &= painter->clipRegion();

        if ( !clipRegion.isEmpty() )
        {   
            const QBrush autoFillBrush = 
                palette().brush(backgroundRole());
            
            painter->setClipRegion( clipRegion );
            painter->setPen( Qt::NoPen );
            painter->setBrush( autoFillBrush );
            painter->drawRect( rect() );
        }   
    }   
}

/*!
  Redraw the canvas, and focus rect
  \param painter Painter
*/
void QwtPlotCanvas::drawContents( QPainter *painter )
{
    if ( d_data->paintAttributes & PaintCached && d_data->cache
        && d_data->cache->size() == contentsRect().size() )
    {
        painter->drawPixmap( contentsRect().topLeft(), *d_data->cache );
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
    const QRect cr = contentsRect();

    if ( !cr.isValid() )
        return;

    if ( ( d_data->paintAttributes & PaintCached ) && d_data->cache )
    {
        *d_data->cache = QPixmap( cr.size() );

#ifdef Q_WS_X11
        if ( d_data->cache->x11Info().screen() != x11Info().screen() )
            d_data->cache->x11SetScreen( x11Info().screen() );
#endif

        if ( testAttribute(Qt::WA_StyledBackground) ) 
        {
            QPainter bgPainter( d_data->cache );
            bgPainter.translate( -cr.topLeft() );
            drawBackground( &bgPainter );
        }
        else
        {
            d_data->cache->fill( this, cr.topLeft() );
        }

        QPainter cachePainter( d_data->cache );
        cachePainter.translate( -cr.topLeft() );

        ( ( QwtPlot * )parent() )->drawCanvas( &cachePainter );

        cachePainter.end();

        painter->drawPixmap( contentsRect(), *d_data->cache );
    }
    else
    {
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

/*!
   Invalidate the paint cache and repaint the canvas
   \sa invalidatePaintCache()
*/
void QwtPlotCanvas::replot()
{
    invalidatePaintCache();
    repaint( contentsRect() );
}
