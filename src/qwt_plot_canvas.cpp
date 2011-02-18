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
        {
            d_pen = state.pen();
        }
        if ( state.state() == QPaintEngine::DirtyBrush )
        {
            d_brush = state.brush();
        }
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

        for ( int i = 0; i < path.elementCount(); i++ )
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

    void debugPaths() const
    {
        for ( int i = 0; i < border.pathList.count(); i++ )
        {
            qDebug() << "Path: " << i;

            const QPainterPath &path = border.pathList[i];
            for (int j = 0; j < path.elementCount(); j++ )
            {
                QPainterPath::Element el = path.elementAt(j);
                switch( el.type )
                {
                    case QPainterPath::MoveToElement:
                    {
                        qDebug() << j << "MoveTo: " << QPointF( el.x, el.y );
                        break;
                    }
                    case QPainterPath::LineToElement:
                    {
                        qDebug() << j << "LineTo: " << QPointF( el.x, el.y );
                        break;
                    }
                    case QPainterPath::CurveToElement:
                    {
                        qDebug() << j << "CurveTo: " << QPointF( el.x, el.y );
                        break;
                    }
                    case QPainterPath::CurveToDataElement:
                    {
                        qDebug() << j << "CurveToData: " << QPointF( el.x, el.y );
                        break;
                    }
                }
            }
        }
    }

private:
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

static void qwtDrawBackground( QPainter *painter, QWidget *widget )
{
    const QBrush &brush = 
        widget->palette().brush( widget->backgroundRole() );

    if ( brush.style() == Qt::TexturePattern )
    {
        QPixmap pm( widget->size() );
        pm.fill( widget, 0, 0 );
        painter->drawPixmap( 0, 0, pm );
    }
    else if ( brush.gradient() )
    {
        QVector<QRect> rects;

        if ( brush.gradient()->coordinateMode() == QGradient::ObjectBoundingMode )
        {
            rects += widget->rect();
        } 
        else 
        {
            rects = painter->clipRegion().rects();
        }

#if 1
        bool useRaster = false;

        if ( painter->paintEngine()->type() == QPaintEngine::X11 )
        {
            // Qt 4.7.1: gradients on X11 are broken ( subrects + 
            // QGradient::StretchToDeviceMode ) and horrible slow.
            // As workaround we have to use the raster paintengine.
            // Even if the QImage -> QPixmap translation is slow
            // it is three times faster, than using X11 directly

            useRaster = true;
        }
#endif
        if ( useRaster )
        {
            QImage::Format format = QImage::Format_RGB32;

            const QGradientStops stops = brush.gradient()->stops();
            for ( int i = 0; i < stops.size(); i++ )
            {
                if ( stops[i].second.alpha() != 255 )
                {
                    // don't use Format_ARGB32_Premultiplied. It's
                    // recommended by the Qt docs, but QPainter::drawImage()
                    // is horrible slow on X11.

                    format = QImage::Format_ARGB32;
                    break;
                }
            }
            
            QImage image( widget->size(), format );

            QPainter p( &image );
            p.setPen( Qt::NoPen );
            p.setBrush( brush );

            p.drawRects( rects );

            p.end();

            painter->drawImage( 0, 0, image );
        }
        else
        {
            painter->save();

            painter->setPen( Qt::NoPen );
            painter->setBrush( brush );

            painter->drawRects( rects );

            painter->restore();
        }
    }
    else
    {
        painter->save();

        painter->setPen( Qt::NoPen );
        painter->setBrush( brush );

        painter->drawRects( painter->clipRegion().rects() );

        painter->restore();
    }
}

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

static void qwtFillBackground( QPainter *painter, QWidget *widget )
{
    QwtClipLogger clipLogger( widget->size() );

    QPainter p( &clipLogger );
    qwtDrawStyledBackground( widget, &p );
    p.end();

    QRegion clipRegion;
    if ( painter->hasClipping() )
        clipRegion = painter->transform().map( painter->clipRegion() );
    else
        clipRegion = widget->contentsRect();

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

                bgWidget = qwtBackgroundWidget( widget->parentWidget() );
            }

            QPixmap pm( rect.size() );
            pm.fill( bgWidget, widget->mapTo( bgWidget, rect.topLeft() ) );
            painter->drawPixmap( rect, pm );
        }
    }
}

static QRegion qwtBorderClipRegion( const QWidget *w, const QRect &rect ) 
{
    int left, top, right, bottom;
    w->getContentsMargins( &left, &top, &right, &bottom );

    const QRect cRect = rect.adjusted( left, top, -right, -bottom );

    if ( !w->testAttribute(Qt::WA_StyledBackground ) )
        return cRect;

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
    // antialiasing - but in the end this is not important as
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
                        QPixmap::grabWidget( this, rect() );
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
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    if ( testPaintAttribute( QwtPlotCanvas::BackingStore ) &&
        d_data->backingStore != NULL )
    {
        QPixmap &bs = *d_data->backingStore;
        if ( bs.size() != size() )
        {
            bs = QPixmap( size() );

#ifdef Q_WS_X11
            if ( bs.x11Info().screen() != x11Info().screen() )
                bs.x11SetScreen( x11Info().screen() );
#endif

            if ( testAttribute(Qt::WA_StyledBackground) )
            {
                QPainter p( &bs );
                qwtFillBackground( &p, this );
                drawCanvas( &p, true );
            }
            else
            {
                bs.fill( this, 0, 0 );

                QPainter p( &bs );
                drawCanvas( &p, false );

                if ( frameWidth() > 0 )
                    drawFrame( &p );
            }
        }

        painter.drawPixmap( 0, 0, *d_data->backingStore );
    }
    else
    {
        if ( testAttribute(Qt::WA_StyledBackground ) )
        {
            if ( testAttribute( Qt::WA_OpaquePaintEvent ) )
            {
                qwtFillBackground( &painter, this );
                drawCanvas( &painter, true );
            }
            else
            {
                drawCanvas( &painter, false );
            }
        }
        else
        {
            if ( testAttribute( Qt::WA_OpaquePaintEvent ) )
            {
                if ( autoFillBackground() )
                    qwtDrawBackground( &painter, this );
            }

            drawCanvas( &painter, false );

            if ( ( frameWidth() > 0 ) 
                && !contentsRect().contains( event->rect() ) )
            {
                drawFrame( &painter );
            }
        }
    }

    if ( hasFocus() && focusIndicator() == CanvasFocusIndicator )
        drawFocusIndicator( &painter );
}

void QwtPlotCanvas::drawCanvas( QPainter *painter, bool styled ) 
{
    if ( styled )
        qwtDrawStyledBackground( this, painter );

    painter->save();
    painter->setClipRegion( d_data->canvasClip, Qt::IntersectClip );

    plot()->drawCanvas( painter );

    painter->restore();
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
