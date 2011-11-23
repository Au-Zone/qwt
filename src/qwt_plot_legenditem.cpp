/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_legenditem.h"
#include "qwt_dyngrid_layout.h"
#include <qlayoutitem.h>
#include <qpen.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qmath.h>

class QwtLegendLayoutItem: public QLayoutItem
{
public:
    QwtLegendLayoutItem( const QwtPlotLegendItem * );
    virtual ~QwtLegendLayoutItem();

    void setData( const QwtLegendData & );
    QwtLegendData data() const;

    QwtText title() const;
    QPixmap icon() const;

    virtual Qt::Orientations expandingDirections() const;
    virtual QRect geometry() const;
    virtual bool hasHeightForWidth() const;
    virtual int heightForWidth( int w ) const;
    virtual bool isEmpty() const;
    virtual QSize maximumSize() const;
    virtual int minimumHeightForWidth( int w ) const;
    virtual QSize minimumSize() const;
    virtual void setGeometry( const QRect & r );
    virtual QSize sizeHint() const;

    void render( QPainter * ) const;

private:

    const QwtPlotLegendItem *d_legendItem;
    QwtLegendData d_data;
    const int d_spacing;

    QRect d_rect;
};

QwtLegendLayoutItem::QwtLegendLayoutItem( const QwtPlotLegendItem *legendItem ):
    d_legendItem( legendItem ),
    d_spacing( 4 )
{
}

QwtLegendLayoutItem::~QwtLegendLayoutItem()
{
}

void QwtLegendLayoutItem::setData( const QwtLegendData &data )
{
    d_data = data;
}

QwtLegendData QwtLegendLayoutItem::data() const
{
    return d_data;
}

void QwtLegendLayoutItem::render( QPainter *painter ) const
{
    const QRect rect = geometry();
    painter->setClipRect( rect, Qt::IntersectClip );

    int titleOff = 0;

    const QPixmap pm = icon();
    if ( !pm.isNull() )
    {
        QRect pmRect( rect.topLeft(), pm.size() );

        pmRect.moveCenter( 
            QPoint( pmRect.center().x(), rect.center().y() ) );

        painter->drawPixmap( pmRect, pm );

        titleOff += pmRect.width() + d_spacing;
    }

    const QwtText text = title();
    if ( !text.isEmpty() )
    {
        painter->setPen( d_legendItem->textPen() );
        painter->setFont( d_legendItem->font() );

        const QRect textRect = rect.adjusted( titleOff, 0, 0, 0 );
        text.draw( painter, textRect );
    }
}

QwtText QwtLegendLayoutItem::title() const
{
    QwtText text;

    const QVariant titleValue = d_data.value( QwtLegendData::TitleRole );
    if ( qVariantCanConvert<QwtText>( titleValue ) )
    {
        text = qVariantValue<QwtText>( titleValue );
    }
    else if ( qVariantCanConvert<QString>( titleValue ) )
    {
        text.setText( qVariantValue<QString>( titleValue ) );
    }

    return text;
}

QPixmap QwtLegendLayoutItem::icon() const
{
    const QVariant iconValue = d_data.value( QwtLegendData::IconRole );

    QPixmap pm;
    if ( qVariantCanConvert<QPixmap>( iconValue ) )
    {
        pm = qVariantValue<QPixmap>( iconValue );
    }

    return pm;
}

Qt::Orientations QwtLegendLayoutItem::expandingDirections() const
{
    return Qt::Horizontal;
}

bool QwtLegendLayoutItem::hasHeightForWidth() const
{
    return !title().isEmpty();
}

int QwtLegendLayoutItem::minimumHeightForWidth( int w ) const
{
    const QPixmap pm = icon();

    const QwtText text = title();
    if ( text.isEmpty() )
        return pm.height();

    if ( pm.width() > 0 )
        w -= pm.width() + d_spacing;

    const int h = text.heightForWidth( w, d_legendItem->font() );

    return qMax( pm.height(), h );
}

int QwtLegendLayoutItem::heightForWidth( int w ) const
{
    return minimumHeightForWidth( w );
}

bool QwtLegendLayoutItem::isEmpty() const
{
    return false;
}

QSize QwtLegendLayoutItem::maximumSize() const
{
    return QSize( QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX );
}

QSize QwtLegendLayoutItem::minimumSize() const
{
    if ( !d_data.isValid() )
        return QSize( 0, 0 );

    const QPixmap pm = icon();
    const QwtText text = title();

    int w = 0;
    int h = 0;

    if ( !pm.isNull() )
    {
        w = pm.width();
        h = pm.height();
    }

    if ( !text.isEmpty() )
    {
        const QSizeF sz = text.textSize( d_legendItem->font() );

        w += qCeil( sz.width() );
        h = qMax( h, qCeil( sz.height() ) );
    }

    if ( pm.width() > 0 && !text.isEmpty() )
        w += d_spacing;

    return QSize( w, h );
}

QSize QwtLegendLayoutItem::sizeHint() const
{
    return minimumSize();
}

void QwtLegendLayoutItem::setGeometry( const QRect &rect )
{
    d_rect = rect;
}

QRect QwtLegendLayoutItem::geometry() const
{
    return d_rect;
}

class QwtPlotLegendItem::PrivateData
{
public:
    PrivateData():
        borderRadius( 0.0 ),
        borderPen( Qt::NoPen ),
        backgroundBrush( Qt::NoBrush ),
        backgroundMode( QwtPlotLegendItem::LegendBackground ),
        borderDistance( 10 ),
        alignment( Qt::AlignRight | Qt::AlignBottom )
    {
        layout = new QwtDynGridLayout();
        layout->setMaxCols( 2 );

        layout->setSpacing( 10 );
    }

    ~PrivateData()
    {
        delete layout;
    }

    QFont font;
    QPen textPen;

    double borderRadius;
    QPen borderPen;
    QBrush backgroundBrush;
    QwtPlotLegendItem::BackgroundMode backgroundMode;

    int borderDistance;
    Qt::Alignment alignment;

    QMap< const QwtPlotItem *, QList<QwtLegendLayoutItem *> > map;
    QwtDynGridLayout *layout;
};

QwtPlotLegendItem::QwtPlotLegendItem():
    QwtPlotItem( QwtText( "Legend" ) )
{
    d_data = new PrivateData;

    setItemInterest( QwtPlotItem::LegendInterest, true );
    setZ( 100.0 );
}

//! Destructor
QwtPlotLegendItem::~QwtPlotLegendItem()
{
    clearLegend();
    delete d_data;
}

//! \return QwtPlotItem::Rtti_PlotLegend
int QwtPlotLegendItem::rtti() const
{
    return QwtPlotItem::Rtti_PlotLegend;
}

void QwtPlotLegendItem::setAlignment( Qt::Alignment alignment )
{
    if ( d_data->alignment != alignment )
    {
        d_data->alignment = alignment;
        itemChanged();
    }
}

Qt::Alignment QwtPlotLegendItem::alignment() const
{
    return d_data->alignment;
}

void QwtPlotLegendItem::setMaxColumns( uint maxColumns )
{
    if ( maxColumns != d_data->layout->maxCols() )
    {
        d_data->layout->setMaxCols( maxColumns );
        itemChanged();
    }
}

uint QwtPlotLegendItem::maxColumns() const
{
    return d_data->layout->maxCols();
}

/*!
   Change the tick label font
   \sa font()
*/
void QwtPlotLegendItem::setFont( const QFont &font )
{
    if ( font != d_data->font )
    {
        d_data->font = font;
        itemChanged();
    }
}

/*!
   \return tick label font
   \sa setFont()
*/
QFont QwtPlotLegendItem::font() const
{
    return d_data->font;
}

void QwtPlotLegendItem::setBorderDistance( int distance )
{
    if ( distance < 0 )
        distance = -1;

    if ( distance != d_data->borderDistance )
    {
        d_data->borderDistance = distance;
        itemChanged();
    }
}

int QwtPlotLegendItem::borderDistance() const
{
    return d_data->borderDistance;
}

void QwtPlotLegendItem::setBorderRadius( double radius )
{
    radius = qMax( 0.0, radius );

    if ( radius != d_data->borderRadius )
    {
        d_data->borderRadius = radius;
        itemChanged();
    }
}

double QwtPlotLegendItem::borderRadius() const
{
    return d_data->borderRadius;
}

void QwtPlotLegendItem::setBorderPen( const QPen &pen )
{
    if ( d_data->borderPen != pen )
    {
        d_data->borderPen = pen;
        itemChanged();
    }
}

QPen QwtPlotLegendItem::borderPen() const
{
    return d_data->borderPen;
}


void QwtPlotLegendItem::setBackgroundBrush( const QBrush &brush )
{
    if ( d_data->backgroundBrush != brush )
    {
        d_data->backgroundBrush = brush;
        itemChanged();
    }
}

QBrush QwtPlotLegendItem::backgroundBrush() const
{
    return d_data->backgroundBrush;
}

void QwtPlotLegendItem::setBackgroundMode( BackgroundMode mode )
{
    if ( mode != d_data->backgroundMode )
    {
        d_data->backgroundMode = mode;
        itemChanged();
    }
}

QwtPlotLegendItem::BackgroundMode QwtPlotLegendItem::backgroundMode() const
{
    return d_data->backgroundMode;
}

void QwtPlotLegendItem::setTextPen( const QPen &pen )
{
    if ( d_data->textPen != pen )
    {
        d_data->textPen = pen;
        itemChanged();
    }
}

QPen QwtPlotLegendItem::textPen() const
{
    return d_data->textPen;
}

void QwtPlotLegendItem::draw( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect ) const
{
    Q_UNUSED( xMap );
    Q_UNUSED( yMap );

    int m = 0;
    if ( d_data->backgroundMode == QwtPlotLegendItem::LegendBackground )
        m += qCeil( 0.5 * d_data->borderRadius );

    d_data->layout->setContentsMargins( m, m, m, m );
    d_data->layout->setGeometry( geometry( canvasRect ) );

    if ( d_data->backgroundMode == QwtPlotLegendItem::LegendBackground )
        drawBackground( painter, d_data->layout->geometry() );
    
    for ( int i = 0; i <  d_data->layout->count(); i++ )
    {
        const QwtLegendLayoutItem *layoutItem = 
            static_cast<QwtLegendLayoutItem *>( d_data->layout->itemAt( i ) );

        if ( d_data->backgroundMode == QwtPlotLegendItem::ItemBackground )
            drawBackground( painter, layoutItem->geometry() );

        painter->save();
        layoutItem->render( painter );
        painter->restore();
    }
}

void QwtPlotLegendItem::drawBackground( 
    QPainter *painter, const QRectF &rect ) const
{
    painter->save();

    painter->setPen( d_data->borderPen );
    painter->setBrush( d_data->backgroundBrush );
    
    const double radius = d_data->borderRadius;
    painter->drawRoundedRect( rect, radius, radius );
    
    painter->restore();
}

QRect QwtPlotLegendItem::geometry( const QRectF &canvasRect ) const
{
    QRect rect;
    rect.setSize( d_data->layout->sizeHint() );

    int margin = d_data->borderDistance;
    if ( d_data->alignment & Qt::AlignHCenter )
    {
        int x = qRound( canvasRect.center().x() );
        rect.moveCenter( QPoint( x, rect.center().y() ) ); 
    }
    else if ( d_data->alignment & Qt::AlignRight )
    {
        rect.moveRight( qFloor( canvasRect.right() - margin ) );
    }
    else 
    {
        rect.moveLeft( qCeil( canvasRect.left() + margin ) );
    }

    if ( d_data->alignment & Qt::AlignVCenter )
    {
        int y = qRound( canvasRect.center().y() );
        rect.moveCenter( QPoint( rect.center().x(), y ) );
    }
    else if ( d_data->alignment & Qt::AlignBottom )
    {
        rect.moveBottom( qFloor( canvasRect.bottom() - margin ) );
    }
    else 
    {
        rect.moveTop( qCeil( canvasRect.top() + margin ) ); 
    }

    return rect;
}

void QwtPlotLegendItem::updateLegend( const QwtPlotItem *item,
        const QList<QwtLegendData> &data )
{
    if ( item == NULL )
        return;

    QList<QwtLegendLayoutItem *> layoutItems;

    QMap<const QwtPlotItem *, QList<QwtLegendLayoutItem *> >::iterator it = 
        d_data->map.find( item );
    if ( it != d_data->map.end() )
        layoutItems = it.value();

    bool changed = false;

    if ( data.size() != layoutItems.size() )
    {
        changed = true;

        for ( int i = 0; i < layoutItems.size(); i++ )
        {
            d_data->layout->removeItem( layoutItems[i] );
            delete layoutItems[i];
        }
        if ( it != d_data->map.end() )
            d_data->map.remove( item );

        if ( !data.isEmpty() )
        {
            for ( int i = 0; i < data.size(); i++ )
            {
                QwtLegendLayoutItem *layoutItem = new QwtLegendLayoutItem( this );
                d_data->layout->addItem( layoutItem );
                layoutItems += layoutItem;
            }

            d_data->map.insert( item, layoutItems );
        }
    }

    for ( int i = 0; i < data.size(); i++ )
    {
        if ( layoutItems[i]->data().values() != data[i].values() )
        {
            layoutItems[i]->setData( data[i] );
            changed = true;
        }
    }

    if ( changed )
        itemChanged();
}

void QwtPlotLegendItem::clearLegend()
{
    if ( !d_data->map.isEmpty() )
    {
        d_data->map.clear();

        for ( int i = d_data->layout->count() - 1; i >= 0; i-- )
            delete d_data->layout->takeAt( i );

        itemChanged();
    }
}
