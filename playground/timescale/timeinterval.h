#ifndef _TIME_INTERVAL_H_
#define _TIME_INTERVAL_H_ 1

#include "qwt_date.h"
#include <qdatetime.h>

class TimeInterval
{
public:
    TimeInterval( double from, double to );
    TimeInterval( const QDateTime &, const QDateTime & );

    QDateTime minDate() const;
    QDateTime maxDate() const;

    TimeInterval rounded( QwtDate::IntervalType ) const;

    TimeInterval adjusted( double stepSize,
        QwtDate::IntervalType ) const;

    double width( QwtDate::IntervalType ) const;
    double roundedWidth( QwtDate::IntervalType ) const;

private:
    QDateTime d_minDate;
    QDateTime d_maxDate;
};

#endif
