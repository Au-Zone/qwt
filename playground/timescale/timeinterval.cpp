#include "timeinterval.h"
#include "qwt_date.h"
#include <qmath.h>
#if QT_VERSION >= 0x040800
#include <qlocale.h>
#endif
#include <qdebug.h>

static inline double qwtAlignValue( 
    double value, double stepSize, bool up )
{
    if ( up )
        return ::ceil( value / stepSize ) * stepSize;
    else
        return ::floor( value / stepSize ) * stepSize;
}

static inline double qwtCeilValue( double value, double stepSize )
{
    return ::ceil( value / stepSize ) * stepSize;
}

static QDateTime qwtAlignDate( const QDateTime &dt,
    double stepSize, QwtDate::IntervalType intervalType, bool up )
{
    // what about: (year == 1582 && month == 10 && day > 4 && day < 15) ??

    switch( intervalType )
    {
        case QwtDate::Millisecond:
        {
            const QTime t = dt.time();
            const int ms = qwtAlignValue( t.msec(), stepSize, up ) ;
            return QDateTime( dt.date(),
                QTime( t.hour(), t.minute(), t.second(), ms ) );
        }
        case QwtDate::Second:
        {
            const QTime t = dt.time();
            const int s = qwtAlignValue( t.second(), stepSize, up );
            return QDateTime( dt.date(), QTime( t.hour(), t.minute(), s ) );
        }
        case QwtDate::Minute:
        {
            const QTime t = dt.time();
            const int m = qwtAlignValue( t.minute(), stepSize, up );
            return QDateTime( dt.date(), QTime( t.hour(), m, 0 ) );
        }
        case QwtDate::Hour:
        {
            const QTime t = dt.time();
            const int h = qwtAlignValue( t.hour(), stepSize, up );
            return QDateTime( dt.date(), QTime( h, 0, 0 ) );
        }
        case QwtDate::Day:
        {
#if 1
            // ????
            const double d = qwtAlignValue( dt.date().dayOfYear(), stepSize, up );

            QDate date( dt.date().year(), 1, 1 );
            date.addDays( static_cast<int>( d ) );
#endif

            return QDateTime( date );
        }
        case QwtDate::Week:
        {
            QwtDate::Week0Type type = QwtDate::FirstThursday;
            if ( QLocale().country() == QLocale::UnitedStates )
                type = QwtDate::FirstDay;
            
            QDate date = QwtDate::dateOfWeek0( dt.date().year(), type );

            const int numWeeks = date.daysTo( dt.date() ) / 7;
            date = date.addDays( qwtAlignValue( numWeeks, stepSize, up ) * 7 );

            return QDateTime( date );
        }
        case QwtDate::Month:
        {
            const double m = qwtAlignValue( dt.date().month() - 1, stepSize, up );
            return QDateTime( QDate( dt.date().year(), static_cast<int>( m + 1 ), 1 ) );
        }
        case QwtDate::Year:
        {
            const int y = static_cast<int>( qwtAlignValue( dt.date().year(), stepSize, up ) );

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

QDateTime TimeInterval::minDate() const
{
    return d_minDate;
}

QDateTime TimeInterval::maxDate() const
{
    return d_maxDate;
}

TimeInterval TimeInterval::rounded( 
    QwtDate::IntervalType intervalType ) const
{
    const QDateTime minDate = QwtDate::floor( d_minDate, intervalType );
    const QDateTime maxDate = QwtDate::ceil( d_maxDate, intervalType );

    return TimeInterval( minDate, maxDate );
}

TimeInterval TimeInterval::adjusted( double stepSize,
    QwtDate::IntervalType intervalType ) const
{
    QDateTime minDate = qwtAlignDate( d_minDate,
        stepSize, intervalType, false );
    if ( !minDate.isValid() )
    {
        minDate = qwtAlignDate( d_minDate,
            stepSize, intervalType, true );
    }

    return TimeInterval( minDate, d_maxDate );
}

double TimeInterval::width( QwtDate::IntervalType intervalType ) const
{
    switch( intervalType )
    {
        case QwtDate::Millisecond:
        {
            const double secsTo = d_minDate.secsTo( d_maxDate );
            const double msecs = d_maxDate.time().msec() -
                d_minDate.time().msec();

            return secsTo * 1000 + msecs;
        }
        case QwtDate::Second:
        {
            return d_minDate.secsTo( d_maxDate );
        }
        case QwtDate::Minute:
        {
            const double secsTo = d_minDate.secsTo( d_maxDate );
            return ::floor( secsTo / 60 );
        }
        case QwtDate::Hour:
        {
            const double secsTo = d_minDate.secsTo( d_maxDate );
            return ::floor( secsTo / 3600 );
        }
        case QwtDate::Day:
        {
            return d_minDate.daysTo( d_maxDate );
        }
        case QwtDate::Week:
        {
            return ::floor( d_minDate.daysTo( d_maxDate ) / 7.0 );
        }
        case QwtDate::Month:
        {
            const double years = double( d_maxDate.date().year() ) - d_minDate.date().year();

            int months = d_maxDate.date().month() - d_minDate.date().month();
            if ( d_maxDate.date().day() < d_minDate.date().day() )
                months--;

            return years * 12 + months;
        }
        case QwtDate::Year:
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
    QwtDate::IntervalType intervalType ) const
{
    return rounded( intervalType ).width( intervalType );
}

