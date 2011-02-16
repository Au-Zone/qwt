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
#include <qbitmap.h>
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

    virtual void updateState( const QPaintEngineState &state )
    {
        if ( state.state() == QPaintEngine::DirtyPen )
            d_pen = state.pen();
        if ( state.state() == QPaintEngine::DirtyBrush )
            d_brush = state.brush();
        if ( state.state() == QPaintEngine::DirtyBrushOrigin )
            d_origin = state.brushOrigin();
    }

    virtual void drawPath( const QPainterPath &path )
    {
        if ( d_pathCount == 0 )
        {
            setCornerRects( path );
            alignCornerRects( QRectF( QPointF( 0.0, 0.0 ) , size() ) );

            background.path = path;
            background.brush = d_brush;
            background.origin = d_origin;
        }
        else
        {
            border.pathList += path;
            
            if ( d_pathCount == 1 )
                border.pen = d_pen;
            
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

    void alignCornerRects( const QRectF &rect )
    {
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


public:
    QVector<QRectF> clipRects;

    struct
    {
        QList<QPainterPath> pathList;
        QPen pen;
    } border;

    struct
    {
        QPainterPath path;
        QBrush brush;
        QPointF origin;
    } background;

private:
    uint d_pathCount;
    QPen d_pen;
    QBrush d_brush;
    QPointF d_origin;
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

static QRegion qwtBorderClipRegion( const QWidget *w, const QRect &rect ) 
{
    int left, top, right, bottom;
    w->getContentsMargins( &left, &top, &right, &bottom );

    const QRect cRect = rect.adjusted( left, top, -right, -bottom );

    if ( !w->testAttribute(Qt::WA_StyledBackground ) )
        return cRect;

#if DEBUG_BACKGROUND
    QTime time;
    time.start();
#endif

    QwtClipLogger clipLogger( rect.size() );

    QPainter painter( &clipLogger );

    QStyleOption opt;
    opt.initFrom(w);
    opt.rect = rect;
    w->style()->drawPrimitive( QStyle::PE_Widget, &opt, &painter, w);

    painter.end();

    if ( clipLogger.clipRects.size() == 0 )
    {
        // No rounded border
        return cRect;
    }

    // The algorithm below doesn't take care of the pixels filled by
    // antialiasing - but int the end this is not important as
    // the only way to get a perfect solution is to paint the border
    // last.

    QBitmap bitmap( cRect.size() );
    bitmap.fill( Qt::color0 );

    QPen pen( clipLogger.border.pen );
    if ( pen.style() != Qt::NoPen )
    {
        pen.setStyle( Qt::SolidLine );
        pen.setColor( Qt::color0 );
    }

    painter.begin( &bitmap );
    painter.translate( -cRect.topLeft() );
    painter.setPen( pen );
    painter.setBrush( Qt::color1 );
    painter.drawPath( clipLogger.background.path );
    painter.end();

    QRegion region = bitmap;
    region.translate( cRect.topLeft() );

#if DEBUG_BACKGROUND
    qDebug() << "QwtPlotCanvas::canvasClipRegion: " << time.elapsed();
#endif

    return region;
}

class QwtPlotCanvas::PrivateData
{
public:
    PrivateData():
        focusIndicator( NoFocusIndicator ),
        paintAttributes( 0 ),
        backingStore( NULL )
    {
    }

    ~PrivateData()
    {
        delete backingStore;
    }

    FocusIndicator focusIndicator;
    int paintAttributes;
    QPixmap *backingStore;

    QRegion canvasClip;
};

//! Sets a cross cursor, enables QwtPlotCanvas::BackingStore

QwtPlotCanvas::QwtPlotCanvas( QwtPlot *plot ):
    QFrame( plot )
{
    d_data = new PrivateData;

#ifndef QT_NO_CURSOR
    setCursor( Qt::CrossCursor );
#endif

    setAutoFillBackground( true );
    setPaintAttribute( QwtPlotCanvas::BackingStore, true );
    setPaintAttribute( QwtPlotCanvas::Opaque, true );
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

  \sa testPaintAttribute(), backingStore()
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
        case BackingStore:
        {
            if ( on )
            {
                if ( d_data->backingStore == NULL )
                    d_data->backingStore = new QPixmap();

                if ( isVisible() )
                {
                    *d_data->backingStore = 
                        QPixmap::grabWidget( this, contentsRect() );
                }
            }
            else
            {
                delete d_data->backingStore;
                d_data->backingStore = NULL;
            }
            break;
        }
        case Opaque:
        {
            if ( on )
                setAttribute( Qt::WA_OpaquePaintEvent, true );

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

//! Return the backing store, might be null
QPixmap *QwtPlotCanvas::backingStore()
{
    return d_data->backingStore;
}

//! Return the backing store, might be null
const QPixmap *QwtPlotCanvas::backingStore() const
{
    return d_data->backingStore;
}

//! Invalidate the internal backing store
void QwtPlotCanvas::invalidateBackingStore()
{
    if ( d_data->backingStore )
        *d_data->backingStore = QPixmap();
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
        if ( testPaintAttribute( QwtPlotCanvas::Opaque ) )
        {
            // Setting a style sheet changes the 
            // Qt::WA_OpaquePaintEvent attribute, but we insist
            // on painting the background.
            
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
    const QRect cr = contentsRect();

    QPainter painter( this );
    painter.setClipRegion( event->region() );

    if ( testAttribute( Qt::WA_OpaquePaintEvent ) )
    {
        bool drawBackground = true;

        if ( testPaintAttribute( QwtPlotCanvas::BackingStore )
            && d_data->backingStore )
        {
            // When we have a backingstore, we can update
            // completely from the backingstore in certain
            // situations.

            if ( cr.contains( event->rect() ) )
            {
                // the update region is completely inside the 
                // cached contents rectangle
                drawBackground = false;
            }
            else if ( !testAttribute(Qt::WA_StyledBackground ) )
            {
                // The widget might be covered completely by frame
                // and the cached contents rect

                if ( frameRect() == rect() && frameWidth() > 0 )
                {
                    int fw = frameWidth();
                    QRect innerRect = frameRect().adjusted( fw, fw, -fw, -fw );
                    if ( cr.contains( innerRect ) )
                        drawBackground = false;
                }
            }
        }

        if ( drawBackground )
        {
            if ( testAttribute(Qt::WA_StyledBackground ) )
            {
                drawStyledBackground( &painter );
            }
            else if ( autoFillBackground() )
            {
                painter.fillRect( rect(), palette().brush( backgroundRole() ) );
            }   
        }
    }
    
    if ( frameWidth() > 0 && !cr.contains( event->rect() ) )
        drawFrame( &painter );
    
    painter.save();
    painter.setClipRegion( cr, Qt::IntersectClip );

    if ( testPaintAttribute( QwtPlotCanvas::BackingStore ) 
        && d_data->backingStore )
    {
        QPixmap &bs = *d_data->backingStore;
        if ( bs.size() != cr.size() )
        {
            bs = QPixmap( cr.size() );

#ifdef Q_WS_X11
            if ( bs.x11Info().screen() != x11Info().screen() )
                bs.x11SetScreen( x11Info().screen() );
#endif

            if ( testAttribute(Qt::WA_StyledBackground) )
            {
                QPainter bgPainter( &bs );
                bgPainter.translate( -cr.topLeft() );
                drawStyledBackground( &bgPainter );
            }
            else
            {
                bs.fill( this, cr.topLeft() );
            }

            QPainter cachePainter( &bs );
            cachePainter.translate( -cr.topLeft() );

            cachePainter.setClipRegion( d_data->canvasClip, Qt::IntersectClip );
            plot()->drawCanvas( &cachePainter );
        }

        painter.drawPixmap( contentsRect().topLeft(), *d_data->backingStore );
    }
    else
    {
        painter.setClipRegion( d_data->canvasClip, Qt::IntersectClip );
        plot()->drawCanvas( &painter );
    }

    painter.restore();

    if ( hasFocus() && focusIndicator() == CanvasFocusIndicator )
        drawFocusIndicator( &painter );
}

/*!
  Resize event
  \param event Resize event
*/
void QwtPlotCanvas::resizeEvent( QResizeEvent *event )
{
    updateCanvasClip();
    QFrame::resizeEvent( event );
}

/*!
  Change event
  \param event Change event
*/
void QwtPlotCanvas::changeEvent( QEvent *event )
{
    if ( event->type() == QEvent::StyleChange ) 
        updateCanvasClip();

    QFrame::changeEvent( event );
}

void QwtPlotCanvas::drawStyledBackground( QPainter *painter )
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
    qDebug() << "QwtPlotCanvas::drawStyledBackground: "
        << d2 - d1 << d3 - d2 << " -> " << d3;
#endif
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
    invalidateBackingStore();
    repaint( contentsRect() );
}

void QwtPlotCanvas::updateCanvasClip()
{
    d_data->canvasClip = qwtBorderClipRegion( this, rect() );
}

QRegion QwtPlotCanvas::borderClip( const QRect &rect ) const
{
    return qwtBorderClipRegion( this, rect );
}
