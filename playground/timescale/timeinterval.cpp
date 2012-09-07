#include "timeinterval.h"
#include "timedate.h"
#include <qdebug.h>

static QDateTime qwtFloorDateToStep( const QDateTime &dt,
    int stepSize, TimeDate::IntervalType intervalType )
{
	// what about: (year == 1582 && month == 10 && day > 4 && day < 15) ??

    switch( intervalType )
    {
        case TimeDate::Millisecond:
        {
            const QTime t = dt.time();
            const int ms = ( t.msec() / stepSize ) * stepSize;
            return QDateTime( dt.date(),
                QTime( t.hour(), t.minute(), t.second(), ms ) );
        }
        case TimeDate::Second:
        {
            const QTime t = dt.time();
            const int s = ( t.second() / stepSize ) * stepSize;
            return QDateTime( dt.date(), QTime( t.hour(), t.minute(), s ) );
        }
        case TimeDate::Minute:
        {
            const QTime t = dt.time();
            const int m = ( t.minute() / stepSize ) * stepSize;
            return QDateTime( dt.date(), QTime( t.hour(), m, 0 ) );
        }
        case TimeDate::Hour:
        {
            const QTime t = dt.time();
            const int h = ( t.hour() / stepSize ) * stepSize;
            return QDateTime( dt.date(), QTime( h, 0, 0 ) );
        }
        case TimeDate::Day:
        {
#if 1
            // ????
            const int d = ( dt.date().dayOfYear() / stepSize ) * stepSize;

            QDate date( dt.date().year(), 1, 1 );
            date.addDays( d );
#endif

            return QDateTime( date );
        }
        case TimeDate::Week:
        {
            QDate date = qwtDateOfWeek0( dt.date().year() );

            const int numWeeks = date.daysTo( dt.date() ) / 7;
            date = date.addDays( ( numWeeks / stepSize ) * stepSize * 7 );

            return QDateTime( date );
        }
        case TimeDate::Month:
        {
            const int m = ( ( dt.date().month() - 1 ) / stepSize ) * stepSize;
            return QDateTime( QDate( dt.date().year(), m + 1, 1 ) );
        }
        case TimeDate::Year:
        {
            const int y = ( dt.date().year() / stepSize ) * stepSize;
            
            if ( y == 0 )
            {
                // there is no year 0 in the Julian calendar

                QDate d = QDate( stepSize, 1, 1 ).addYears( -stepSize );
                return QDateTime( d );
            }
            return QDateTime( QDate( y, 1, 1 ) );
        }
    }

    return dt;
}

TimeInterval::TimeInterval( 
        const QDateTime &from, const QDateTime &to ):
    d_minDate( from ),
    d_maxDate( to )
{
}

TimeInterval::TimeInterval( double from, double to ):
    d_minDate( qwtToDateTime( from ) ),
    d_maxDate( qwtToDateTime( to ) )
{
}

QDateTime TimeInterval::minDate() const
{
    return d_minDate;
}

QDateTime TimeInterval::maxDate() const
{
    return d_maxDate;
}

TimeInterval TimeInterval::rounded( 
    TimeDate::IntervalType intervalType ) const
{
    const QDateTime minDate = qwtFloorDate( d_minDate, intervalType );
    const QDateTime maxDate = qwtCeilDate( d_maxDate, intervalType );

    return TimeInterval( minDate, maxDate );
}

TimeInterval TimeInterval::adjusted( int stepSize,
    TimeDate::IntervalType intervalType ) const
{
    const QDateTime minDate = qwtFloorDateToStep( d_minDate,
        stepSize, intervalType );

    return TimeInterval( minDate, d_maxDate );
}

int TimeInterval::width( TimeDate::IntervalType intervalType ) const
{
    switch( intervalType )
    {
        case TimeDate::Millisecond:
        {
#if QT_VERSION >= 0x070000
            return d_minDate.msecsTo( d_maxDate );
#else
			int secsTo = d_minDate.secsTo( d_maxDate );
			int mesecs = d_maxDate.time().msec() -
				d_minDate.time().msec();

			return secsTo * 1000 + mesecs;
#endif
        }
        case TimeDate::Second:
        {
            return d_minDate.secsTo( d_maxDate );
        }
        case TimeDate::Minute:
        {
            return d_minDate.secsTo( d_maxDate ) / 60;
        }
        case TimeDate::Hour:
        {
            return d_minDate.secsTo( d_maxDate ) / 3600;
        }
        case TimeDate::Day:
        {
            return d_minDate.daysTo( d_maxDate );
        }
        case TimeDate::Week:
        {
            return d_minDate.daysTo( d_maxDate ) / 7;
        }
        case TimeDate::Month:
        {
            const int years = d_maxDate.date().year() - d_minDate.date().year();

            int months = d_maxDate.date().month() - d_minDate.date().month();
            if ( d_maxDate.date().day() < d_minDate.date().day() )
                months--;

            return 12 * years + months;
        }
        case TimeDate::Year:
        {
            int years = d_maxDate.date().year() - d_minDate.date().year();
            if ( d_maxDate.date().month() < d_minDate.date().month() )
                years--;

            return years;
        }
    }

    return 0;
}

int TimeInterval::roundedWidth( 
    TimeDate::IntervalType intervalType ) const
{
    return rounded( intervalType ).width( intervalType );
}

