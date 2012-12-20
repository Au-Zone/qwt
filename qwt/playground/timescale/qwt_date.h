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

class QWT_EXPORT QwtDate
{
public:
    enum IntervalType
    {
        Millisecond,
        Second,
        Minute,
        Hour,
        Day,
        Week,
        Month,
        Year
    };

#if QT_VERSION >= 0x050000
	typedef qint64 JulianDay;
#else
	typedef int JulianDay;
#endif

    static double minJulianDay();
    static double maxJulianDay();

    static QDate minDate();
    static QDate maxDate();

    static double msecsOfType( IntervalType );

	static QDateTime toDateTime( double value );
	static double toDouble( const QDateTime & );

	static QDateTime ceil( const QDateTime &, IntervalType );
	static QDateTime floor( const QDateTime &, IntervalType );

	static QDate dateOfWeek0( int year );
};

#endif
