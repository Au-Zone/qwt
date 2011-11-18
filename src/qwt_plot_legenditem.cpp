/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_legenditem.h"
#include <qpen.h>
#include <qbrush.h>
#if 1
#include <qdebug.h>
#endif

class QwtPlotLegendItem::PrivateData
{
public:
    PrivateData():
        span( 2 ),
        borderRadius( 0.0 ),
        borderPen( Qt::NoPen ),
        backgroundBrush( Qt::NoBrush ),
        borderDistance( 10 ),
        alignment( Qt::AlignRight | Qt::AlignBottom ),
        orientation( Qt::Vertical )
    {
    }

    int span;

    QFont font;
    QPen textPen;

    double borderRadius;
    QPen borderPen;
    QBrush backgroundBrush;

    int borderDistance;
    Qt::Alignment alignment;
    Qt::Orientation orientation;
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

void QwtPlotLegendItem::setOrientation( Qt::Orientation orientation )
{
    if ( orientation != d_data->orientation )
    {
        d_data->orientation = orientation;
        itemChanged();
    }
}

Qt::Orientation QwtPlotLegendItem::orientation() const
{
    return d_data->orientation;
}

void QwtPlotLegendItem::setSpan( int span )
{
    span = qMax( span, 1 );
    if ( span != d_data->span )
    {
        d_data->span = span;
        itemChanged();
    }
}

int QwtPlotLegendItem::span() const
{
    return d_data->span;
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

    Q_UNUSED( painter );
    Q_UNUSED( canvasRect );
}

void QwtPlotLegendItem::updateLegend( const QwtPlotItem *item,
        const QList<QwtLegendData> &data )
{
    Q_UNUSED( item );
    Q_UNUSED( data );

    qDebug() << "QwtPlotLegendItem::updateLegend: " 
        << item << item->title().text() << data.size();
}

QSizeF QwtPlotLegendItem::legendSize() const
{
    return QSizeF();
}
