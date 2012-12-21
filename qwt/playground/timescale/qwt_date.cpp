#include "qwt_date.h"
#include <qdebug.h>
#include <qlocale.h>
#include <limits>
#include <math.h>

#if QT_VERSION >= 0x050000
static const QwtDate::JulianDay minJulianDayD = Q_INT64_C( -784350574879 );
static const QwtDate::JulianDay maxJulianDayD = Q_INT64_C( 784354017364 );
#else
static const QwtDate::JulianDay minJulianDayD = 0.0;
static const QwtDate::JulianDay maxJulianDayD = std::numeric_limits<int>::max();
#endif


static inline Qt::DayOfWeek qwtFirstDayOfWeek()
{
#if QT_VERSION >= 0x040800
    return QLocale().firstDayOfWeek();
#else
    return Qt::Monday;
#endif
}

QDate QwtDate::toDate( int year, int month, int day )
{
    if ( year > 100000 )
    {
        const double jd = toJulianDay( year, month, day );
        if ( jd > maxJulianDayD )
        {
            qWarning() << "qwtToDate: overflow";
            return QDate();
        }

        return QDate::fromJulianDay( static_cast<QwtDate::JulianDay>( jd ) );
    }
    else
    {
        return QDate( year, month, day );
    }
}

double QwtDate::toJulianDay( int year, int month, int day )
{
    // code from QDate but using doubles to avoid overflows
    // for large values

    const int m1 = ( month - 14 ) / 12;
    const int m2 = ( 367 * ( month - 2 - 12 * m1 ) ) / 12;
    const double y1 = ::floor( ( 4900.0 + year + m1 ) / 100 );

    return ::floor( ( 1461.0 * ( year + 4800 + m1 ) ) / 4 ) + m2
            - ::floor( ( 3 * y1 ) / 4 ) + day - 32075;
}

QDateTime QwtDate::toTimeSpec( const QDateTime &dt, Qt::TimeSpec spec )
{
    if ( dt.timeSpec() == spec )
        return dt;

    const qint64 jd = dt.date().toJulianDay();
    if ( jd < 0 || jd >= INT_MAX )
    {
        // the conversion between local time and UTC
        // is internally limited. To avoid
        // overflows we simply ignore the difference
        // for those dates

        QDateTime dt2 = dt;
        dt2.setTimeSpec( spec );
        return dt2;
    }

    return dt.toTimeSpec( spec );
}

QDateTime QwtDate::toDateTime( double value, Qt::TimeSpec timeSpec )
{
    const int msecsPerDay = 86400000;

    const double days = static_cast<qint64>( ::floor( value / msecsPerDay ) );

    const double jd = QwtDate::JulianDayForEpoch + days;
    if ( ( jd > maxJulianDayD ) || ( jd < minJulianDayD ) )
    {
        qWarning() << "QwtDate::toDateTime: overflow";
        return QDateTime();
    }

    const QDate d = QDate::fromJulianDay( static_cast<QwtDate::JulianDay>( jd ) );

    const int msecs = static_cast<int>( value - days * msecsPerDay );

    static const QTime timeNull( 0, 0, 0, 0 );

    QDateTime dt( d, timeNull.addMSecs( msecs ), Qt::UTC );
    if ( timeSpec != Qt::UTC )
        dt = toTimeSpec( dt, timeSpec );

    return dt;
}

double QwtDate::toDouble( const QDateTime &dateTime )
{
    const int msecsPerDay = 86400000;

    const QDateTime dt = toTimeSpec( dateTime, Qt::UTC );

    const double days = dt.date().toJulianDay() - QwtDate::JulianDayForEpoch;

    const QTime time = dt.time();
    const double secs = 3600.0 * time.hour() + 60.0 * time.minute() + time.second();

    return days * msecsPerDay + time.msec() + 1000.0 * secs;
}

QDateTime QwtDate::ceil( const QDateTime &dateTime, IntervalType type )
{
    if ( dateTime.date() >= QwtDate::maxDate() )
        return dateTime;

    QDateTime dt;

    switch ( type )
    {
        case QwtDate::Millisecond:
        {
            dt = dateTime;
            break;
        }
        case QwtDate::Second:
        {
            dt.setDate( dateTime.date() );

            const QTime t = dateTime.time();
            dt.setTime( QTime( t.hour(), t.minute(), t.second(), 0 ) );

            if ( dt < dateTime )
                dt.addSecs( 1 );

            break;
        }
        case QwtDate::Minute:
        {
            dt.setDate( dateTime.date() );

            const QTime t = dateTime.time();
            dt.setTime( QTime( t.hour(), t.minute(), 0, 0 ) );

            if ( dt < dateTime )
                dt.addSecs( 60 );

            break;
        }
        case QwtDate::Hour:
        {
            dt.setDate( dateTime.date() );
            
            const QTime t = dateTime.time();
            dt.setTime( QTime( t.hour(), 0, 0, 0 ) );

            if ( dt < dateTime )
                dt.addSecs( 3600 );

            break;
        }
        case QwtDate::Day:
        {
            dt = QDateTime( dateTime.date() );
            if ( dt < dateTime )
                dt = dt.addDays( 1 );

            break;
        }
        case QwtDate::Week:
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
        case QwtDate::Month:
        {
            dt = QDateTime( toDate( dateTime.date().year(), dateTime.date().month(), 1 ) );
            if ( dt < dateTime )
                dt.addMonths( 1 );

            break;
        }
        case QwtDate::Year:
        {
            const QDate d = dateTime.date();

            int year = d.year();
            if ( d.month() > 1 || d.day() > 1 || !dateTime.time().isNull() )
                year++;

            if ( year == 0 )
                year++; // there is no year 0

            dt = QDateTime( toDate( year, 1, 1 ) );
            break;
        }
    }

    return dt;
}

QDateTime QwtDate::floor( const QDateTime &dateTime, IntervalType type )
{
    if ( dateTime.date() <= QwtDate::minDate() )
        return dateTime;

    QDateTime dt;

    switch ( type )
    {
        case QwtDate::Millisecond:
        {
            dt = dateTime;
            break;
        }
        case QwtDate::Second:
        {
            dt.setDate( dateTime.date() );

            const QTime t = dateTime.time();
            dt.setTime( QTime( t.hour(), t.minute(), t.second(), 0 ) );

            break;
        }
        case QwtDate::Minute:
        {
            dt.setDate( dateTime.date() );

            const QTime t = dateTime.time();
            dt.setTime( QTime( t.hour(), t.minute(), 0, 0 ) );

            break;
        }
        case QwtDate::Hour:
        {
            dt.setDate( dateTime.date() );

            const QTime t = dateTime.time();
            dt.setTime( QTime( t.hour(), 0, 0, 0 ) );
            break;
        }
        case QwtDate::Day:
        {
            dt = QDateTime( dateTime.date() );
            break;
        }
        case QwtDate::Week:
        {
            dt = QDateTime( dateTime.date() );

            int days = dt.date().dayOfWeek() - qwtFirstDayOfWeek();
            if ( days < 0 )
                days += 7;

            dt = dt.addDays( -days );

            break;
        }
        case QwtDate::Month:
        {
            dt = QDateTime( toDate( dateTime.date().year(), dateTime.date().month(), 1 ) );
            break;
        }
        case QwtDate::Year:
        {
            dt = QDateTime( toDate( dateTime.date().year(), 1, 1 ) );
            break;
        }
    }

    return dt;
}

QDate QwtDate::dateOfWeek0( int year )
{
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

QDate QwtDate::minDate()
{
    static QDate date;
    if ( !date.isValid() )
        date = QDate::fromJulianDay( minJulianDayD );

    return date;
}

QDate QwtDate::maxDate()
{
    static QDate date;
    if ( !date.isValid() )
        date = QDate::fromJulianDay( maxJulianDayD );

    return date;
}
