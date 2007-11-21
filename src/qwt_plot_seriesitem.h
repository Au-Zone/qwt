/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_SERIES_ITEM_H
#define QWT_PLOT_SERIES_ITEM_H

#include "qwt_global.h"
#include "qwt_plot_item.h"
#include "qwt_scale_div.h"
#include "qwt_series_data.h"
#include <cassert>

template <typename T> 
class QwtPlotSeriesItem: public QwtPlotItem
{
public:
    explicit QwtPlotSeriesItem<T>(const QString &title = QString::null);
    explicit QwtPlotSeriesItem<T>(const QwtText &title);

    virtual ~QwtPlotSeriesItem<T>();

    void setOrientation(Qt::Orientation);
    Qt::Orientation orientation() const;

    void setData(const QwtSeriesData<T> &);
    
    QwtSeriesData<T> &data();
    const QwtSeriesData<T> &data() const;

    int dataSize() const;
    T sample(int i) const;

    virtual QwtDoubleRect boundingRect() const;
    virtual void updateScaleDiv(const QwtScaleDiv &,
        const QwtScaleDiv &);

protected:
    QwtSeriesData<T> *d_series;
    Qt::Orientation d_orientation;
};

template <typename T> 
QwtPlotSeriesItem<T>::QwtPlotSeriesItem(const QString &title):
    QwtPlotItem(QwtText(title)),
    d_series(NULL),
    d_orientation(Qt::Vertical)
{
}

template <typename T> 
QwtPlotSeriesItem<T>::QwtPlotSeriesItem(
        const QwtText &title):
    QwtPlotItem(title),
    d_series(NULL),
    d_orientation(Qt::Vertical)
{
}

template <typename T> 
QwtPlotSeriesItem<T>::~QwtPlotSeriesItem()
{
    delete d_series;
}

/*!
  Assign the curve type

  <dt>Qt::Vertical
  <dd>Draws y as a function of x (the default). The
      baseline is interpreted as a horizontal line
      with y = baseline().</dd>
  <dt>Qt::Horizontal
  <dd>Draws x as a function of y. The baseline is
      interpreted as a vertical line with x = baseline().</dd>

  The baseline is used for aligning the sticks, or
  filling the curve with a brush.

  \sa curveType()
*/
template <typename T> 
void QwtPlotSeriesItem<T>::setOrientation(Qt::Orientation orientation)
{
    if ( d_orientation != orientation )
    {
        d_orientation = orientation;
        itemChanged();
    }
}

template <typename T> 
Qt::Orientation QwtPlotSeriesItem<T>::orientation() const 
{ 
    return d_orientation; 
}

//! \return the the curve data
template <typename T> 
inline QwtSeriesData<T> &QwtPlotSeriesItem<T>::data()
{
    return *d_series;
}

//! \return the the curve data
template <typename T> 
inline const QwtSeriesData<T> &QwtPlotSeriesItem<T>::data() const
{
    return *d_series;
}

/*!
    \param i index
    \return Sample at position i
*/
template <typename T> 
inline T QwtPlotSeriesItem<T>::sample(int i) const 
{ 
    return d_series->sample(i); 
}

/*!
  Assign a series of samples

  \param data Data
  \sa QwtSeriesData<T>::copy()
*/
template <typename T> 
void QwtPlotSeriesItem<T>::setData(const QwtSeriesData<T> &data)
{
    delete d_series;
    d_series = data.copy();
    itemChanged();
}

/*!
  Return the size of the data arrays
  \sa setData()
*/
template <typename T> 
int QwtPlotSeriesItem<T>::dataSize() const
{
    if ( d_series == NULL )
        return 0;

    return d_series->size();
}

/*!
  Returns the bounding rectangle of the curve data. If there is
  no bounding rect, like for empty data the rectangle is invalid.
  \sa QwtSeriesData<T>::boundingRect(), QwtDoubleRect::isValid()
*/
template <typename T> 
QwtDoubleRect QwtPlotSeriesItem<T>::boundingRect() const
{
    if ( d_series == NULL )
        return QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid

    return d_series->boundingRect();
}

template <typename T> 
void QwtPlotSeriesItem<T>::updateScaleDiv(const QwtScaleDiv &xScaleDiv,
        const QwtScaleDiv &yScaleDiv)
{   
    const QwtDoubleRect rect = QwtDoubleRect( 
        xScaleDiv.lBound(), yScaleDiv.lBound(),
        xScaleDiv.range(), yScaleDiv.range());

    d_series->setRectOfInterest(rect);
} 

#endif
