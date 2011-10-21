/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_LEGEND_MAP_H
#define QWT_LEGEND_MAP_H

#include "qwt_global.h"
#include <qlist.h>

class QwtLegendItemManager;
class QWidget;

class QWT_EXPORT QwtLegendMap
{
public:
	QwtLegendMap();
	~QwtLegendMap();

    void insert( const QwtLegendItemManager *, QWidget * );

    void remove( const QwtLegendItemManager * );
    void remove( QWidget * );

    void clear();

    uint count() const;

    const QWidget *find( const QwtLegendItemManager * ) const;
    QWidget *find( const QwtLegendItemManager * );

    const QwtLegendItemManager *find( const QWidget * ) const;
    QwtLegendItemManager *find( const QWidget * );

	QList<QWidget *> legendItems() const;

private:
	class PrivateData;
	PrivateData *d_data;
};

#endif 
