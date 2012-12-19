#ifndef _TIME_INTERVAL_H_
#define _TIME_INTERVAL_H_ 1

#include "timedate.h"
#include <qdatetime.h>

class TimeInterval
{
public:
    TimeInterval( double from, double to );
    TimeInterval( const QDateTime &, const QDateTime & );

    QDateTime minDate() const;
    QDateTime maxDate() const;

    TimeInterval rounded( TimeDate::IntervalType ) const;

    TimeInterval adjusted( double stepSize,
        TimeDate::IntervalType ) const;

    double width( TimeDate::IntervalType ) const;
    double roundedWidth( TimeDate::IntervalType ) const;

private:
    QDateTime d_minDate;
    QDateTime d_maxDate;
};

#endif
