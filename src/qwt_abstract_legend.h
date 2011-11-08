/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_ABSTRACT_LEGEND_H
#define QWT_ABSTRACT_LEGEND_H

#include "qwt_global.h"
#include "qwt_legend_data.h"
#include <qframe.h>
#include <qlist.h>

class QwtPlotItem;

class QWT_EXPORT QwtAbstractLegend : public QFrame
{
    Q_OBJECT

public:
    explicit QwtAbstractLegend( QWidget *parent = NULL );
    virtual ~QwtAbstractLegend();

    virtual void renderLegend( QPainter *, 
        const QRectF &, bool fillBackground ) const = 0;

    virtual bool isEmpty() const = 0;

	virtual int scrollExtent( Qt::Orientation ) const;

public Q_SLOTS:
    virtual void updateLegend( const QwtPlotItem *, 
        const QList<QwtLegendData> & ) = 0;
};

#endif 
