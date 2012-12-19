#ifndef _TIME_DATE_H
#define _TIME_DATE_H

#include <qwt_global.h>
#include <qdatetime.h>

namespace TimeDate
{
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

    QWT_EXPORT double minJulianDay();
    QWT_EXPORT double maxJulianDay();

    QWT_EXPORT QDate minDate();
    QWT_EXPORT QDate maxDate();
};

extern QDateTime qwtToDateTime( double value );
extern double qwtFromDateTime( const QDateTime & );

extern QDateTime qwtCeilDate( const QDateTime &, 
    TimeDate::IntervalType );

extern QDateTime qwtFloorDate( const QDateTime &, 
    TimeDate::IntervalType );

extern QDate qwtDateOfWeek0( int year );

#endif
