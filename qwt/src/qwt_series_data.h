/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_SERIES_DATA_H
#define QWT_SERIES_DATA_H 1

#include "qwt_global.h"
#include "qwt_array.h"
#include "qwt_double_rect.h"
#include "qwt_double_interval.h"

class QwtIntervalSample
{
public:
    QwtIntervalSample():
        value(0.0)
    {
    }

    double value;
    QwtDoubleInterval interval;
};

class QwtSetSample
{
public:
    QwtSetSample():
        value(0.0)
    {
    }

    double value;
    QwtArray<double> set;
};

template <typename T> 
class QwtSeriesData
{
public:
    virtual ~QwtSeriesData() {} 

    //! \return Pointer to a copy (virtual copy constructor)
    virtual QwtSeriesData *copy() const = 0;

    //! \return Size of the data set
    virtual size_t size() const = 0;

    /*!
      Return a sample
      \param i Index
      \return sample at position i
     */
    virtual T sample(size_t i) const = 0;

    virtual QwtDoubleRect boundingRect() const = 0;

private:
    /*!
      Assignment operator (virtualized)
     */
    QwtSeriesData<T> &operator=(const QwtSeriesData<T> &);
};

template <typename T>
class QwtArraySeriesData: public QwtSeriesData<T>
{
public:
    QwtArraySeriesData();
    QwtArraySeriesData(const QwtArray<T> &);

    void setData(const QwtArray<T> &);
    const QwtArray<T> data() const;

    virtual size_t size() const;
    virtual T sample(size_t) const;

protected:
    QwtArray<T> d_samples;
};

template <typename T>
QwtArraySeriesData<T>::QwtArraySeriesData()
{
}

template <typename T>
QwtArraySeriesData<T>::QwtArraySeriesData(const QwtArray<T> &samples):
    d_samples(samples)
{
}
    
template <typename T>
void QwtArraySeriesData<T>::setData(const QwtArray<T> &samples)
{
    d_samples = samples;
}

template <typename T>
size_t QwtArraySeriesData<T>::size() const
{
    return d_samples.size();
}

template <typename T>
T QwtArraySeriesData<T>::sample(size_t i) const
{
    return d_samples[i];
}

class QWT_EXPORT QwtPointSeriesData: public QwtArraySeriesData<QwtDoublePoint>
{
public:
    QwtPointSeriesData(
        const QwtArray<QwtDoublePoint> & = QwtArray<QwtDoublePoint>());

    virtual QwtSeriesData<QwtDoublePoint> *copy() const;
    virtual QwtDoubleRect boundingRect() const;
};

class QWT_EXPORT QwtIntervalSeriesData: public QwtArraySeriesData<QwtIntervalSample>
{
public:
    QwtIntervalSeriesData(
        const QwtArray<QwtIntervalSample> & = QwtArray<QwtIntervalSample>());

    virtual QwtSeriesData<QwtIntervalSample> *copy() const;
    virtual QwtDoubleRect boundingRect() const;
};

class QWT_EXPORT QwtSetSeriesData: public QwtArraySeriesData<QwtSetSample>
{
public:
    QwtSetSeriesData(
        const QwtArray<QwtSetSample> & = QwtArray<QwtSetSample>());

    virtual QwtSeriesData<QwtSetSample> *copy() const;
    virtual QwtDoubleRect boundingRect() const;
};

/*!
  \brief Data class containing two QwtArray<double> objects.
 */

class QWT_EXPORT QwtPointArrayData: public QwtSeriesData<QwtDoublePoint>
{
public:
    QwtPointArrayData(const QwtArray<double> &x, const QwtArray<double> &y);
    QwtPointArrayData(const double *x, const double *y, size_t size);
    QwtPointArrayData &operator=(const QwtPointArrayData &);
    virtual QwtSeriesData<QwtDoublePoint> *copy() const;

    virtual QwtDoubleRect boundingRect() const;
    virtual size_t size() const;
    virtual QwtDoublePoint sample(size_t i) const;

    const QwtArray<double> &xData() const;
    const QwtArray<double> &yData() const;

private:
    QwtArray<double> d_x;
    QwtArray<double> d_y;
};

/*!
  \brief Data class containing two pointers to memory blocks of doubles.
 */
class QWT_EXPORT QwtCPointerData: public QwtSeriesData<QwtDoublePoint>
{
public:
    QwtCPointerData(const double *x, const double *y, size_t size);
    QwtCPointerData &operator=(const QwtCPointerData &);
    virtual QwtSeriesData<QwtDoublePoint> *copy() const;

    virtual QwtDoubleRect boundingRect() const;
    virtual size_t size() const;
    virtual QwtDoublePoint sample(size_t i) const;

    const double *xData() const;
    const double *yData() const;

private:
    const double *d_x;
    const double *d_y;
    size_t d_size;
};

QWT_EXPORT QwtDoubleRect qwtBoundingRect(
    const QwtSeriesData<QwtDoublePoint> &);
QWT_EXPORT QwtDoubleRect qwtBoundingRect(
    const QwtSeriesData<QwtIntervalSample> &);
QWT_EXPORT QwtDoubleRect qwtBoundingRect(
    const QwtSeriesData<QwtSetSample> &);

#if defined(QWT_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class QWT_EXPORT QwtArray<QwtDoublePoint>;
template class QWT_EXPORT QwtArray<QwtIntervalSample>;
template class QWT_EXPORT QwtArray<QwtSetSample>;
// MOC_SKIP_END
#endif

#endif // !QWT_SERIES_DATA_H
