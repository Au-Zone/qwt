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

QwtDoubleRect qwtBoundingRect(const QwtSeriesData<QwtDoublePoint>& data)
{
    const size_t sz = data.size();

    if ( sz <= 0 )
        return QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid

    double minX, maxX, minY, maxY;

    const QwtDoublePoint point0 = data.sample(0);
    minX = maxX = point0.x();
    minY = maxY = point0.y();

    for ( size_t i = 1; i < sz; i++ )
    {
        const QwtDoublePoint point = data.sample(i);

        if ( point.x() < minX )
            minX = point.x();
        if ( point.x() > maxX )
            maxX = point.x();

        if ( point.y() < minY )
            minY = point.y();
        if ( point.y() > maxY )
            maxY = point.y();
    }
    return QwtDoubleRect(minX, minY, maxX - minX, maxY - minY);
}

QwtDoubleRect qwtBoundingRect(const QwtSeriesData<QwtIntervalSample>& data)
{
    double minX, maxX, minY, maxY;
    minX = maxX = minY = maxY = 0.0;

    bool isValid = false;

    const size_t sz = data.size();
    for ( size_t i = 0; i < sz; i++ )
    {
        const QwtIntervalSample sample = data.sample(i);

        if ( !sample.interval.isValid() )
            continue;

        if ( !isValid )
        {
            minX = sample.interval.minValue();
            maxX = sample.interval.maxValue();
            minY = maxY = sample.value;

            isValid = true;
        }
        else
        {
            if ( sample.interval.minValue() < minX )
                minX = sample.interval.minValue();
            if ( sample.interval.maxValue() > maxX )
                maxX = sample.interval.maxValue();

            if ( sample.value < minY )
                minY = sample.value;
            if ( sample.value > maxY )
                maxY = sample.value;
        }
    }
    if ( !isValid )
        return QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid

    return QwtDoubleRect(minX, minY, maxX - minX, maxY - minY);
}

QwtDoubleRect qwtBoundingRect(const QwtSeriesData<QwtSetSample>& data)
{
    double minX, maxX, minY, maxY;
    minX = maxX = minY = maxY = 0.0;

    bool isValid = false;

    const size_t sz = data.size();
    for ( size_t i = 0; i < sz; i++ )
    {
        const QwtSetSample sample = data.sample(i);

        if ( !sample.set.isEmpty() )
            continue;

        if ( !isValid )
        {
            minX = sample.set[0];
            maxX = sample.set[0];
            minY = maxY = sample.value;

            isValid = true;
        }

        if ( sample.value < minY )
            minY = sample.value;
        if ( sample.value > maxY )
            maxY = sample.value;

        for ( int i = 0; i < (int)sample.set.size(); i++ )
        {
            if ( sample.set[i] < minX )
                minX = sample.set[i];
            if ( sample.set[i] > maxX )
                maxX = sample.set[i];
        }
    }
    if ( !isValid )
        return QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid

    return QwtDoubleRect(minX, minY, maxX - minX, maxY - minY);
}

QwtPointSeriesData::QwtPointSeriesData(
        const QwtArray<QwtDoublePoint> &samples):
    QwtArraySeriesData<QwtDoublePoint>(samples)
{
}   

QwtSeriesData<QwtDoublePoint> *QwtPointSeriesData::copy() const
{
    return new QwtPointSeriesData(d_samples);
}

QwtDoubleRect QwtPointSeriesData::boundingRect() const
{
    return qwtBoundingRect(*this);
}

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

QwtSetSeriesData::QwtSetSeriesData(
        const QwtArray<QwtSetSample> &samples):
    QwtArraySeriesData<QwtSetSample>(samples)
{
}   

QwtSeriesData<QwtSetSample> *QwtSetSeriesData::copy() const
{
    return new QwtSetSeriesData(d_samples);
}

QwtDoubleRect QwtSetSeriesData::boundingRect() const
{
    return qwtBoundingRect(*this);
}

/*!
  Constructor

  \param x Array of x values
  \param y Array of y values
  
  \sa QwtPlotCurve::setData
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
  \sa QwtPlotCurve::setData
*/
QwtPointArrayData::QwtPointArrayData(const double *x, const double *y, size_t size)
{
#if QT_VERSION >= 0x040000
    d_x.resize(size);
    qMemCopy(d_x.data(), x, size * sizeof(double));

    d_y.resize(size);
    qMemCopy(d_y.data(), y, size * sizeof(double));
#else
    d_x.detach();
    d_x.duplicate(x, size);

    d_y.detach();
    d_y.duplicate(y, size);
#endif
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

  \sa QwtPlotCurve::setData(), QwtPlotCurve::setRawData()
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
