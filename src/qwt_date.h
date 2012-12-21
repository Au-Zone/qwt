/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef _QWT_DATE_H
#define _QWT_DATE_H

#include <qwt_global.h>
#include <qdatetime.h>

/*!
  \brief A collection of methods around date/time values

  Qt offers convenient classes for dealing date/time values.
  As the coordinate system is based on doubles QwtDate offers
  methods to translate QDateTime to double and v.v.

  A double is interpreted as the number of milliseconds since
  1970-01-01T00:00:00 Universal Coordinated Time - also known
  as "The Epoch". As the significance of a double is below
  the available in QDateTime the translation is not bijective with
  rounding errors for dates very far from Epoch.

  While the range of the julian day in Qt4 is limited in the range
  between [0, MAX_INT], Qt5 stores it as qint64 offering a huge range
  of valid dates. As the significance of a double is below
  the available in QDateTime the translation is not bijective with
  rounding errors for dates very far from Epoch.

  An axis for a date/time interval is expected to be aligned
  and divided in time/date units like seconds, minutes, ...
  QwtDate offers several algorithms that are needed to
  calculate these axes.

  \sa QwtDateTimeScaleEngine, QwtTimeScaleDraw, QDate, QTime
*/
class QWT_EXPORT QwtDate
{
public:
    /*! 
      Classification of an time interval

      Time intervals needs to be classified to decide how to
      align and divide it.
     */
    enum IntervalType
    {
        //! The interval is related to miliseconds
        Millisecond,

        //! The interval is related to seconds
        Second,

        //! The interval is related to minutes
        Minute,

        //! The interval is related to hours
        Hour,

        //! The interval is related to days
        Day,

        //! The interval is related to weeks
        Week,

        //! The interval is related to months
        Month,

        //! The interval is related to years
        Year
    };

    enum
    {
        //! The julian day of "The Epoch"
        JulianDayForEpoch = 2440588
    };

    static QDate minDate();
    static QDate maxDate();

    static QDateTime toDateTime( double value, 
        Qt::TimeSpec = Qt::UTC );

    static double toDouble( const QDateTime & );

    static QDateTime ceil( const QDateTime &, IntervalType );
    static QDateTime floor( const QDateTime &, IntervalType );
};

#endif
