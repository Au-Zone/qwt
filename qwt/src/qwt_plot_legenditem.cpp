/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_legenditem.h"
#include <qpalette.h>

class QwtPlotLegendItem::PrivateData
{
public:
    PrivateData():
        borderDistance( -1 ),
        alignment( Qt::AlignRight | Qt::AlignBottom )
    {
    }

    QPalette palette;
    QFont font;
    int borderDistance;
    Qt::Alignment alignment;
};

QwtPlotLegendItem::QwtPlotLegendItem():
    QwtPlotItem( QwtText( "Legend" ) )
{
    d_data = new PrivateData;

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

/*!
   Set the palette
*/
void QwtPlotLegendItem::setPalette( const QPalette &palette )
{
    if ( palette != d_data->palette )
    {
        d_data->palette = palette;
        itemChanged();
    }
}

/*!
   \return palette
   \sa setPalette()
*/
QPalette QwtPlotLegendItem::palette() const
{
    return d_data->palette;
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
}

