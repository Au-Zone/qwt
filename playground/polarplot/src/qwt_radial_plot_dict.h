/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

/*! \file !*/
#ifndef QWT_RDAIAL_PLOT_DICT
#define QWT_RDAIAL_PLOT_DICT

#include "qwt_global.h"
#include "qwt_radial_plot_item.h"

#if QT_VERSION < 0x040000
#include <qvaluelist.h>
typedef QValueListConstIterator<QwtRadialPlotItem *> QwtRadialPlotItemIterator;
/// \var typedef QValueList< QwtRadialPlotItem *> QwtRadialPlotItemList
/// \brief See QT 3.x assistant documentation for QValueList
typedef QValueList<QwtRadialPlotItem *> QwtRadialPlotItemList;
#else
#include <qlist.h>
typedef QList<QwtRadialPlotItem *>::ConstIterator QwtRadialPlotItemIterator;
/// \var typedef QList< QwtRadialPlotItem *> QwtRadialPlotItemList
/// \brief See QT 4.x assistant documentation for QList
typedef QList<QwtRadialPlotItem *> QwtRadialPlotItemList;
#endif

/*!
  \brief A dictionary for plot items

  QwtRadialPlotDict organizes plot items in increasing z-order.
  If autoDelete() is enabled, all attached items will be deleted
  in the destructor of the dictionary.

  \sa QwtRadialPlotItem::attach(), QwtRadialPlotItem::detach(), QwtRadialPlotItem::z()
*/
class QWT_EXPORT QwtRadialPlotDict
{
public:
    explicit QwtRadialPlotDict();
    ~QwtRadialPlotDict();

    void setAutoDelete(bool);
    bool autoDelete() const;

    const QwtRadialPlotItemList& itemList() const;

    void detachItems(int rtti = QwtRadialPlotItem::Rtti_RadialPlotItem,
        bool autoDelete = true);

private:
    friend class QwtRadialPlotItem;

    void attachItem(QwtRadialPlotItem *, bool);

    class PrivateData;
    PrivateData *d_data;
};

#endif
