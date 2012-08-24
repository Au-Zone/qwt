#include "timescaleengine.h"
#include <qwt_interval.h>
#include <qwt_math.h>
#include <qwt_transform.h>
#include <qdatetime.h>
#include <qdebug.h>

#define DEBUG_ENGINE 0

static inline int qwtFloorToStep( int value, double stepSize )
{
    return qRound( int( value / stepSize ) * stepSize );
}

static inline int qwtDivideInterval( double intervalSize, int numSteps, 
    const int limits[], size_t numLimits )
{
    const double v = intervalSize / numSteps;

    for ( uint i = 0; i < numLimits - 1; i++ )
    {
#if 1 // >= ??
        if ( v <= limits[i] )
            return limits[i];
#endif
    }

    return limits[ numLimits - 1 ];
}

static inline int qwtDivideMonths(
    double intervalSize, int numSteps )
{
    static int limits[] = { 1, 2, 3, 4, 6, 12 };

    return qwtDivideInterval( intervalSize, numSteps,
        limits, sizeof( limits ) / sizeof( int ) );
}

static inline int qwtDivideWeeks(
    double intervalSize, int numSteps )
{
    static int limits[] = { 1, 2, 4, 8, 12, 26, 52 };

    return qwtDivideInterval( intervalSize, numSteps,
        limits, sizeof( limits ) / sizeof( int ) );
}

static inline int qwtDivideDays(
    double intervalSize, int numSteps )
{
    const double v = intervalSize / numSteps;
    if ( v <= 5.0 )
        return qCeil( v );

    return qCeil( v / 7 ) * 7;
}

static inline int qwtDivideHours(
    double intervalSize, int numSteps )
{
    static int limits[] = { 1, 2, 3, 4, 6, 12, 24 };

    return qwtDivideInterval( intervalSize, numSteps,
        limits, sizeof( limits ) / sizeof( int ) );
}

static inline int qwtDivide60(
    double intervalSize, int numSteps )
{
    static int limits[] = { 1, 2, 5, 10, 15, 20, 30, 60 };
    
    return qwtDivideInterval( intervalSize, numSteps,
        limits, sizeof( limits ) / sizeof( int ) );
}   

TimeScaleEngine::TimeScaleEngine():
    d_maxWeeks( -1 )
{
}

TimeScaleEngine::~TimeScaleEngine()
{
}

void TimeScaleEngine::setMaxWeeks( int weeks )
{
    d_maxWeeks = qMax( weeks, -1 );
}

int TimeScaleEngine::maxWeeks() const
{
    return d_maxWeeks;
}

TimeDate::IntervalType TimeScaleEngine::intervalType( 
    double min, double max, int maxSteps ) const
{
    const QDateTime from = qwtToDateTime( min );
    const QDateTime to = qwtToDateTime( max );

    const QDate dm1 = qwtFloorDate( from, TimeDate::Month ).date();
    const QDate dm2 = qwtCeilDate( to, TimeDate::Month ).date();

    const int months = 12 * ( dm2.year() - dm1.year() ) + ( dm2.month() - dm1.month() );
    if ( months > maxSteps * 6 )
        return TimeDate::Year;

    const QDate dt1 = qwtFloorDate( from, TimeDate::Day ).date();
    const QDate dt2 = qwtCeilDate( to, TimeDate::Day ).date();

    const int days = dt1.daysTo( dt2 );
    const int weeks = qCeil( days / 7.0 );

    if ( d_maxWeeks >= 0 )
    {
        if ( weeks > d_maxWeeks )
        {
            if ( days > 2 * maxSteps * 7 )
                return TimeDate::Month;
        }
        else
        {
            if ( days > 2 * maxSteps )
                return TimeDate::Week;
        }
    }
    else
    {
        if ( days > 2 * maxSteps * 7 )
            return TimeDate::Month;

        if ( days > 2 * maxSteps )
            return TimeDate::Week;
    }

    const QDateTime dh1 = qwtFloorDate( from, TimeDate::Hour );
    const QDateTime dh2 = qwtCeilDate( to, TimeDate::Hour );

    const int hours = dh1.secsTo( dh2 ) / 3600;
    if ( hours > maxSteps * 24 )
        return TimeDate::Day;

    const int seconds = from.secsTo( to );

    if ( seconds > maxSteps * 3600 )
        return TimeDate::Hour;

    if ( seconds > maxSteps * 60 )
        return TimeDate::Minute;

    if ( seconds >= maxSteps )
        return TimeDate::Second;

    return TimeDate::Millisecond;
}


void TimeScaleEngine::autoScale( int maxNumSteps,
    double &x1, double &x2, double &stepSize ) const
{
    QwtLinearScaleEngine::autoScale( maxNumSteps, x1, x2, stepSize );
}

QwtScaleDiv TimeScaleEngine::divideScale( double x1, double x2,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
    if ( maxMajSteps < 1 )
        maxMajSteps = 1;

    stepSize = qAbs( stepSize );

    const double min = qMin( x1, x2 );
    const double max = qMax( x1, x2 );

    const QDateTime from = qwtToDateTime( min );
    if ( !from.isValid() )
    {
        qDebug() << "Invalid: " << min << from;
        return QwtScaleDiv();
    }

    const QDateTime to = qwtToDateTime( max );
    if ( !to.isValid() )
    {
        qDebug() << "Invalid: " << max << to;
        return QwtScaleDiv();
    }

#if DEBUG_ENGINE >= 1
    qDebug() << "Divide: " << min << max << from << to << maxMajSteps;
#endif
    QwtScaleDiv scaleDiv;

    switch ( intervalType( min, max, maxMajSteps ) )
    {
        case TimeDate::Year:
        {
            scaleDiv = divideToYears( min, max, maxMajSteps, maxMinSteps, stepSize );
            break;
        }
        case TimeDate::Month:
        {
            scaleDiv = divideToMonths( min, max, maxMajSteps, maxMinSteps, stepSize );
            break;
        }
        case TimeDate::Week:
        {
            scaleDiv = divideToWeeks( min, max, maxMajSteps, maxMinSteps, stepSize );
            break;
        }
        case TimeDate::Day:
        {
            scaleDiv = divideToDays( min, max, maxMajSteps, maxMinSteps, stepSize );
            break;
        }
        case TimeDate::Hour:
        {
            scaleDiv = divideToHours( min, max, maxMajSteps, maxMinSteps, stepSize );
            break;
        }
        case TimeDate::Minute:
        {
            scaleDiv = divideToMinutes( min, max, maxMajSteps, maxMinSteps, stepSize );
            break;
        }
        case TimeDate::Second:
        {
            scaleDiv = divideToSeconds( min, max, maxMajSteps, maxMinSteps, stepSize );
            break;
        }
        case TimeDate::Millisecond:
        {
            scaleDiv = QwtLinearScaleEngine::divideScale( min, max,
                maxMajSteps, maxMinSteps, stepSize );

#if DEBUG_ENGINE >= 2
            qDebug() << "Millisecond: " << max - min;
            const QList<double> ticks = scaleDiv.ticks( QwtScaleDiv::MajorTick);
            for ( int i = 0; i < ticks.size(); i++ )
                qDebug() << i << qRound64( ticks[i] );
#endif

            break;
        }
    }

    if ( x1 > x2 )
        scaleDiv.invert();

#if DEBUG_ENGINE >= 3
    qDebug() << scaleDiv;
#endif

    return scaleDiv;
}


QwtScaleDiv TimeScaleEngine::divideToSeconds( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
    // round min/max to seconds
    QDateTime from = qwtToDateTime( min );
    from = qwtFloorDate( from, TimeDate::Second );

    QDateTime to = qwtToDateTime( max );
    to = qwtCeilDate( to, TimeDate::Second );

    // calculate the step size

    if ( stepSize == 0.0 )
    {
        const double seconds = from.secsTo( to );
        stepSize = qwtDivide60( seconds, maxMajSteps );

#if DEBUG_ENGINE >= 2
        qDebug() << "Seconds: " << seconds << " -> " << stepSize;
#endif

        stepSize *= 1000; // in ms
    }

    // align to step size
    const int s = qwtFloorToStep( from.time().second(), stepSize / 1000 );
    from.setTime( QTime( from.time().hour(), from.time().minute(), s ) );

    return buildScaleDiv( min, max, from, to, stepSize, maxMinSteps );
}

QwtScaleDiv TimeScaleEngine::divideToMinutes( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
    const double msMinute = 60 * 1000; // ms per minute

    QDateTime from = qwtToDateTime( min );
    from = qwtFloorDate( from, TimeDate::Minute );

    QDateTime to = qwtToDateTime( max );
    to = qwtCeilDate( to, TimeDate::Minute );

    if ( stepSize == 0.0 )
    {
        const double minutes = from.secsTo( to ) / 60;
        stepSize = qwtDivide60( minutes, maxMajSteps );
#if DEBUG_ENGINE >= 2
        qDebug() << "Minutes: " << minutes << " -> " << stepSize;
#endif

        stepSize *= msMinute; // ms
    }

    // align to step size
    const int m = qwtFloorToStep( 
        from.time().minute(), stepSize / msMinute );
    from.setTime( QTime( from.time().hour(), m ) );

    return buildScaleDiv( min, max, from, to, stepSize, maxMinSteps );
}

QwtScaleDiv TimeScaleEngine::divideToHours( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
    const double msHour = 60 * 60 * 1000; // ms per hour

    QDateTime from = qwtToDateTime( min );
    from = qwtFloorDate( from, TimeDate::Hour );

    QDateTime to = qwtToDateTime( max );
    to = qwtCeilDate( to, TimeDate::Hour );

    if ( stepSize == 0.0 )
    {
        const double hours = from.secsTo( to ) / 3600;
        stepSize = qwtDivideHours( hours, maxMajSteps );

#if DEBUG_ENGINE >= 2
        qDebug() << "Hours: " << hours << " -> " << stepSize;
#endif

        stepSize *= msHour; // ms
    }

    // align to step size
    const int h = qwtFloorToStep( from.time().hour(), stepSize / msHour );
    from.setTime( QTime( h, 0 ) );

    return buildScaleDiv( min, max, from, to, stepSize, maxMinSteps );
}

QwtScaleDiv TimeScaleEngine::divideToDays( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
    const double msDay = 24 * 60 * 60 * 1000; // ms per day

    QDateTime from = qwtToDateTime( min );
    from = qwtFloorDate( from, TimeDate::Day );

    QDateTime to = qwtToDateTime( max );
    to = qwtCeilDate( to, TimeDate::Day );

    if ( stepSize == 0.0 )
    {
        const double days = from.daysTo( to );
        stepSize = qwtDivideDays( days, maxMajSteps );

#if DEBUG_ENGINE >= 2
        qDebug() << "Days: " << days << " -> " << stepSize;
#endif

        stepSize *= msDay; // ms
    }

    // align to step size
    const int d = qwtFloorToStep( 
        from.date().dayOfYear(), stepSize / msDay );

    from = QDateTime( QDate( from.date().year(), 1, 1 ) );
    from.addDays( d );

    return buildScaleDiv( min, max, from, to, stepSize, maxMinSteps );
}

QwtScaleDiv TimeScaleEngine::divideToWeeks( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
    const double msWeek = 7 * 24 * 60 * 60 * 1000; // ms per week

    QDateTime from = qwtToDateTime( min );
    from = qwtFloorDate( from, TimeDate::Week );

    QDateTime to = qwtToDateTime( max );
    to = qwtCeilDate( to, TimeDate::Week );

    if ( stepSize == 0.0 )
    {
        const int weeks = qFloor( from.daysTo( to ) / 7 );
        stepSize = qwtDivideWeeks( weeks, maxMajSteps );

#if DEBUG_ENGINE >= 2
        qDebug() << "Weeks: " << weeks << " -> " << stepSize;
#endif
        stepSize *= msWeek; // ms
    }

    // align to step size
    const int weeks1 = int( from.date().daysInYear() / 7 );
    const int weeks2 = qwtFloorToStep( weeks1, stepSize / msWeek );

    QDateTime dt( QDate( from.date().year(), 1, 1 ) );
    dt.addDays( from.date().daysInYear() - ( weeks1 - weeks2 ) * 7 );

    from = dt;

    return buildScaleDiv( min, max, from, to, stepSize, maxMinSteps );
}

QwtScaleDiv TimeScaleEngine::divideToMonths( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
    QDateTime from = qwtToDateTime( min );
    from = qwtFloorDate( from, TimeDate::Month );

    QDateTime to = qwtToDateTime( max );
    to = qwtCeilDate( to, TimeDate::Month );

    if ( stepSize == 0.0 )
    {
        const double months = 12 * ( to.date().year() - from.date().year() ) 
            + to.date().month() - from.date().month();

        stepSize = qwtDivideMonths( months, maxMajSteps );

#if DEBUG_ENGINE >= 2
        qDebug() << "Months: " << months << " -> " << stepSize;
#endif
    }

    stepSize = qMax( stepSize, 1.0 );

    // align to step size
    const int m = qwtFloorToStep( from.date().month(), stepSize );
    from = QDateTime( QDate( from.date().year(), m + 1, 1 ) );

    QList<double> majorTicks;
    while ( from <= to )
    {
        const double value = qwtFromDateTime( from );
        if ( value >= min && value <= max )
            majorTicks += value;

        from = from.addMonths( stepSize );
    }

    // calculate the minor/medium ticks
    QList<double> minorTicks;
    QList<double> mediumTicks;

    if ( maxMinSteps >= 1 )
    {
        // what to do here ?
    }

    QwtScaleDiv scaleDiv;
    scaleDiv.setInterval( min, max );
    scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );
    scaleDiv.setTicks( QwtScaleDiv::MediumTick, mediumTicks );
    scaleDiv.setTicks( QwtScaleDiv::MinorTick, minorTicks );

    return scaleDiv;
}

QwtScaleDiv TimeScaleEngine::divideToYears( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
    Q_UNUSED( maxMinSteps );

    QDateTime from = qwtToDateTime( min );
    from = qwtFloorDate( from, TimeDate::Year );

    QDateTime to = qwtToDateTime( max );
    to = qwtCeilDate( to, TimeDate::Year );

    if ( stepSize == 0.0 )
    {
        const int years = to.date().year() - from.date().year();
        stepSize = minorStepSize( years, maxMajSteps );

#if DEBUG_ENGINE >= 2
        qDebug() << "Years: " << from.date() << to.date() << years << " -> " << stepSize;
#endif
    }

    // align to step size
    const int y = qwtFloorToStep( from.date().year(), stepSize );
    from = QDateTime( QDate( y, 1, 1 ) );

    QList<double> majorTicks;
    while ( from <= to )
    {
        const double value = qwtFromDateTime( from );
        if ( value >= min && value <= max )
            majorTicks += value;

        from = from.addMonths( qRound( stepSize * 12 ) );
    }   

    // calculate the minor/medium ticks
    QList<double> minorTicks;
    QList<double> mediumTicks;

    if ( maxMinSteps >= 1 )
    {
        // what to do here ?
    }

    QwtScaleDiv scaleDiv;
    scaleDiv.setInterval( min, max );
    scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );
    scaleDiv.setTicks( QwtScaleDiv::MediumTick, mediumTicks );
    scaleDiv.setTicks( QwtScaleDiv::MinorTick, minorTicks );

    return scaleDiv;
}

double TimeScaleEngine::minorStepSize( 
    double stepSize, int maxMinSteps ) const
{
    double minStepSize = divideInterval( stepSize, maxMinSteps );

#if 1
    const int numSteps = qCeil( stepSize / minStepSize );
    if ( qwtFuzzyCompare( numSteps * minStepSize, stepSize, stepSize ) > 0 )
    {
#if 1
        qDebug() << "Minor steps doesn't fit: " 
            << stepSize << maxMinSteps << " -> " << minStepSize;
#endif
        // The minor steps doesn't fit into the interval
        minStepSize = stepSize * 0.5;
    }
#endif

    return minStepSize;
}

QwtScaleDiv TimeScaleEngine::buildScaleDiv( double min, double max,
    const QDateTime &from, const QDateTime &to, double stepSize,
    int maxMinSteps ) const
{
    // calculate the major ticks
    QList<double> majorTicks;

    for ( QDateTime dt = from; dt <= to; 
        dt = dt.addMSecs( qRound64( stepSize ) ) )
    {
        const double value = qwtFromDateTime( dt );
        if ( value >= min && value <= max )
            majorTicks += value;
    }

    // calculate the minor/medium ticks
    QList<double> minorTicks;
    QList<double> mediumTicks;

    if ( maxMinSteps > 1 )
    {
        const double minStepSize = minorStepSize( stepSize, maxMinSteps );
        const int numSteps = stepSize / minStepSize;

        for ( QDateTime dt = from; dt <= to; 
            dt = dt.addMSecs( qRound64( stepSize ) ) )
        {
            const double tick0 = qwtFromDateTime( dt );

            for ( int i = 1; i < numSteps; i++ )
            {
                const double value = tick0 + i * minStepSize;
                if ( value >= min && value <= max )
                {
                    if ( numSteps % 2 == 0 && i == numSteps / 2 )
                        mediumTicks += value;
                    else
                        minorTicks += value;
                }
            }
        }
    }

    QwtScaleDiv scaleDiv;
    scaleDiv.setInterval( min, max );
    scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );
    scaleDiv.setTicks( QwtScaleDiv::MediumTick, mediumTicks );
    scaleDiv.setTicks( QwtScaleDiv::MinorTick, minorTicks );

    return scaleDiv;
}

QwtTransform *TimeScaleEngine::transformation() const
{
    return new QwtNullTransform();
}

