/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_legend_map.h"
#include "qwt_legend_itemmanager.h"
#include "qwt_legend_item.h"
#include<qmap.h>

class QwtLegendMap::PrivateData
{
public:
    QMap<QWidget *, const QwtLegendItemManager *> widgetMap;
    QMap<const QwtLegendItemManager *, QWidget *> itemMap;
};

QwtLegendMap::QwtLegendMap()
{
	d_data = new PrivateData;
}

QwtLegendMap::~QwtLegendMap()
{
	delete d_data;
}

void QwtLegendMap::insert(
    const QwtLegendItemManager *item, QWidget *widget )
{
    d_data->itemMap.insert( item, widget );
    d_data->widgetMap.insert( widget, item );
}

void QwtLegendMap::remove( const QwtLegendItemManager *item )
{
    QWidget *widget = d_data->itemMap[item];
    d_data->itemMap.remove( item );
    d_data->widgetMap.remove( widget );
}

void QwtLegendMap::remove( QWidget *widget )
{
    const QwtLegendItemManager *item = d_data->widgetMap[widget];
    d_data->itemMap.remove( item );
    d_data->widgetMap.remove( widget );
}

void QwtLegendMap::clear()
{

    /*
       We can't delete the widgets in the following loop, because
       we would get ChildRemoved events, changing d_data->itemMap, while
       we are iterating.
     */

    QList<const QWidget *> widgets;

    QMap<const QwtLegendItemManager *, QWidget *>::const_iterator it;
    for ( it = d_data->itemMap.begin(); it != d_data->itemMap.end(); ++it )
        widgets.append( it.value() );

    d_data->itemMap.clear();
    d_data->widgetMap.clear();

    for ( int i = 0; i < widgets.size(); i++ )
        delete widgets[i];
}

uint QwtLegendMap::count() const
{
    return d_data->itemMap.count();
}

const QWidget *QwtLegendMap::find( 
    const QwtLegendItemManager *item ) const
{
    if ( !d_data->itemMap.contains( item ) )
        return NULL;

    return d_data->itemMap[item];
}

QWidget *QwtLegendMap::find( 
    const QwtLegendItemManager *item )
{
    if ( !d_data->itemMap.contains( item ) )
        return NULL;

    return d_data->itemMap[item];
}

const QwtLegendItemManager *QwtLegendMap::find(
    const QWidget *widget ) const
{
    QWidget *w = const_cast<QWidget *>( widget );
    if ( !d_data->widgetMap.contains( w ) )
        return NULL;

    return d_data->widgetMap[w];
}

QwtLegendItemManager *QwtLegendMap::find( const QWidget *widget )
{
    QWidget *w = const_cast<QWidget *>( widget );
    if ( !d_data->widgetMap.contains( w ) )
        return NULL;

    return const_cast<QwtLegendItemManager *>( d_data->widgetMap[w] );
}

//! Return a list of all legend items
QList<QWidget *> QwtLegendMap::legendItems() const
{
    const QMap<QWidget *, const QwtLegendItemManager *> &map = d_data->widgetMap;

    QList<QWidget *> list;

    QMap<QWidget *, const QwtLegendItemManager *>::const_iterator it;
    for ( it = map.begin(); it != map.end(); ++it )
        list += it.key();

    return list;
}
