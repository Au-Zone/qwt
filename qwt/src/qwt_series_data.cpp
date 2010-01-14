/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_math.h"
#include "qwt_series_data.h"

static inline QwtDoubleRect qwtBoundingRect(const QwtDoublePoint &sample)
{
    return QwtDoubleRect(sample.x(), sample.y(), 0.0, 0.0);
}

static inline QwtDoubleRect qwtBoundingRect(const QwtDoublePoint3D &sample)
{
    return QwtDoubleRect(sample.x(), sample.y(), 0.0, 0.0);
}

static inline QwtDoubleRect qwtBoundingRect(const QwtIntervalSample &sample)
{
    return QwtDoubleRect(sample.interval.minValue(), sample.value, 
        sample.interval.maxValue() - sample.interval.minValue(), 0.0);
}

static inline QwtDoubleRect qwtBoundingRect(const QwtSetSample &sample)
{
    double minX = sample.set[0];
    double maxX = sample.set[0];

    for ( int i = 1; i < (int)sample.set.size(); i++ )
    {
        if ( sample.set[i] < minX )
            minX = sample.set[i];
        if ( sample.set[i] > maxX )
            maxX = sample.set[i];
    }

    double minY = sample.value;
    double maxY = sample.value;

    return QwtDoubleRect(minX, minY, maxX - minX, maxY - minY);
}

/*!
  \brief Calculate the bounding rect of a series

  Slow implementation, that iterates over the series.

  \param series Series
  \return Bounding rectangle
*/

template <class T> 
QwtDoubleRect qwtBoundingRectT(const QwtSeriesData<T>& series)
{
    QwtDoubleRect boundingRect(1.0, 1.0, -2.0, -2.0); // invalid;

    const size_t sz = series.size();
    if ( sz <= 0 )
        return boundingRect;

    size_t i;
    for ( i = 0; i < sz; i++ )
    {
        const QwtDoubleRect rect = qwtBoundingRect(series.sample(i));
        if ( rect.width() >= 0.0 && rect.height() >= 0.0 )
        {
            boundingRect = rect;
            i++;
            break;
        }
    }

    for ( ;i < sz; i++ )
    {
        const QwtDoubleRect rect = qwtBoundingRect(series.sample(i));
        if ( rect.width() >= 0.0 && rect.height() >= 0.0 )
        {
            boundingRect.setLeft(qwtMin(boundingRect.left(), rect.left()));
            boundingRect.setRight(qwtMax(boundingRect.right(), rect.right()));
            boundingRect.setTop(qwtMin(boundingRect.top(), rect.top()));
            boundingRect.setBottom(qwtMax(boundingRect.bottom(), rect.bottom()));
        }
    }

    return boundingRect;
}

/*!
  \brief Calculate the bounding rect of a series

  Slow implementation, that iterates over the series.

  \param series Series
  \return Bounding rectangle
*/
QwtDoubleRect qwtBoundingRect(const QwtSeriesData<QwtDoublePoint> &series)
{
    return qwtBoundingRectT<QwtDoublePoint>(series);
}

/*!
  \brief Calculate the bounding rect of a series

  Slow implementation, that iterates over the series.

  \param series Series
  \return Bounding rectangle
*/
QwtDoubleRect qwtBoundingRect(const QwtSeriesData<QwtDoublePoint3D> &series)
{
    return qwtBoundingRectT<QwtDoublePoint3D>(series);
}

/*!
  \brief Calculate the bounding rect of a series

  Slow implementation, that iterates over the series.

  \param series Series
  \return Bounding rectangle
*/
QwtDoubleRect qwtBoundingRect(const QwtSeriesData<QwtIntervalSample>& series)
{
    return qwtBoundingRectT<QwtIntervalSample>(series);
}

/*!
  \brief Calculate the bounding rect of a series

  Slow implementation, that iterates over the series.

  \param series Series
  \return Bounding rectangle
*/
QwtDoubleRect qwtBoundingRect(const QwtSeriesData<QwtSetSample>& series)
{
    return qwtBoundingRectT<QwtSetSample>(series);
}

/*! 
   Constructor
   \param samples Samples
*/
QwtPointSeriesData::QwtPointSeriesData(
        const QwtArray<QwtDoublePoint> &samples):
    QwtArraySeriesData<QwtDoublePoint>(samples)
{
}   

//! Copy operator
QwtSeriesData<QwtDoublePoint> *QwtPointSeriesData::copy() const
{
    return new QwtPointSeriesData(d_samples);
}

/*!
  \brief Calculate the bounding rect

  This implementation iterates over all points. 
  \return Bounding rectangle
*/
QwtDoubleRect QwtPointSeriesData::boundingRect() const
{
    return qwtBoundingRect(*this);
}

/*! 
   Constructor
   \param samples Samples
*/
QwtPoint3DSeriesData::QwtPoint3DSeriesData(
        const QwtArray<QwtDoublePoint3D> &samples):
    QwtArraySeriesData<QwtDoublePoint3D>(samples)
{
}

//! Copy operator
QwtSeriesData<QwtDoublePoint3D> *QwtPoint3DSeriesData::copy() const
{
    return new QwtPoint3DSeriesData(d_samples);
}

QwtDoubleRect QwtPoint3DSeriesData::boundingRect() const
{
    return qwtBoundingRect(*this);
}

/*! 
   Constructor
   \param samples Samples
*/
QwtIntervalSeriesData::QwtIntervalSeriesData(
        const QwtArray<QwtIntervalSample> &samples):
    QwtArraySeriesData<QwtIntervalSample>(samples)
{
}   

QwtSeriesData<QwtIntervalSample> *QwtIntervalSeriesData::copy() const
{
    return new QwtIntervalSeriesData(d_samples);
}

QwtDoubleRect QwtIntervalSeriesData::boundingRect() const
{
    return qwtBoundingRect(*this);
}

/*! 
   Constructor
   \param samples Samples
*/
QwtSetSeriesData::QwtSetSeriesData(
        const QwtArray<QwtSetSample> &samples):
    QwtArraySeriesData<QwtSetSample>(samples)
{
}   

QwtSeriesData<QwtSetSample> *QwtSetSeriesData::copy() const
{
    return new QwtSetSeriesData(d_samples);
}

/*!
  \brief Calculate the bounding rect

  This implementation iterates over all points. 
  \return Bounding rectangle
*/
QwtDoubleRect QwtSetSeriesData::boundingRect() const
{
    return qwtBoundingRect(*this);
}

/*!
  Constructor

  \param x Array of x values
  \param y Array of y values
  
  \sa QwtPlotCurve::setData(), QwtPlotCurve::setSamples()
*/
QwtPointArrayData::QwtPointArrayData(
        const QwtArray<double> &x, const QwtArray<double> &y): 
    d_x(x), 
    d_y(y)
{
}

/*!
  Constructor
  
  \param x Array of x values
  \param y Array of y values
  \param size Size of the x and y arrays
  \sa QwtPlotCurve::setData(), QwtPlotCurve::setSamples()
*/
QwtPointArrayData::QwtPointArrayData(const double *x, const double *y, size_t size)
{
    d_x.resize(size);
    qMemCopy(d_x.data(), x, size * sizeof(double));

    d_y.resize(size);
    qMemCopy(d_y.data(), y, size * sizeof(double));
}

//! Assignment 
QwtPointArrayData& QwtPointArrayData::operator=(const QwtPointArrayData &data)
{
    if (this != &data)
    {
        d_x = data.d_x;
        d_y = data.d_y;
    }
    return *this;
}

/*!
  \brief Calculate the bounding rect

  This implementation iterates over all points. 
  \return Bounding rectangle
*/
QwtDoubleRect QwtPointArrayData::boundingRect() const
{
    return qwtBoundingRect(*this);
}

//! \return Size of the data set 
size_t QwtPointArrayData::size() const 
{ 
    return qwtMin(d_x.size(), d_y.size()); 
}

/*!
  Return the sample at position i

  \param i Index
  \return Sample at position i
*/
QwtDoublePoint QwtPointArrayData::sample(size_t i) const 
{ 
    return QwtDoublePoint(d_x[int(i)], d_y[int(i)]); 
}

//! \return Array of the x-values
const QwtArray<double> &QwtPointArrayData::xData() const
{
    return d_x;
}

//! \return Array of the y-values
const QwtArray<double> &QwtPointArrayData::yData() const
{
    return d_y;
}

/*!
  \return Pointer to a copy (virtual copy constructor)
*/
QwtSeriesData<QwtDoublePoint> *QwtPointArrayData::copy() const 
{ 
    return new QwtPointArrayData(d_x, d_y); 
}

/*!
  Constructor

  \param x Array of x values
  \param y Array of y values
  \param size Size of the x and y arrays

  \warning The programmer must assure that the memory blocks referenced
           by the pointers remain valid during the lifetime of the 
           QwtPlotCPointer object.

  \sa QwtPlotCurve::setData(), QwtPlotCurve::setRawSamples()
*/
QwtCPointerData::QwtCPointerData(
    const double *x, const double *y, size_t size):
    d_x(x), 
    d_y(y), 
    d_size(size)
{
}

//! Assignment 
QwtCPointerData& QwtCPointerData::operator=(const QwtCPointerData &data)
{
    if (this != &data)
    {
        d_x = data.d_x;
        d_y = data.d_y;
        d_size = data.d_size;
    }
    return *this;
}

/*!
  \brief Calculate the bounding rect

  This implementation iterates over all points. 
  \return Bounding rectangle
*/
QwtDoubleRect QwtCPointerData::boundingRect() const
{
    return qwtBoundingRect(*this);
}

//! \return Size of the data set 
size_t QwtCPointerData::size() const 
{   
    return d_size; 
}

/*!
  Return the sample at position i

  \param i Index
  \return Sample at position i
*/
QwtDoublePoint QwtCPointerData::sample(size_t i) const 
{ 
    return QwtDoublePoint(d_x[int(i)], d_y[int(i)]); 
}

//! \return Array of the x-values
const double *QwtCPointerData::xData() const
{
    return d_x;
}

//! \return Array of the y-values
const double *QwtCPointerData::yData() const
{
    return d_y;
}

/*!
  \return Pointer to a copy (virtual copy constructor)
*/
QwtSeriesData<QwtDoublePoint> *QwtCPointerData::copy() const 
{
    return new QwtCPointerData(d_x, d_y, d_size);
}

/*! 
   Constructor

   \param size Number of points
   \param interval Bounding interval for the points

   \sa setInterval(), setSize()
*/
QwtSyntheticPointData::QwtSyntheticPointData(
        size_t size, const QwtDoubleInterval &interval):
    d_size(size),
    d_interval(interval)
{
}

/*!
  Change the number of points
   
  \param size Number of points
  \sa size(), setInterval()
*/
void QwtSyntheticPointData::setSize(size_t size)
{
   d_size = size;
}

/*!
  \return Number of points
  \sa setSize(), interval()
*/
size_t QwtSyntheticPointData::size() const
{
    return d_size;
}

/*!
   Set the bounding interval

   \param interval Interval
   \sa interval(), setSize()
*/
void QwtSyntheticPointData::setInterval(const QwtDoubleInterval &interval)
{
    d_interval = interval.normalized();
}

/*!
   \return Bounding interval
   \sa setInterval(), size()
*/
QwtDoubleInterval QwtSyntheticPointData::interval() const
{
    return d_interval;
}

/*!
   Set a the "rect of interest"

   QwtPlotSeriesItem defines the current area of the plot canvas
   as "rect of interest" ( QwtPlotSeriesItem::updateScaleDiv() ).

   If interval().isValid() == false the x values are calculated
   in the interval rect.left() -> rect.right(). 

   \sa rectOfInterest()
*/
void QwtSyntheticPointData::setRectOfInterest(const QwtDoubleRect &rect)
{
    d_rectOfInterest = rect;
    d_intervalOfInterest = QwtDoubleInterval(
        rect.left(), rect.right()).normalized();
}

/*!
   \return "rect of interest"
   \sa setRectOfInterest()
*/
QwtDoubleRect QwtSyntheticPointData::rectOfInterest() const
{
   return d_rectOfInterest;
}

/*!
  \brief Calculate the bounding rect

  This implementation iterates over all points. 
  \return Bounding rectangle
*/
QwtDoubleRect QwtSyntheticPointData::boundingRect() const
{
    if ( d_size == 0 || !d_interval.isValid() )
        return QwtDoubleRect();

    return qwtBoundingRect(*this);
}

/*!
   Calculate the point from an index

   \param index Index
   \return QwtDoublePoint(x(index), y(x(index)));

   \warning For invalid indices ( index < 0 || index >= size() ) 
            (0, 0) is returned.
*/
QwtDoublePoint QwtSyntheticPointData::sample(size_t index) const
{
    if ( index >= d_size )
        return QwtDoublePoint(0, 0);

    const double xValue = x(index);
    const double yValue = y(xValue);

    return QwtDoublePoint(xValue, yValue);
}

/*!
   Calculate a x-value from an index

   x values are calculated by deviding an interval into 
   equidistant steps. If !interval().isValid() the
   interval is calculated from the "rect of interest".

   \sa interval(), rectOfInterest(), y()
*/
double QwtSyntheticPointData::x(uint index) const
{
    const QwtDoubleInterval &interval = d_interval.isValid() ?
        d_interval : d_intervalOfInterest;

    if ( !interval.isValid() || d_size == 0 || index >= d_size)
        return 0.0;

    const double dx = interval.width() / d_size;
    return interval.minValue() + index * dx;
}
