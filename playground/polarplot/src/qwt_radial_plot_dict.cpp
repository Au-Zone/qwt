/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#include "qwt_radial_plot_dict.h"

class QwtRadialPlotDict::PrivateData
{
public:

#if QT_VERSION < 0x040000
    class ItemList: public QValueList<QwtRadialPlotItem *>
#else
    class ItemList: public QList<QwtRadialPlotItem *>
#endif
    {
    public:
        void insertItem(QwtRadialPlotItem *item)
        {
            if ( item == NULL )
                return;

            // Unfortunately there is no inSort operation
            // for lists in Qt4. The implementation below
            // is slow, but there shouldn't be many plot items.

#ifdef __GNUC__
#warning binary search missing
#endif

#if QT_VERSION < 0x040000
            QValueListIterator<QwtRadialPlotItem *> it;
#else
            QList<QwtRadialPlotItem *>::Iterator it;
#endif
            for ( it = begin(); it != end(); ++it )
            {
                if ( *it == item )
                    return;

                if ( (*it)->z() > item->z() )
                {
                    insert(it, item);
                    return;
                }
            }
            append(item);
        }

        void removeItem(QwtRadialPlotItem *item)
        {
            if ( item == NULL )
                return;

            int i = 0;

#if QT_VERSION < 0x040000
            QValueListIterator<QwtRadialPlotItem *> it;
#else
            QList<QwtRadialPlotItem *>::Iterator it;
#endif
            for ( it = begin(); it != end(); ++it )
            {
                if ( item == *it )
                {
#if QT_VERSION < 0x040000
                    remove(it);
#else
                    removeAt(i);
#endif
                    return;
                }
                i++;
            }
        }
    };

    ItemList itemList;
    bool autoDelete;
};

/*! 
   Constructor 

   Auto deletion is enabled.
   \sa setAutoDelete, attachItem
*/
QwtRadialPlotDict::QwtRadialPlotDict()
{
    d_data = new QwtRadialPlotDict::PrivateData;
    d_data->autoDelete = true;
}

/*! 
   Destructor

   If autoDelete is on, all attached items will be deleted
   \sa setAutoDelete, autoDelete, attachItem
*/
QwtRadialPlotDict::~QwtRadialPlotDict()
{
    detachItems(QwtRadialPlotItem::Rtti_RadialPlotItem, d_data->autoDelete);
    delete d_data;
}

/*!
   En/Disable Auto deletion

   If Auto deletion is on all attached plot items will be deleted
   in the destructor of QwtRadialPlotDict. The default value is on.

   \sa autoDelete, attachItem
*/
void QwtRadialPlotDict::setAutoDelete(bool autoDelete)
{
    d_data->autoDelete = autoDelete;
}

/*!
   \return true if auto deletion is enabled
   \sa setAutoDelete, attachItem
*/
bool QwtRadialPlotDict::autoDelete() const
{
    return d_data->autoDelete;
}

/*!
   Attach/Detach a plot item

   Attached items will be deleted in the destructor,
   if auto deletion is enabled (default). Manually detached
   items are not deleted.

   \param item Plot item to attach/detach
   \ on If true attach, else detach the item

   \sa setAutoDelete, ~QwtRadialPlotDict
*/
void QwtRadialPlotDict::attachItem(QwtRadialPlotItem *item, bool on)
{
    if ( on )
        d_data->itemList.insertItem(item);
    else
        d_data->itemList.removeItem(item);
}

/*!
   Detach items from the dictionary

   \param rtti In case of QwtRadialPlotItem::Rtti_PlotItem detach all items 
               otherwise only those items of the type rtti.
   \param autoDelete If true, delete all detached items
*/
void QwtRadialPlotDict::detachItems(int rtti, bool autoDelete)
{
    PrivateData::ItemList list = d_data->itemList;
    QwtRadialPlotItemIterator it = list.begin();
    while ( it != list.end() )
    {
        QwtRadialPlotItem *item = *it;

        ++it; // increment before removing item from the list

        if ( rtti == QwtRadialPlotItem::Rtti_RadialPlotItem || item->rtti() == rtti )
        {
            item->attach(NULL);
            if ( autoDelete )
                delete item;
        }
    }
}

//! \brief A QwtRadialPlotItemList of all attached plot items.
///
/// Use caution when iterating these lists, as removing/detaching an item will
/// invalidate the iterator. Instead you can place pointers to objects to be
/// removed in a removal list, and traverse that list later.
//! \return List of all attached plot items.

const QwtRadialPlotItemList &QwtRadialPlotDict::itemList() const
{
    return d_data->itemList;
}
