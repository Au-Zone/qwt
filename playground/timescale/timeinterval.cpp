#include "timeinterval.h"
#include "timedate.h"
#include <qmath.h>
#include <qdebug.h>

static inline double qwtFloorValue( double value )
{
	return ::floor( value );
}

static inline double qwtAlignValue( double value, double stepSize )
{
	return qwtFloorValue( value / stepSize ) * stepSize;
}

static QDateTime qwtFloorDateToStep( const QDateTime &dt,
    double stepSize, TimeDate::IntervalType intervalType )
{
    // what about: (year == 1582 && month == 10 && day > 4 && day < 15) ??

    switch( intervalType )
    {
        case TimeDate::Millisecond:
        {
            const QTime t = dt.time();
            const int ms = qwtAlignValue( t.msec(), stepSize ) ;
            return QDateTime( dt.date(),
                QTime( t.hour(), t.minute(), t.second(), ms ) );
        }
        case TimeDate::Second:
        {
            const QTime t = dt.time();
            const int s = qwtAlignValue( t.second(), stepSize );
            return QDateTime( dt.date(), QTime( t.hour(), t.minute(), s ) );
        }
        case TimeDate::Minute:
        {
            const QTime t = dt.time();
            const int m = qwtAlignValue( t.minute(), stepSize );
            return QDateTime( dt.date(), QTime( t.hour(), m, 0 ) );
        }
        case TimeDate::Hour:
        {
            const QTime t = dt.time();
            const int h = qwtAlignValue( t.hour(), stepSize );
            return QDateTime( dt.date(), QTime( h, 0, 0 ) );
        }
        case TimeDate::Day:
        {
#if 1
            // ????
            const double d = qwtAlignValue( dt.date().dayOfYear(), stepSize );

            QDate date( dt.date().year(), 1, 1 );
            date.addDays( static_cast<int>( d ) );
#endif

            return QDateTime( date );
        }
        case TimeDate::Week:
        {
            QDate date = qwtDateOfWeek0( dt.date().year() );

            const int numWeeks = date.daysTo( dt.date() ) / 7;
            date = date.addDays( qwtAlignValue( numWeeks, stepSize ) * 7 );

            return QDateTime( date );
        }
        case TimeDate::Month:
        {
            const double m = qwtAlignValue( dt.date().month() - 1, stepSize );
            return QDateTime( QDate( dt.date().year(), static_cast<int>( m + 1 ), 1 ) );
        }
        case TimeDate::Year:
        {
            const int y = qwtAlignValue( dt.date().year(), stepSize );
            
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

TimeInterval TimeInterval::adjusted( double stepSize,
    TimeDate::IntervalType intervalType ) const
{
    const QDateTime minDate = qwtFloorDateToStep( d_minDate,
        stepSize, intervalType );

    return TimeInterval( minDate, d_maxDate );
}

double TimeInterval::width( TimeDate::IntervalType intervalType ) const
{
    switch( intervalType )
    {
        case TimeDate::Millisecond:
        {
            const double secsTo = d_minDate.secsTo( d_maxDate );
            const double msecs = d_maxDate.time().msec() -
                d_minDate.time().msec();

            return secsTo * 1000 + msecs;
        }
        case TimeDate::Second:
        {
            return d_minDate.secsTo( d_maxDate );
        }
        case TimeDate::Minute:
        {
			const double secsTo = d_minDate.secsTo( d_maxDate );
            return qwtFloorValue( secsTo / 60 );
        }
        case TimeDate::Hour:
        {
			const double secsTo = d_minDate.secsTo( d_maxDate );
            return qwtFloorValue( secsTo / 3600 );
        }
        case TimeDate::Day:
        {
            return d_minDate.daysTo( d_maxDate );
        }
        case TimeDate::Week:
        {
            return qwtFloorValue( d_minDate.daysTo( d_maxDate ) / 7.0 );
        }
        case TimeDate::Month:
        {
            const double years = double( d_maxDate.date().year() ) - d_minDate.date().year();

            int months = d_maxDate.date().month() - d_minDate.date().month();
            if ( d_maxDate.date().day() < d_minDate.date().day() )
                months--;

            return years * 12 + months;
        }
        case TimeDate::Year:
        {
            double years = double( d_maxDate.date().year() ) - d_minDate.date().year();
            if ( d_maxDate.date().month() < d_minDate.date().month() )
                years -= 1.0;

            return years;
        }
    }

    return 0.0;
}

double TimeInterval::roundedWidth( 
    TimeDate::IntervalType intervalType ) const
{
    return rounded( intervalType ).width( intervalType );
}

