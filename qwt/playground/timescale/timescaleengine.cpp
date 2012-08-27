#include "timescaleengine.h"
#include <qwt_interval.h>
#include <qwt_math.h>
#include <qwt_transform.h>
#include <qdatetime.h>
#include <qdebug.h>

#define DEBUG_ENGINE 2

static const double msSecond = 1000;
static const double msMinute = 60 * msSecond; 
static const double msHour = 60 * msMinute;
static const double msDay = 24 * msHour;
static const double msWeek = 7 * msDay;

static inline int qwtFloorToStep( int value, double stepSize )
{
    return qRound( int( value / stepSize ) * stepSize );
}

static inline int qwtStepCount( double intervalSize, int maxSteps,
    const int limits[], size_t numLimits )
{
    for ( uint i = 0; i < numLimits; i++ )
    {
        const int numSteps = int( intervalSize / limits[ i ] );

        if ( numSteps > 1 && numSteps <= maxSteps &&
            qFuzzyCompare( intervalSize, numSteps * limits[ i ] ) )
        {
            return numSteps;
        }
    }

    return 0;
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

static double qwtStepSize( double intervalSize, int maxSteps, uint base ) 
{
    if ( maxSteps <= 0 )
        return 0.0;

    if ( maxSteps > 2 )
    {
        for ( int numSteps = maxSteps; numSteps > 1; numSteps-- )
        {
            const double stepSize = intervalSize / numSteps;

            const double p = ::floor( ::log( stepSize ) / ::log( double( base ) ) );
            const double fraction = qPow( base, p );

            for ( uint n = base; n >= 1; n /= 2 )
            {
                if ( qFuzzyCompare( stepSize, n * fraction ) )
                    return stepSize;

                if ( n == 3 && ( base % 2 ) == 0 )
                {
                    if ( qFuzzyCompare( stepSize, 2 * fraction ) )
                        return stepSize;
                }
            }
        }
    }

    return intervalSize * 0.5;
}

static QwtScaleDiv qwtBuildScaleDiv( double min, double max,
    const QDateTime &from, const QDateTime &to, 
    double stepSize, double minStepSize, bool daylightSaving )
{
	Q_UNUSED( daylightSaving );

    QList<double> majorTicks;
    QList<double> minorTicks;
    QList<double> mediumTicks;

    for ( QDateTime dt = from; dt <= to; 
        dt = dt.addMSecs( qRound64( stepSize ) ) )
    {
        const double majorValue = qwtFromDateTime( dt );
        if ( majorValue >= min && majorValue <= max )
            majorTicks += majorValue;

        if ( minStepSize > 0.0 )
        {
            const int numSteps = qFloor( stepSize / minStepSize );

            for ( int i = 1; i < numSteps; i++ )
            {
                const double minorValue = majorValue + i * minStepSize;
                if ( minorValue >= min && minorValue <= max )
                {
                    if ( numSteps % 2 == 0 && i == numSteps / 2 )
                        mediumTicks += minorValue;
                    else
                        minorTicks += minorValue;
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
    if ( months / maxSteps > 6 )
        return TimeDate::Year;

    const QDate dt1 = qwtFloorDate( from, TimeDate::Day ).date();
    const QDate dt2 = qwtCeilDate( to, TimeDate::Day ).date();

    const int days = dt1.daysTo( dt2 );
    const int weeks = qCeil( days / 7.0 );

    if ( d_maxWeeks >= 0 )
    {
        if ( weeks > d_maxWeeks )
        {
            if ( days > 4 * maxSteps * 7 )
                return TimeDate::Month;
        }
        else
        {
            if ( days > maxSteps * 7 )
                return TimeDate::Week;
        }
    }
    else
    {
        if ( days > 4 * maxSteps * 7 )
            return TimeDate::Month;

        if ( days > maxSteps * 7 )
            return TimeDate::Week;
    }

    const QDateTime dh1 = qwtFloorDate( from, TimeDate::Hour );
    const QDateTime dh2 = qwtCeilDate( to, TimeDate::Hour );

    const int hours = dh1.secsTo( dh2 ) / 3600;
    if ( hours > maxSteps * 24 )
        return TimeDate::Day;

    const int seconds = from.secsTo( to );

    if ( seconds >= maxSteps * 3600 )
        return TimeDate::Hour;

    if ( seconds >= maxSteps * 60 )
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
            scaleDiv = divideToYears( min, max, 
                maxMajSteps, maxMinSteps, stepSize );
            break;
        }
        case TimeDate::Month:
        {
            scaleDiv = divideToMonths( min, max, 
                maxMajSteps, maxMinSteps, stepSize );
            break;
        }
        case TimeDate::Week:
        {
            scaleDiv = divideToWeeks( min, max, 
                maxMajSteps, maxMinSteps, stepSize );
            break;
        }
        case TimeDate::Day:
        {
            scaleDiv = divideToDays( min, max, 
                maxMajSteps, maxMinSteps, stepSize );
            break;
        }
        case TimeDate::Hour:
        {
            scaleDiv = divideToHours( min, max, 
                maxMajSteps, maxMinSteps, stepSize );
            break;
        }
        case TimeDate::Minute:
        {
            scaleDiv = divideToMinutes( min, max, 
                maxMajSteps, maxMinSteps, stepSize );
            break;
        }
        case TimeDate::Second:
        {
            scaleDiv = divideToSeconds( min, max, 
                maxMajSteps, maxMinSteps, stepSize );
            break;
        }
        case TimeDate::Millisecond:
        {
            scaleDiv = QwtLinearScaleEngine::divideScale( min, max,
                maxMajSteps, maxMinSteps, stepSize );

#if DEBUG_ENGINE >= 2
            qDebug() << "Millisecond: " << max - min;
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

        stepSize *= msSecond; // in ms
    }

    // align to step size
    const int s = qwtFloorToStep( from.time().second(), stepSize / msSecond );
    from.setTime( QTime( from.time().hour(), from.time().minute(), s ) );

    double minStepSize = 0.0;
    if ( maxMinSteps > 1 )
	{
        minStepSize = qwtStepSize( stepSize, maxMinSteps, 10 );
	}

    return qwtBuildScaleDiv( min, max, from, to, 
		stepSize, minStepSize, false );
}

QwtScaleDiv TimeScaleEngine::divideToMinutes( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
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

    double minStepSize = 0.0;
    if ( maxMinSteps > 1 )
    {
        static int limits[] = { 1, 2, 5, 10, 15, 20, 30, 60 };

        int numSteps = 0;
        if ( stepSize / msSecond / maxMinSteps > 60 )
        {
            numSteps = qwtStepCount( 
                stepSize / msMinute, maxMinSteps, 
                limits, sizeof( limits ) / sizeof( int ) );
        }
        else
        {
            numSteps = qwtStepCount( 
                stepSize / msSecond, maxMinSteps, 
                limits, sizeof( limits ) / sizeof( int ) );
        }

        if ( numSteps > 0 )
        {
            minStepSize = stepSize / numSteps;
        }
        else 
        {
            // fallback ???
            minStepSize = qwtStepSize( stepSize, maxMinSteps, 10 );
        }
    }

    return qwtBuildScaleDiv( min, max, from, to, 
		stepSize, minStepSize, false );
}

QwtScaleDiv TimeScaleEngine::divideToHours( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
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

    double minStepSize = 0.0;
    if ( maxMinSteps > 1 )
	{
		int numSteps = 0;

		if ( stepSize / msHour / maxMinSteps >= 1 )
		{
			static int limits[] = { 1, 2, 3, 4, 6, 12, 24, 48, 72 };

			numSteps = qwtStepCount(
				stepSize / msHour, maxMinSteps,
				limits, sizeof( limits ) / sizeof( int ) );
		}
		else
		{
			static int limits[] = { 1, 2, 5, 10, 15, 20, 30, 60 };

			numSteps = qwtStepCount(
				stepSize / msMinute, maxMinSteps,
				limits, sizeof( limits ) / sizeof( int ) );
		}

        if ( numSteps > 0 )
        {
            minStepSize = stepSize / numSteps;
        }
        else
        {
            // fallback ???
            minStepSize = qwtStepSize( stepSize, maxMinSteps, 10 );
        }
	}

    return qwtBuildScaleDiv( min, max, from, to, 
		stepSize, minStepSize, false );
}

QwtScaleDiv TimeScaleEngine::divideToDays( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
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

    double minStepSize = 0.0;
    if ( maxMinSteps > 1 )
	{
        int numSteps = 0;

        if ( stepSize / msDay / maxMinSteps >= 1 )
        {
            static int limits[] = { 1, 2, 3, 7, 14, 28 };

            numSteps = qwtStepCount(
                stepSize / msDay, maxMinSteps,
                limits, sizeof( limits ) / sizeof( int ) );
        }
        else
        {
            static int limits[] = { 1, 2, 3, 4, 6, 12, 24, 48, 72 };

            numSteps = qwtStepCount(
                stepSize / msHour, maxMinSteps,
                limits, sizeof( limits ) / sizeof( int ) );
        }

        if ( numSteps > 0 )
        {
            minStepSize = stepSize / numSteps;
        }
        else
        {
            // fallback ???
            minStepSize = qwtStepSize( stepSize, maxMinSteps, 10 );
        }
	}

    return qwtBuildScaleDiv( min, max, from, to, 
		stepSize, minStepSize, true );
}

QwtScaleDiv TimeScaleEngine::divideToWeeks( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
    QDateTime from = qwtToDateTime( min );
    from = qwtFloorDate( from, TimeDate::Week );

    QDateTime to = qwtToDateTime( max );
    to = qwtCeilDate( to, TimeDate::Week );

    if ( stepSize == 0.0 )
    {
        const int weeks = from.daysTo( to ) / 7;
        stepSize = qwtDivideWeeks( weeks, maxMajSteps );

#if DEBUG_ENGINE >= 2
        qDebug() << "Weeks: " << weeks << " -> " << stepSize;
#endif
        stepSize *= msWeek; // ms
    }

    // align to step size
    const int weeks1 = from.date().daysInYear() / 7;
    const int weeks2 = qwtFloorToStep( weeks1, stepSize / msWeek );

    QDateTime dt( QDate( from.date().year(), 1, 1 ) );
    dt.addDays( from.date().daysInYear() - ( weeks1 - weeks2 ) * 7 );

    from = dt;

    // calculate the minor step size
    double minStepSize = 0.0;
    if ( maxMinSteps > 1 )
    {
        const int daysInStep = qRound( stepSize / msWeek * 7 );

        if ( maxMinSteps >= daysInStep )
        {
            // we want to have one tick per day
            minStepSize = stepSize / daysInStep;
        }
        else
        {
            // when the stepSize is more than a week we want to
            // have a tick for each week

            const int stepSizeInWeeks = qRound( stepSize / msWeek );

            if ( stepSizeInWeeks <= maxMinSteps )
            {
                minStepSize = msWeek;
            }
            else
            {
                minStepSize = divideInterval( stepSizeInWeeks, maxMinSteps );
                minStepSize *= msWeek;
            }
        }
    }

    return qwtBuildScaleDiv( min, max, from, to, 
		stepSize, minStepSize, true );
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
    const int m = qwtFloorToStep( from.date().month() - 1, stepSize );
    from = QDateTime( QDate( from.date().year(), m + 1, 1 ) );

    QList<double> majorTicks;
    QList<double> mediumTicks;
    QList<double> minorTicks;

    for ( QDateTime dt = from; dt <= to; dt = dt.addMonths( stepSize ) )
    {
        const double value = qwtFromDateTime( dt );
        if ( value >= min && value <= max )
            majorTicks += value;

		// a) minor ticks for each week
		// b) minor ticks for each day, medium for each week

#if 1
    	if ( maxMinSteps >= 4 )
		{
			// weeks as minor ticks

			QDateTime wt0 = qwtCeilDate( dt, TimeDate::Week );
			if ( wt0 == dt )
				wt0 = wt0.addDays( 7 );

			const QDateTime dt2 = dt.addMonths( stepSize );

			QDateTime wt1 = qwtFloorDate( dt2, TimeDate::Week );
			if ( wt1 == dt2 )
				wt1 = wt1.addDays( -7 );
			
			for ( QDateTime wt = wt0; wt <= wt1; wt = wt.addDays( 7 ) )
			{
				minorTicks += qwtFromDateTime( wt );
			}
		}
#endif
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
        stepSize = qwtStepSize( years, maxMajSteps, 10 );

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

QwtTransform *TimeScaleEngine::transformation() const
{
    return new QwtNullTransform();
}

