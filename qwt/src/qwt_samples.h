/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SAMPLES_H
#define QWT_SAMPLES_H 1

#include "qwt_global.h"
#include "qwt_interval.h"
#include "qwt_point_3d.h"
#include "qwt_point_polar.h"
#include <qvector.h>
#include <qrect.h>

//! \brief A sample of the types (x1-x2, y) or (x, y1-y2)
class QWT_EXPORT QwtIntervalSample
{
public:
    QwtIntervalSample();
    QwtIntervalSample( double, const QwtInterval & );
    QwtIntervalSample( double value, double min, double max );

    bool operator==( const QwtIntervalSample & ) const;
    bool operator!=( const QwtIntervalSample & ) const;

    //! Value
    double value;

    //! Interval
    QwtInterval interval;
};

/*!
  Constructor
  The value is set to 0.0, the interval is invalid
*/
inline QwtIntervalSample::QwtIntervalSample():
    value( 0.0 )
{
}

//! Constructor
inline QwtIntervalSample::QwtIntervalSample(
        double v, const QwtInterval &intv ):
    value( v ),
    interval( intv )
{
}

//! Constructor
inline QwtIntervalSample::QwtIntervalSample(
        double v, double min, double max ):
    value( v ),
    interval( min, max )
{
}

//! Compare operator
inline bool QwtIntervalSample::operator==(
    const QwtIntervalSample &other ) const
{
    return value == other.value && interval == other.interval;
}

//! Compare operator
inline bool QwtIntervalSample::operator!=(
    const QwtIntervalSample &other ) const
{
    return !( *this == other );
}

//! \brief A sample of the types (x1...xn, y) or (x, y1..yn)
class QWT_EXPORT QwtSetSample
{
public:
    QwtSetSample();
    bool operator==( const QwtSetSample &other ) const;
    bool operator!=( const QwtSetSample &other ) const;

    //! value
    double value;

    //! Vector of values associated to value
    QVector<double> set;
};

/*!
  Constructor
  The value is set to 0.0
*/
inline QwtSetSample::QwtSetSample():
    value( 0.0 )
{
}

//! Compare operator
inline bool QwtSetSample::operator==( const QwtSetSample &other ) const
{
    return value == other.value && set == other.set;
}

//! Compare operator
inline bool QwtSetSample::operator!=( const QwtSetSample &other ) const
{
    return !( *this == other );
}

/*!
   \brief Open-High-Low-Close sample used in financial charts

   In financial charts the movement of a price in a time interval is often
   represented by the opening/closing prices and the lowest/highest prices
   in this interval.

   \sa QwtTradingChartData
*/
class QWT_EXPORT QwtOHLCSample
{
public:
    QwtOHLCSample( double time = 0.0, 
        double open = 0.0, double high = 0.0,
        double low = 0.0, double close = 0.0 );

    QwtInterval boundingInterval() const;

    bool isValid() const;

    //! Time of the sample, often a number representing a specific day
    double time;

    //! Opening price
    double open;

    //! Highest price
    double high;

    //! Lowest price
    double low;

    //! Closing price
    double close;
};


/*!
  Constructor
*/
inline QwtOHLCSample::QwtOHLCSample( double t,
        double o, double h, double l, double c):
    time( t ),
    open( o ),
    high( h ),
    low( l ),
    close( c )
{
}

inline bool QwtOHLCSample::isValid() const
{
    return ( low <= high )
        && ( open >= low )
        && ( open <= high )
        && ( close >= low )
        && ( close <= high );
}

inline QwtInterval QwtOHLCSample::boundingInterval() const
{
    double minY = open;
    minY = qMin( minY, high );
    minY = qMin( minY, low );
    minY = qMin( minY, close );

    double maxY = open;
    maxY = qMax( maxY, high );
    maxY = qMax( maxY, low );
    maxY = qMax( maxY, close );

    return QwtInterval( minY, maxY );
}

#endif
