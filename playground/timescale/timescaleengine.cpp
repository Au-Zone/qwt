#include "timescaleengine.h"
#include <qwt_math.h>
#include <qwt_transform.h>
#include <qdatetime.h>
#include <qdebug.h>

#define DEBUG_ENGINE 0

static inline int qwtAlignValue(
    double value, double stepSize, bool up )
{
    double d = value / stepSize;
    d = up ? ceil( d ) : floor( d );

    return static_cast<int>( d * stepSize );
}

static QDateTime qwtAlignDate( const QDateTime &dateTime,
    double stepSize, QwtDate::IntervalType intervalType, bool up )
{
    // what about: (year == 1582 && month == 10 && day > 4 && day < 15) ??

    QDateTime dt( dateTime.date(), QTime( 0, 0, 0 ), dateTime.timeSpec() );

    switch( intervalType )
    {
        case QwtDate::Millisecond:
        {
            const QTime t = dateTime.time();
            const int ms = qwtAlignValue( t.msec(), stepSize, up ) ;

            dt.setTime( QTime( t.hour(), t.minute(), t.second() ) );
            dt = dt.addMSecs( ms );

            break;
        }
        case QwtDate::Second:
        {
            const QTime t = dateTime.time();
            const int s = qwtAlignValue( t.second(), stepSize, up );

            dt.setTime( QTime( t.hour(), t.minute() ) );
            dt = dt.addSecs( s );

            break;
        }
        case QwtDate::Minute:
        {
            const QTime t = dateTime.time();
            const int m = qwtAlignValue( t.minute(), stepSize, up );

            dt.setTime( QTime( t.hour(), 0 ) );
            dt = dt.addSecs( m * 60 );

            break;
        }
        case QwtDate::Hour:
        {
            const QTime t = dateTime.time();
            const int h = qwtAlignValue( t.hour(), stepSize, up );

            dt = dt.addSecs( h * 3600 );

            break;
        }
        case QwtDate::Day:
        {
            // ????
#if 1
            const int d = qwtAlignValue(
                dateTime.date().dayOfYear(), stepSize, up );

            dt.setDate( QDate( dateTime.date().year(), 1, 1 ) );
            dt = dt.addDays( d );

            break;
#endif
        }
        case QwtDate::Week:
        {
#if 1
            QwtDate::Week0Type week0Type = QwtDate::FirstThursday;
#endif
            const QDate date = QwtDate::dateOfWeek0(
                dateTime.date().year(), week0Type );

            const int numWeeks = date.daysTo( dateTime.date() ) / 7;
            const int d = qwtAlignValue( numWeeks, stepSize, up ) * 7;

            dt.setDate( date );
            dt = dt.addDays( d );

            break;
        }
        case QwtDate::Month:
        {
            const QDate date = dateTime.date();

            const int m = qwtAlignValue( date.month() - 1, stepSize, up );

            dt.setDate( QDate( date.year(), m + 1, 1 ) );

            break;
        }
        case QwtDate::Year:
        {
            const int y = qwtAlignValue( dateTime.date().year(), stepSize, up );

            if ( y == 0 )
            {
                // there is no year 0 in the Julian calendar
                dt.setDate( QDate( stepSize, 1, 1 ).addYears( -stepSize ) );
            }
            else
            {
                dt.setDate( QDate( y, 1, 1 ) );
            }

            break;
        }
    }

    return dt;
}

static double qwtIntervalWidth( const QDateTime &minDate,
    const QDateTime &maxDate, QwtDate::IntervalType intervalType ) 
{
    switch( intervalType )
    {
        case QwtDate::Millisecond:
        {
            const double secsTo = minDate.secsTo( maxDate );
            const double msecs = maxDate.time().msec() -
                minDate.time().msec();

            return secsTo * 1000 + msecs;
        }
        case QwtDate::Second:
        {
            return minDate.secsTo( maxDate );
        }
        case QwtDate::Minute:
        {
            const double secsTo = minDate.secsTo( maxDate );
            return ::floor( secsTo / 60 );
        }
        case QwtDate::Hour:
        {
            const double secsTo = minDate.secsTo( maxDate );
            return ::floor( secsTo / 3600 );
        }
        case QwtDate::Day:
        {
            return minDate.daysTo( maxDate );
        }
        case QwtDate::Week:
        {
            return ::floor( minDate.daysTo( maxDate ) / 7.0 );
        }
        case QwtDate::Month:
        {
            const double years = double( maxDate.date().year() ) - minDate.date().year();

            int months = maxDate.date().month() - minDate.date().month();
            if ( maxDate.date().day() < minDate.date().day() )
                months--;

            return years * 12 + months;
        }
        case QwtDate::Year:
        {
            double years = double( maxDate.date().year() ) - minDate.date().year();
            if ( maxDate.date().month() < minDate.date().month() )
                years -= 1.0;

            return years;
        }
    }

    return 0.0;
}

static double qwtRoundedIntervalWidth( 
    const QDateTime &minDate, const QDateTime &maxDate, 
    QwtDate::IntervalType intervalType ) 
{
    const QDateTime minD = QwtDate::floor( minDate, intervalType );
    const QDateTime maxD = QwtDate::ceil( maxDate, intervalType );

    return qwtIntervalWidth( minD, maxD, intervalType );
}

static inline double qwtMsecsForType( QwtDate::IntervalType type )
{
    static const double msecs[] =
    {
        1.0,
        1000.0,
        60.0 * 1000.0,
        3600.0 * 1000.0,
        24.0 * 3600.0 * 1000.0,
        7.0 * 24.0 * 3600.0 * 1000.0,
        30.0 * 24.0 * 3600.0 * 1000.0,
        365.0 * 24.0 * 3600.0 * 1000.0,
    };

    if ( type < 0 || type >= sizeof( msecs ) / sizeof( msecs[0] ) )
        return 1.0;

    return msecs[ type ];
}

static inline QDateTime qwtToDateTime( double value, Qt::TimeSpec timeSpec )
{
    QDateTime dt = QwtDate::toDateTime( value, timeSpec );
    if ( !dt.isValid() )
    {
        const QDate date = ( value <= 0.0 ) ? QwtDate::minDate() : QwtDate::maxDate();
        dt = QDateTime( date, QTime( 0, 0 ), timeSpec );
    }

    return dt;
}

static inline QwtInterval qwtRoundInterval( 
    const QwtInterval &interval, QwtDate::IntervalType type,
    Qt::TimeSpec timeSpec )
{
    const QDateTime d1 = QwtDate::floor( 
        QwtDate::toDateTime( interval.minValue(), timeSpec ), type );

    const QDateTime d2 = QwtDate::ceil( 
        QwtDate::toDateTime( interval.maxValue(), timeSpec ), type );

    return QwtInterval( QwtDate::toDouble( d1 ), QwtDate::toDouble( d2 ) );
}

static inline int qwtStepCount( int intervalSize, int maxSteps,
    const int limits[], size_t numLimits )
{
    for ( uint i = 0; i < numLimits; i++ )
    {
        const int numSteps = intervalSize / limits[ i ];

        if ( numSteps > 1 && numSteps <= maxSteps &&
            numSteps * limits[ i ] == intervalSize )
        {
            return numSteps;
        }
    }

    return 0;
}

static int qwtStepSize( int intervalSize, int maxSteps, uint base ) 
{
    if ( maxSteps <= 0 )
        return 0;

    if ( maxSteps > 2 )
    {
        for ( int numSteps = maxSteps; numSteps > 1; numSteps-- )
        {
            const double stepSize = double( intervalSize ) / numSteps;

            const double p = ::floor( ::log( stepSize ) / ::log( double( base ) ) );
            const double fraction = qPow( base, p );

            for ( uint n = base; n >= 1; n /= 2 )
            {
                if ( qFuzzyCompare( stepSize, n * fraction ) )
                    return qRound( stepSize );

                if ( n == 3 && ( base % 2 ) == 0 )
                {
                    if ( qFuzzyCompare( stepSize, 2 * fraction ) )
                        return qRound( stepSize );
                }
            }
        }
    }

    return 0;
}

static int qwtDivideInterval( double intervalSize, int numSteps, 
    const int limits[], size_t numLimits )
{
    const int v = qCeil( intervalSize / double( numSteps ) );

    for ( uint i = 0; i < numLimits - 1; i++ )
    {
        if ( v <= limits[i] )
            return limits[i];
    }

    return limits[ numLimits - 1 ];
}

static double qwtDivideScale( double intervalSize, int numSteps,
    QwtDate::IntervalType intervalType )
{
    if ( intervalType != QwtDate::Day )
    {
        if ( ( intervalSize > numSteps ) && 
            ( intervalSize <= 2 * numSteps ) )
        {
            return 2.0;
        }
    }

    double stepSize;

    switch( intervalType )
    {
        case QwtDate::Second:
        case QwtDate::Minute:
        {
            static int limits[] = { 1, 2, 5, 10, 15, 20, 30, 60 };
    
            stepSize = qwtDivideInterval( intervalSize, numSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            break;
        }
        case QwtDate::Hour:
        {
            static int limits[] = { 1, 2, 3, 4, 6, 12, 24 };
    
            stepSize = qwtDivideInterval( intervalSize, numSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            break;
        }
        case QwtDate::Day:
        {
            const double v = intervalSize / double( numSteps );
            if ( v <= 5.0 )
                stepSize = qCeil( v );
            else
                stepSize = qCeil( v / 7 ) * 7;

            break;
        }
        case QwtDate::Week:
        {
            static int limits[] = { 1, 2, 4, 8, 12, 26, 52 };

            stepSize = qwtDivideInterval( intervalSize, numSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            break;
        }
        case QwtDate::Month:
        {
            static int limits[] = { 1, 2, 3, 4, 6, 12 };

            stepSize = qwtDivideInterval( intervalSize, numSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            break;
        }
        case QwtDate::Year:
        case QwtDate::Millisecond:
        default:
        {
            stepSize = QwtScaleArithmetic::divideInterval(
                intervalSize, numSteps, 10 );
        }
    }

    return stepSize;
}

static double qwtDivideMajorStep( double stepSize, int maxMinSteps,
    QwtDate::IntervalType intervalType )
{
    double minStepSize = 0.0;

    switch( intervalType )
    {
        case QwtDate::Second:
        {
            minStepSize = qwtStepSize( stepSize, maxMinSteps, 10 );
            if ( minStepSize == 0.0 )
                minStepSize = 0.5 * stepSize;

            break;
        }
        case QwtDate::Minute:
        {
            static int limits[] = { 1, 2, 5, 10, 15, 20, 30, 60 };

            int numSteps;

            if ( stepSize > maxMinSteps )
            {
                numSteps = qwtStepCount( stepSize, maxMinSteps, 
                    limits, sizeof( limits ) / sizeof( int ) );

            }
            else
            {
                numSteps = qwtStepCount( stepSize * 60, maxMinSteps, 
                    limits, sizeof( limits ) / sizeof( int ) );
            }

            if ( numSteps > 0 )
                minStepSize = double( stepSize ) / numSteps;

            break;
        }
        case QwtDate::Hour:
        {
            int numSteps = 0;

            if ( stepSize > maxMinSteps )
            {
                static int limits[] = { 1, 2, 3, 4, 6, 12, 24, 48, 72 };

                numSteps = qwtStepCount( stepSize, maxMinSteps,
                    limits, sizeof( limits ) / sizeof( int ) );
            }
            else
            {
                static int limits[] = { 1, 2, 5, 10, 15, 20, 30, 60 };

                numSteps = qwtStepCount( stepSize * 60, maxMinSteps,
                    limits, sizeof( limits ) / sizeof( int ) );
            }

            if ( numSteps > 0 )
                minStepSize = double( stepSize ) / numSteps;

            break;
        }
        case QwtDate::Day:
        {
            int numSteps = 0;

            if ( stepSize > maxMinSteps )
            {
                static int limits[] = { 1, 2, 3, 7, 14, 28 };

                numSteps = qwtStepCount( stepSize, maxMinSteps,
                    limits, sizeof( limits ) / sizeof( int ) );
            }
            else
            {
                static int limits[] = { 1, 2, 3, 4, 6, 12, 24, 48, 72 };

                numSteps = qwtStepCount( stepSize * 24, maxMinSteps,
                    limits, sizeof( limits ) / sizeof( int ) );
            }

            if ( numSteps > 0 )
                minStepSize = double( stepSize ) / numSteps;

            break;
        }
        case QwtDate::Week:
        {
            const int daysInStep = stepSize * 7;

            if ( maxMinSteps >= daysInStep )
            {
                // we want to have one tick per day
                minStepSize = 1.0 / 7.0;
            }
            else
            {
                // when the stepSize is more than a week we want to
                // have a tick for each week

                const int stepSizeInWeeks = stepSize;

                if ( stepSizeInWeeks <= maxMinSteps )
                {
                    minStepSize = 1;
                }
                else
                {
                    minStepSize = QwtScaleArithmetic::divideInterval( 
                        stepSizeInWeeks, maxMinSteps, 10 );
                }
            }
            break;
        }
        case QwtDate::Month:
        {
            // fractions of months doesn't make any sense

            if ( stepSize < maxMinSteps )
                maxMinSteps = static_cast<int>( stepSize );

            static int limits[] = { 1, 2, 3, 4, 6, 12 };

            int numSteps = qwtStepCount( stepSize, maxMinSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            if ( numSteps > 0 )
                minStepSize = double( stepSize ) / numSteps;

            break;
        }
        case QwtDate::Year:
        {
            if ( stepSize >= maxMinSteps )
            {
                minStepSize = QwtScaleArithmetic::divideInterval(
                    stepSize, maxMinSteps, 10 );
            }
            else
            {
                // something in months

                static int limits[] = { 1, 2, 3, 4, 6, 12 };

                int numSteps = qwtStepCount( 12 * stepSize, maxMinSteps,
                    limits, sizeof( limits ) / sizeof( int ) );

                if ( numSteps > 0 )
                    minStepSize = double( stepSize ) / numSteps;
            }
                
            break;
        }
        default:
            break;
    }

    if ( intervalType != QwtDate::Month
        && minStepSize == 0.0 )
    {
        minStepSize = 0.5 * stepSize;
    }

    return minStepSize;
}

static inline int qwtHoursUTC( const QDateTime &date )
{
    const QDateTime dateUTC = date.toUTC();

    const int hours = date.time().hour() - dateUTC.time().hour();

    int days = date.date().dayOfYear() - dateUTC.date().dayOfYear();
    if ( qAbs( days ) > 1 )
        days = ( date.date().year() > dateUTC.date().year() ) ? 1 : -1;

    return days * 24 + hours;
}

// stepSize, minStepSize in seconds
static QwtScaleDiv qwtBuildScaleDiv( 
    const QDateTime &minDate, const QDateTime &maxDate,
    int stepSize, double minStepSize, bool daylightSaving )
{
    // UTC excludes daylight savings. So from the difference
    // of a date and its UTC counterpart we can find out
    // the daylight saving hours

    const int hourDST = qwtHoursUTC( minDate );

    QList<double> majorTicks;
    QList<double> minorTicks;
    QList<double> mediumTicks;

    for ( QDateTime dt = minDate; dt <= maxDate; 
        dt = dt.addSecs( stepSize ) )
    {
        if ( !dt.isValid() )
        {
#if DEBUG_ENGINE >= 1
            qDebug() << "Invalid date in: " << minDate << maxDate();
#endif
            break;
        }

        double majorValue = QwtDate::toDouble( dt );

        if ( daylightSaving )
            majorValue += ( hourDST - qwtHoursUTC( dt ) ) * 3600000;

        if ( majorTicks.isEmpty() || majorTicks.last() != majorValue )
            majorTicks += majorValue;

        if ( minStepSize > 0.0 )
        {
            const int numSteps = qFloor( stepSize / minStepSize );

            for ( int i = 1; i < numSteps; i++ )
            {
                const QDateTime mt = dt.addMSecs( 
                    qRound64( i * minStepSize * 1000 ) );

                double minorValue = QwtDate::toDouble( mt );
                if ( daylightSaving )
                    minorValue += ( hourDST - qwtHoursUTC( mt ) ) * 3600000;

                if ( minorTicks.isEmpty() || minorTicks.last() != minorValue )
                {
                    const bool isMedium = ( numSteps % 2 == 0 ) 
                        && ( i != 1 ) && ( i == numSteps / 2 );

                    if ( isMedium )
                        mediumTicks += minorValue;
                    else
                        minorTicks += minorValue;
                }
            }
        }
    }

    QwtScaleDiv scaleDiv;

    scaleDiv.setInterval( QwtDate::toDouble( minDate ),
        QwtDate::toDouble( maxDate ) );

    scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );
    scaleDiv.setTicks( QwtScaleDiv::MediumTick, mediumTicks );
    scaleDiv.setTicks( QwtScaleDiv::MinorTick, minorTicks );

    return scaleDiv;
}

TimeScaleEngine::TimeScaleEngine( Qt::TimeSpec timeSpec ):
    d_timeSpec( timeSpec ),
    d_maxWeeks( 4 )
{
}

TimeScaleEngine::~TimeScaleEngine()
{
}

void TimeScaleEngine::setTimeSpec( Qt::TimeSpec timeSpec )
{
    d_timeSpec = timeSpec;
}

Qt::TimeSpec TimeScaleEngine::timeSpec() const
{
    return d_timeSpec;
}

void TimeScaleEngine::setMaxWeeks( int weeks )
{
    d_maxWeeks = qMax( weeks, 0 );
}

int TimeScaleEngine::maxWeeks() const
{
    return d_maxWeeks;
}

QwtDate::IntervalType TimeScaleEngine::intervalType( 
    double min, double max, int maxSteps ) const
{
    const double i0 = maxSteps * qwtMsecsForType( QwtDate::Year );
    if ( min < 0 && max > 0 )
    {
        if ( max - i0 > min )
            return QwtDate::Year;
    }
    else
    {
        if ( max - min > i0 )
            return QwtDate::Year;
    }

    const QDateTime minDate = QwtDate::toDateTime( min, d_timeSpec );
    const QDateTime maxDate = QwtDate::toDateTime( max, d_timeSpec );

    const int months = qwtRoundedIntervalWidth( minDate, maxDate, QwtDate::Month );
    if ( months > maxSteps * 6 )
        return QwtDate::Year;

    const int days = qwtRoundedIntervalWidth( minDate, maxDate, QwtDate::Day );
    const int weeks = qwtRoundedIntervalWidth( minDate, maxDate, QwtDate::Week );

    if ( weeks > d_maxWeeks )
    {
        if ( days > 4 * maxSteps * 7 )
            return QwtDate::Month;
    }

    if ( days > maxSteps * 7 )
        return QwtDate::Week;

    const int hours = qwtRoundedIntervalWidth( minDate, maxDate, QwtDate::Hour );
    if ( hours > maxSteps * 24 )
        return QwtDate::Day;

    const int seconds = qwtRoundedIntervalWidth( minDate, maxDate, QwtDate::Second );

    if ( seconds >= maxSteps * 3600 )
        return QwtDate::Hour;

    if ( seconds >= maxSteps * 60 )
        return QwtDate::Minute;

    if ( seconds >= maxSteps )
        return QwtDate::Second;

    return QwtDate::Millisecond;
}

void TimeScaleEngine::autoScale( int maxNumSteps,
    double &x1, double &x2, double &stepSize ) const
{
    stepSize = 0.0;

    QwtInterval interval( x1, x2 );
    interval = interval.normalized();

    interval.setMinValue( interval.minValue() - lowerMargin() );
    interval.setMaxValue( interval.maxValue() + upperMargin() );

    if ( testAttribute( QwtScaleEngine::Symmetric ) )
        interval = interval.symmetrize( reference() );

    if ( testAttribute( QwtScaleEngine::IncludeReference ) )
        interval = interval.extend( reference() );

    if ( interval.width() == 0.0 )
        interval = buildInterval( interval.minValue() );

    const QDateTime from = QwtDate::toDateTime( interval.minValue(), d_timeSpec );
    const QDateTime to = QwtDate::toDateTime( interval.maxValue(), d_timeSpec );

    if ( from.isValid() && to.isValid() )
    {
        if ( maxNumSteps < 1 )
            maxNumSteps = 1;

        const QwtDate::IntervalType type =
            intervalType( interval.minValue(), interval.maxValue(), maxNumSteps );

        stepSize = divideInterval( from, to, type, maxNumSteps );

        if ( stepSize != 0.0 && !testAttribute( QwtScaleEngine::Floating ) )
        {
            interval = align( interval, stepSize );
            interval = qwtRoundInterval( interval, type, d_timeSpec );
        }
    }

    x1 = interval.minValue();
    x2 = interval.maxValue();

    if ( testAttribute( QwtScaleEngine::Inverted ) )
    {
        qSwap( x1, x2 );
        stepSize = -stepSize;
    }
}

double TimeScaleEngine::divideInterval( 
    const QDateTime &from, const QDateTime &to,
    QwtDate::IntervalType intervalType, int numSteps ) const
{
    const double width = qwtIntervalWidth( from, to, intervalType );
    
    double stepSize = QwtScaleArithmetic::divideInterval( width, numSteps, 10 );

    return stepSize * qwtMsecsForType( intervalType );
}

QwtScaleDiv TimeScaleEngine::divideScale( double x1, double x2,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
    if ( maxMajSteps < 1 )
        maxMajSteps = 1;

    stepSize = qAbs( stepSize );

    double min = qMin( x1, x2 );
    const double max = qMax( x1, x2 );

    const QDateTime from = qwtToDateTime( min, d_timeSpec );
    const QDateTime to = qwtToDateTime( max, d_timeSpec );

    if ( from == to )
        return QwtScaleDiv();

    if ( stepSize > 0.0 )
    {
        // as interval types above hours are not equidistant
        // ( even days might have 23/25 hours because of daylight saving )
        // the stepSize is used as a hint only

        maxMajSteps = qCeil( ( max - min ) / stepSize );
    }

    const QwtDate::IntervalType type = intervalType( min, max, maxMajSteps );

#if DEBUG_ENGINE >= 1
    qDebug() << "Divide: " << min << max << from << to << maxMajSteps 
        << "Type: " << ( int )type;
#endif

    QwtScaleDiv scaleDiv;

    if ( type == QwtDate::Millisecond )
    {
        scaleDiv = QwtLinearScaleEngine::divideScale( min, max,
            maxMajSteps, maxMinSteps, stepSize );
    }
    else
    {
        scaleDiv = divideTo( min, max, maxMajSteps, maxMinSteps, type );
    }

    if ( x1 > x2 )
        scaleDiv.invert();

#if DEBUG_ENGINE >= 3
    qDebug() << scaleDiv;
#endif

    return scaleDiv;
}

QwtScaleDiv TimeScaleEngine::divideTo( double min, double max,
    int maxMajSteps, int maxMinSteps,
    QwtDate::IntervalType intervalType ) const
{
    QDateTime minDate = qwtToDateTime( min, d_timeSpec );
    QDateTime maxDate = qwtToDateTime( max, d_timeSpec );

    // round the interval
    minDate = QwtDate::floor( minDate, intervalType );
    maxDate = QwtDate::ceil( maxDate, intervalType );

    // calculate the step size
    const double stepSize = qwtDivideScale( 
        qwtIntervalWidth( minDate, maxDate, intervalType ), 
        maxMajSteps, intervalType );

    // align to step size
    QDateTime dt0 = qwtAlignDate( minDate, stepSize, intervalType, false );
    if ( !dt0.isValid() )
    {
        dt0 = qwtAlignDate( minDate, stepSize, intervalType, true );
    }
    minDate = dt0;

    QwtScaleDiv scaleDiv;

    if ( intervalType <= QwtDate::Week )
    {
        // calculate the min step size
        double minStepSize = 0;

        if ( maxMinSteps > 1 ) 
        {
            minStepSize = qwtDivideMajorStep( stepSize, maxMinSteps, intervalType );
        }

        static const int seconds[] =
            { 1, 60, 60 * 60, 24 * 60 * 60, 7 * 24 * 60 * 60 };

        const int s = seconds[ intervalType - QwtDate::Second ];

        bool daylightSaving = intervalType > QwtDate::Hour;
        if ( intervalType == QwtDate::Hour )
        {
            daylightSaving = stepSize > 1;
        }
    
        scaleDiv = qwtBuildScaleDiv( minDate, maxDate,
            stepSize * s, minStepSize * s, daylightSaving );
    }
    else
    {
        // months and years are intervals with non equidistant ( in ms ) steps:
        // we have to build the scale division manually

        QList<double> majorTicks;
        QList<double> mediumTicks;
        QList<double> minorTicks;

        if( intervalType == QwtDate::Month )
        {
            int minStepDays = 0;
            int minStepSize = 0.0; 

            if ( maxMinSteps > 1 )
            {
                if ( stepSize == 1 )
                {
                    if ( maxMinSteps >= 30 )
                        minStepDays = 1;
                    else if ( maxMinSteps >= 6 )
                        minStepDays = 5;
                    else if ( maxMinSteps >= 3 )
                        minStepDays = 10;

                    minStepDays = 15;
                }
                else
                {
                    minStepSize = qwtDivideMajorStep( 
                        stepSize, maxMinSteps, intervalType );
                }
            }

                
            for ( QDateTime dt = minDate; 
                dt <= maxDate; dt = dt.addMonths( stepSize ) )
            {
                if ( !dt.isValid() )
                {   
#if DEBUG_ENGINE >= 1
                    qDebug() << "Invalid date in: " << minDate << maxDate;
#endif  
                    break;
                }   

                majorTicks += QwtDate::toDouble( dt );

                if ( minStepDays > 0 )
                {
                    for ( int days = minStepDays; 
                        days < 30; days += minStepDays )
                    {
                        const double tick = QwtDate::toDouble( dt.addDays( days ) );
 
                        if ( days == 15 && minStepDays != 15 )
                            mediumTicks += tick;
                        else
                            minorTicks += tick;
                    }
                }
                else if ( minStepSize > 0.0 )
                {
                    const int numMinorSteps = qRound( stepSize / (double) minStepSize );

                    for ( int i = 1; i < numMinorSteps; i++ )
                    {
                        const double minorValue =
                            QwtDate::toDouble( dt.addMonths( i * minStepSize ) );

                        if ( ( numMinorSteps % 2 == 0 ) && ( i == numMinorSteps / 2 ) )
                            mediumTicks += minorValue;
                        else
                            minorTicks += minorValue;
                    }
                }
            }
        }
        else if ( intervalType == QwtDate::Year )
        {
            double minStepSize = 0.0;

            if ( maxMinSteps > 1 )
            {
                minStepSize = qwtDivideMajorStep( 
                    stepSize, maxMinSteps, intervalType );
            }


            int numMinorSteps = 0;
            if ( minStepSize > 0.0 )
                numMinorSteps = qFloor( stepSize / minStepSize );

            bool dateBC = minDate.date().year() < -1;

            for ( QDateTime dt = minDate; dt <= maxDate;
                dt = dt.addYears( stepSize ) )
            {
                if ( dateBC && dt.date().year() > 1 )
                {
                    // there is no year 0 in the Julian calendar
                    dt = dt.addYears( -1 );
                    dateBC = false;
                }

                if ( !dt.isValid() )
                {
#if DEBUG_ENGINE >= 1
                    qDebug() << "Invalid date in: " << minDate << maxDate;
#endif
                    break;
                }

                const double tick = QwtDate::toDouble( dt );
                if ( tick >= min )
                {
                    majorTicks += tick;
                }

                for ( int i = 1; i < numMinorSteps; i++ )
                {
                    QDateTime tickDate;

                    const double years = qRound( i * minStepSize );
                    if ( years >= INT_MAX / 12 )
                    {
                        tickDate = dt.addYears( years );
                    }
                    else
                    {
                        tickDate = dt.addMonths( qRound( years * 12 ) );
                    }

                    const bool isMedium = ( numMinorSteps > 2 ) &&
                        ( numMinorSteps % 2 == 0 ) && ( i == numMinorSteps / 2 );

                    const double minorValue = QwtDate::toDouble( tickDate );
                    if ( isMedium )
                        mediumTicks += minorValue;
                    else
                        minorTicks += minorValue;
                }

                if ( QwtDate::maxDate().addYears( -stepSize ) < dt.date() )
                {
                    break;
                }
            }   
        }

        scaleDiv.setInterval( QwtDate::toDouble( minDate ),
            QwtDate::toDouble( maxDate ) );

        scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );
        scaleDiv.setTicks( QwtScaleDiv::MediumTick, mediumTicks );
        scaleDiv.setTicks( QwtScaleDiv::MinorTick, minorTicks );
    }

    // scaleDiv has been calculated from an extended interval
    // that had been rounded according to the step size.
    // So we have to shrink it again.

    scaleDiv = scaleDiv.bounded( min, max );

    return scaleDiv;
}

QwtTransform *TimeScaleEngine::transformation() const
{
    return new QwtNullTransform();
}

