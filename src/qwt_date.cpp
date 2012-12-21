#include "qwt_date.h"
#include <qdebug.h>
#include <qlocale.h>
#include <limits>
#include <math.h>

#if QT_VERSION >= 0x050000

typedef qint64 QwtJulianDay;
static const QwtJulianDay minJulianDayD = Q_INT64_C( -784350574879 );
static const QwtJulianDay maxJulianDayD = Q_INT64_C( 784354017364 );

#else

// QDate stores the julian day as unsigned int, but
// but it is QDate::fromJulianDay( int ). That's why
// we have the range [ 1, INT_MAX ]
typedef int QwtJulianDay;
static const QwtJulianDay minJulianDayD = 1;
static const QwtJulianDay maxJulianDayD = std::numeric_limits<int>::max();

#endif

static inline Qt::DayOfWeek qwtFirstDayOfWeek()
{
#if QT_VERSION >= 0x040800
    return QLocale().firstDayOfWeek();
#else
    return Qt::Monday;
#endif
}

static inline QDateTime qwtToTimeSpec( const QDateTime &dt, Qt::TimeSpec spec )
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

static inline qint64 qwtFloorDiv64( qint64 a, int b )
{
    if ( a < 0 )
        a -= b - 1;

    return a / b;
}

static inline qint64 qwtFloorDiv( int a, int b )
{
    if ( a < 0 )
        a -= b - 1;
        
    return a / b;
}   

static inline QDate qwtToDate( int year, int month = 1, int day = 1 )
{
#if QT_VERSION >= 0x050000
    return QDate( year, month, day );
#else
    if ( year > 100000 )
    {
        // code from QDate but using doubles to avoid overflows
        // for large values

        const int m1 = ( month - 14 ) / 12;
        const int m2 = ( 367 * ( month - 2 - 12 * m1 ) ) / 12;
        const double y1 = ::floor( ( 4900.0 + year + m1 ) / 100 );

        const double jd = ::floor( ( 1461.0 * ( year + 4800 + m1 ) ) / 4 ) + m2
            - ::floor( ( 3 * y1 ) / 4 ) + day - 32075;

        if ( jd > maxJulianDayD )
        {
            qWarning() << "qwtToDate: overflow";
            return QDate();
        }

        return QDate::fromJulianDay( static_cast<QwtJulianDay>( jd ) );
    }
    else
    {
        return QDate( year, month, day );
    }
#endif
}

/*!
  Translate from double to QDateTime

  \param value Number of milliseconds since the epoch, 
               1970-01-01T00:00:00 UTC
  \param timeSpec Time specification
  \return datetime value

  \sa toDouble()
 */
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

    const QDate d = QDate::fromJulianDay( static_cast<QwtJulianDay>( jd ) );

    const int msecs = static_cast<int>( value - days * msecsPerDay );

    static const QTime timeNull( 0, 0, 0, 0 );

    QDateTime dt( d, timeNull.addMSecs( msecs ), Qt::UTC );
    if ( timeSpec != Qt::UTC )
        dt = qwtToTimeSpec( dt, timeSpec );

    return dt;
}

/*!
  Translate from QDateTime to double

  \param dateTime Datetime value
  \return Number of milliseconds since 1970-01-01T00:00:00 UTC has passed.

  \sa toDateTime()
  \warning For values very far below or above 1970-01-01 UTC rounding errors
           will happen due to the limited significance of a double.
 */
double QwtDate::toDouble( const QDateTime &dateTime )
{
    const int msecsPerDay = 86400000;

    const QDateTime dt = qwtToTimeSpec( dateTime, Qt::UTC );

    const double days = dt.date().toJulianDay() - QwtDate::JulianDayForEpoch;

    const QTime time = dt.time();
    const double secs = 3600.0 * time.hour() + 60.0 * time.minute() + time.second();

    return days * msecsPerDay + time.msec() + 1000.0 * secs;
}

/*!
  Ceil a datetime according the interval type

  \param dateTime Datetime value
  \param intervalType Interval type, how to ceil. 
                      F.e. when intervalType = QwtDate::Months, the result
                      will be ceiled to the next beginning of a month
  \sa floor()
 */
QDateTime QwtDate::ceil( const QDateTime &dateTime, IntervalType intervalType )
{
    if ( dateTime.date() >= QwtDate::maxDate() )
        return dateTime;

    QDateTime dt;
    dt.setTimeSpec( dateTime.timeSpec() );

    switch ( intervalType )
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
            dt = QDateTime( qwtToDate( dateTime.date().year(), dateTime.date().month() ) );
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

            dt = QDateTime( qwtToDate( year ) );
            break;
        }
    }

    return dt;
}

/*!
  Floor a datetime according the interval type

  \param dateTime Datetime value
  \param intervalType Interval type, how to ceil. 
                      F.e. when intervalType = QwtDate::Months, the result
                      will be ceiled to the next beginning of a month
  \sa floor()
 */
QDateTime QwtDate::floor( const QDateTime &dateTime, IntervalType type )
{
    if ( dateTime.date() <= QwtDate::minDate() )
        return dateTime;

    QDateTime dt;
    dt.setTimeSpec( dateTime.timeSpec() );

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
            dt = QDateTime( qwtToDate( dateTime.date().year(), dateTime.date().month() ) );
            break;
        }
        case QwtDate::Year:
        {
            dt = QDateTime( qwtToDate( dateTime.date().year() ) );
            break;
        }
    }

    return dt;
}

/*!
  \return Minimum for the supported date range, that depends
          on how QDate stores the julian day internally
  \sa maxDate()
  \note The minimum differs between Qt4 and Qt5
 */
QDate QwtDate::minDate()
{
    static QDate date;
    if ( !date.isValid() )
        date = QDate::fromJulianDay( minJulianDayD );

    return date;
}

/*!
  \return Maximum of the supported date range, that depends
          on how QDate stores the julian day internally
  \sa minDate()
  \note The maximum differs between Qt4 and Qt5
 */
QDate QwtDate::maxDate()
{
    static QDate date;
    if ( !date.isValid() )
        date = QDate::fromJulianDay( maxJulianDayD );

    return date;
}
