#include "timedate.h"
#include <qlocale.h>
#include <qdebug.h>
#include <limits.h>
#include <math.h>

static const qint64 s_julianDay0 = QDate(1970, 1, 1).toJulianDay();
static const int s_msecsPerDay = 86400000;

static inline QDateTime qwtToTimeSpec( const QDateTime &dt, Qt::TimeSpec spec )
{
	if ( dt.timeSpec() == spec )
		return dt;

	const qint64 jd = dt.date().toJulianDay();

	if ( jd < 0 || jd >= INT_MAX )
	{
		QDateTime dt2 = dt;
		dt2.setTimeSpec( spec );
		return dt2;
	}

	return dt.toTimeSpec( spec );
}

static inline double qwtDateToDays( const QDate &date )
{
    return static_cast<double>( date.toJulianDay() ) - s_julianDay0;
}

static inline QDate qwtDaysToDate( double days )
{
	const double jd = s_julianDay0 + days;
	if ( jd > TimeDate::maxJulianDay() || jd < TimeDate::minJulianDay() )
		return QDate();

    return QDate::fromJulianDay( static_cast<qint64>( jd ) );
}

static Qt::DayOfWeek qwtFirstDayOfWeek()
{
    QLocale locale;
#if QT_VERSION >= 0x040800
    return locale.firstDayOfWeek();
#else
    return Qt::Monday;
#endif
}

static inline double qwtToJulianDay( int year, int month, int day )
{
    // code from QDate but using doubles to avoid overflows
    // for large values

    const int m1 = ( month - 14 ) / 12;
    const int m2 = ( 367 * ( month - 2 - 12 * m1 ) ) / 12;
    const double y1 = ::floor( ( 4900.0 + year + m1 ) / 100 );

    return ::floor( ( 1461.0 * ( year + 4800 + m1 ) ) / 4 ) + m2
            - ::floor( ( 3 * y1 ) / 4 ) + day - 32075;
}

static inline QDate qwtToDate( int year, int month, int day )
{
    if ( year > 100000 )
    {
        const double jd = qwtToJulianDay( year, month, day );
		if ( jd < TimeDate::minJulianDay() || jd > TimeDate::maxJulianDay() )
        {
            qWarning() << "qwtToDate: overflow";
            return QDate();
        }

#if QT_VERSION >= 0x050000
        return QDate::fromJulianDay( static_cast<qint64>( jd ) );
#else
        return QDate::fromJulianDay( static_cast<int>( jd ) );
#endif
    }
    else
    {
        return QDate( year, month, day );
    }
}

QDateTime qwtToDateTime( double value )
{
    const double days = value / s_msecsPerDay;

    const QDate d = qwtDaysToDate( days );
	if ( !d.isValid() )
	{
		qWarning() << "qwtToDateTime: value out of range: " << value;
		return QDateTime();
	}

    const int msecs = static_cast<int>( value - days * s_msecsPerDay );
    const QTime t = QTime().addMSecs( msecs );

    return qwtToTimeSpec( QDateTime( d, t, Qt::UTC ), Qt::LocalTime );
}

double qwtFromDateTime( const QDateTime &dateTime )
{
	const QDateTime dt = qwtToTimeSpec( dateTime, Qt::UTC );

    const int msecs = QTime( 0, 0, 0 ).msecsTo( dt.time() );
    const double days = qwtDateToDays( dt.date() );

    return days * s_msecsPerDay + msecs;
}

QDateTime qwtCeilDate( const QDateTime &dateTime, 
    TimeDate::IntervalType type )
{
	if ( dateTime.date() >= TimeDate::maxDate() )
		return dateTime;

    QDateTime dt;

    switch ( type )
    {
        case TimeDate::Millisecond:
        {
            dt = dateTime;
            break;
        }
        case TimeDate::Second:
        {
            dt.setDate( dateTime.date() );

            const QTime t = dateTime.time();
            dt.setTime( QTime( t.hour(), t.minute(), t.second(), 0 ) );

            if ( dt < dateTime )
                dt.addSecs( 1 );

            break;
        }
        case TimeDate::Minute:
        {
            dt.setDate( dateTime.date() );

            const QTime t = dateTime.time();
            dt.setTime( QTime( t.hour(), t.minute(), 0, 0 ) );

            if ( dt < dateTime )
                dt.addSecs( 60 );

            break;
        }
        case TimeDate::Hour:
        {
            dt.setDate( dateTime.date() );
            
            const QTime t = dateTime.time();
            dt.setTime( QTime( t.hour(), 0, 0, 0 ) );

            if ( dt < dateTime )
                dt.addSecs( 3600 );

            break;
        }
        case TimeDate::Day:
        {
            dt = QDateTime( dateTime.date() );
            if ( dt < dateTime )
                dt = dt.addDays( 1 );

            break;
        }
        case TimeDate::Week:
        {
            dt = QDateTime( dateTime.date() );
            if ( dt < dateTime )
                dt = dt.addDays( 1 );

            int days = qwtFirstDayOfWeek() - dt.date().dayOfWeek();
            if ( days < 0 )
                days += 7;

            dt = dt.addDays( days );

            break;
        }
        case TimeDate::Month:
        {
            dt = QDateTime( qwtToDate( dateTime.date().year(), dateTime.date().month(), 1 ) );
            if ( dt < dateTime )
                dt.addMonths( 1 );

            break;
        }
        case TimeDate::Year:
        {
            const QDate d = dateTime.date();

            int year = d.year();
            if ( d.month() > 1 || d.day() > 1 || !dateTime.time().isNull() )
                year++;

            if ( year == 0 )
                year++; // there is no year 0

            dt = QDateTime( qwtToDate( year, 1, 1 ) );
            break;
        }
    }

    return dt;
}

QDateTime qwtFloorDate( const QDateTime &dateTime, TimeDate::IntervalType type )
{
	if ( dateTime.date() <= TimeDate::minDate() )
		return dateTime;

    QDateTime dt;

    switch ( type )
    {
        case TimeDate::Millisecond:
        {
            dt = dateTime;
            break;
        }
        case TimeDate::Second:
        {
            dt.setDate( dateTime.date() );

            const QTime t = dateTime.time();
            dt.setTime( QTime( t.hour(), t.minute(), t.second(), 0 ) );

            break;
        }
        case TimeDate::Minute:
        {
            dt.setDate( dateTime.date() );

            const QTime t = dateTime.time();
            dt.setTime( QTime( t.hour(), t.minute(), 0, 0 ) );

            break;
        }
        case TimeDate::Hour:
        {
            dt.setDate( dateTime.date() );

            const QTime t = dateTime.time();
            dt.setTime( QTime( t.hour(), 0, 0, 0 ) );
            break;
        }
        case TimeDate::Day:
        {
            dt = QDateTime( dateTime.date() );
            break;
        }
        case TimeDate::Week:
        {
            dt = QDateTime( dateTime.date() );

            int days = dt.date().dayOfWeek() - qwtFirstDayOfWeek();
            if ( days < 0 )
                days += 7;

            dt = dt.addDays( -days );

            break;
        }
        case TimeDate::Month:
        {
            dt = QDateTime( qwtToDate( dateTime.date().year(), dateTime.date().month(), 1 ) );
            break;
        }
        case TimeDate::Year:
        {
            dt = QDateTime( qwtToDate( dateTime.date().year(), 1, 1 ) );
            break;
        }
    }

    return dt;
}

QDate qwtDateOfWeek0( int year )
{
    const QLocale locale;

    QDate dt0( year, 1, 1 );

    // floor to the first day of the week
    int days = dt0.dayOfWeek() - qwtFirstDayOfWeek();
    if ( days < 0 )
        days += 7;

    dt0 = dt0.addDays( -days );

    if ( QLocale().country() != QLocale::UnitedStates )
    {
        // according to ISO 8601 the first week is defined
        // by the first thursday. 

        int d = Qt::Thursday - qwtFirstDayOfWeek();
        if ( d < 0 )
            d += 7;

        if ( dt0.addDays( d ).year() < year )
            dt0 = dt0.addDays( 7 );
    }

    return dt0;
}

double TimeDate::minJulianDay()
{
#if QT_VERSION >= 0x050000
	// Q_INT64_C(-784350574879) )
	return -784350574879.0;
#else
	return 0.0;
#endif

}

double TimeDate::maxJulianDay()
{
#if QT_VERSION >= 0x050000
	return 784354017364.0;
#else
	return UINT_MAX;
#endif
}

QDate TimeDate::minDate()
{
	static QDate dt;
	if ( !dt.isValid() )
	{
		const qint64 jd = static_cast<qint64>( minJulianDay() );
		dt = QDate::fromJulianDay( jd );
	}

	return dt;
}

QDate TimeDate::maxDate()
{
    static QDate dt;
    if ( !dt.isValid() )
    {
        const qint64 jd = static_cast<qint64>( maxJulianDay() );
        dt = QDate::fromJulianDay( jd );
    }

    return dt;
}

