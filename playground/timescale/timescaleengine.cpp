#include "timescaleengine.h"
#include "timeinterval.h"
#include <qwt_math.h>
#include <qwt_transform.h>
#include <qdatetime.h>
#include <qdebug.h>

#define DEBUG_ENGINE 0

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

static int qwtDivideInterval( int intervalSize, int numSteps, 
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

static int qwtDivideScale( int intervalSize, int numSteps,
    TimeDate::IntervalType intervalType )
{
    if ( intervalType != TimeDate::Day )
    {
        if ( ( intervalSize > numSteps ) && 
            ( intervalSize <= 2 * numSteps ) )
        {
            return 2;
        }
    }

    int stepSize;

    switch( intervalType )
    {
        case TimeDate::Second:
        case TimeDate::Minute:
        {
            static int limits[] = { 1, 2, 5, 10, 15, 20, 30, 60 };
    
            stepSize = qwtDivideInterval( intervalSize, numSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            break;
        }
        case TimeDate::Hour:
        {
            static int limits[] = { 1, 2, 3, 4, 6, 12, 24 };
    
            stepSize = qwtDivideInterval( intervalSize, numSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            break;
        }
        case TimeDate::Day:
        {
            const double v = intervalSize / double( numSteps );
            if ( v <= 5.0 )
                stepSize = qCeil( v );
            else
                stepSize = qCeil( v / 7 ) * 7;

            break;
        }
        case TimeDate::Week:
        {
            static int limits[] = { 1, 2, 4, 8, 12, 26, 52 };

            stepSize = qwtDivideInterval( intervalSize, numSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            break;
        }
        case TimeDate::Month:
        {
            static int limits[] = { 1, 2, 3, 4, 6, 12 };

            stepSize = qwtDivideInterval( intervalSize, numSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            break;
        }
        case TimeDate::Year:
        case TimeDate::Millisecond:
        default:
        {
            stepSize = QwtScaleArithmetic::divideInterval(
                intervalSize, numSteps, 10 );
        }
    }

    return stepSize;
}

static double qwtDivideMajorStep( int stepSize, int maxMinSteps,
    TimeDate::IntervalType intervalType )
{
    double minStepSize = 0.0;

    switch( intervalType )
    {
        case TimeDate::Second:
        {
            minStepSize = qwtStepSize( stepSize, maxMinSteps, 10 );
            if ( minStepSize == 0.0 )
                minStepSize = 0.5 * stepSize;

            break;
        }
        case TimeDate::Minute:
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
        case TimeDate::Hour:
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
        case TimeDate::Day:
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
        case TimeDate::Week:
        {
            const int daysInStep = qRound( stepSize * 7 );

            if ( maxMinSteps >= daysInStep )
            {
                // we want to have one tick per day
                minStepSize = 1.0 / 7.0;
            }
            else
            {
                // when the stepSize is more than a week we want to
                // have a tick for each week

                const int stepSizeInWeeks = qRound( stepSize );

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
        case TimeDate::Month:
        {
            // fractions of months doesn't make any sense

            maxMinSteps = qMin( stepSize, maxMinSteps );

            static int limits[] = { 1, 2, 3, 4, 6, 12 };

            int numSteps = qwtStepCount( stepSize, maxMinSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            if ( numSteps > 0 )
                minStepSize = double( stepSize ) / numSteps;

            break;
        }
        case TimeDate::Year:
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

    if ( intervalType != TimeDate::Month
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
static QwtScaleDiv qwtBuildScaleDiv( const TimeInterval& interval,
    int stepSize, double minStepSize, bool daylightSaving )
{
    // UTC excludes daylight savings. So from the difference
    // of a date and its UTC counterpart we can find out
    // the daylight saving hours

    const int hourDST = qwtHoursUTC( interval.minDate() );

    QList<double> majorTicks;
    QList<double> minorTicks;
    QList<double> mediumTicks;

    for ( QDateTime dt = interval.minDate(); dt <= interval.maxDate(); 
        dt = dt.addSecs( stepSize ) )
    {
        if ( !dt.isValid() )
        {
#if DEBUG_ENGINE >= 1
            qDebug() << "Invalid date in: " << interval.minDate()
                << interval.maxDate();
#endif
            break;
        }

        double majorValue = qwtFromDateTime( dt );

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

                double minorValue = qwtFromDateTime( mt );
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

    scaleDiv.setInterval( qwtFromDateTime( interval.minDate() ),
        qwtFromDateTime( interval.maxDate() ) );

    scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );
    scaleDiv.setTicks( QwtScaleDiv::MediumTick, mediumTicks );
    scaleDiv.setTicks( QwtScaleDiv::MinorTick, minorTicks );

    return scaleDiv;
}

TimeScaleEngine::TimeScaleEngine():
    d_timeSpec( Qt::LocalTime ),
    d_maxWeeks( 4 )
{
}

TimeScaleEngine::~TimeScaleEngine()
{
}

void TimeScaleEngine::setMaxWeeks( int weeks )
{
    d_maxWeeks = qMax( weeks, 0 );
}

int TimeScaleEngine::maxWeeks() const
{
    return d_maxWeeks;
}

TimeDate::IntervalType TimeScaleEngine::intervalType( 
    double min, double max, int maxSteps ) const
{
    const TimeInterval interval( qwtToDateTime( min ), qwtToDateTime( max ) );

    const int months = interval.roundedWidth( TimeDate::Month );
    if ( months > maxSteps * 6 )
        return TimeDate::Year;

    const int days = interval.roundedWidth( TimeDate::Day );
    const int weeks = interval.roundedWidth( TimeDate::Week );

    if ( weeks > d_maxWeeks )
    {
        if ( days > 4 * maxSteps * 7 )
            return TimeDate::Month;
    }

    if ( days > maxSteps * 7 )
        return TimeDate::Week;

    const int hours = interval.roundedWidth( TimeDate::Hour );
    if ( hours > maxSteps * 24 )
        return TimeDate::Day;

    const int seconds = interval.roundedWidth( TimeDate::Second );

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

    if ( stepSize > 0.0 )
    {
        // as interval types above hours are not equidistant
        // ( even days might have 23/25 hours because of daylight saving )
        // the stepSize is used as a hint only

        maxMajSteps = qCeil( ( max - min ) / stepSize );
    }

    const TimeDate::IntervalType type = intervalType( min, max, maxMajSteps );

#if DEBUG_ENGINE >= 1
    qDebug() << "Divide: " << min << max << from << to << maxMajSteps 
        << "Type: " << ( int )type;
#endif

    QwtScaleDiv scaleDiv;

    if ( type == TimeDate::Millisecond )
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
    TimeDate::IntervalType intervalType ) const
{
    TimeInterval interval( min, max );

    // round the interval
    interval = interval.rounded( intervalType );

    // calculate the step size
    const int stepSize = qwtDivideScale( interval.width( intervalType ), 
        maxMajSteps, intervalType );

    // align to step size
    interval = interval.adjusted( stepSize, intervalType );

    QwtScaleDiv scaleDiv;

    if ( intervalType <= TimeDate::Week )
    {
        // calculate the min step size
        double minStepSize = 0;

        if ( maxMinSteps > 1 ) 
        {
            minStepSize = qwtDivideMajorStep( stepSize, maxMinSteps, intervalType );
        }

        static const int seconds[] =
            { 1, 60, 60 * 60, 24 * 60 * 60, 7 * 24 * 60 * 60 };

        const int s = seconds[ intervalType - TimeDate::Second ];

        bool daylightSaving = intervalType > TimeDate::Hour;
        if ( intervalType == TimeDate::Hour )
        {
            daylightSaving = stepSize > 1;
        }
    
        scaleDiv = qwtBuildScaleDiv( interval,
            stepSize * s, minStepSize * s, daylightSaving );
    }
    else
    {
        // months and years are intervals with non equidistant ( in ms ) steps:
        // we have to build the scale division manually

        QList<double> majorTicks;
        QList<double> mediumTicks;
        QList<double> minorTicks;

        if( intervalType == TimeDate::Month )
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

                
            for ( QDateTime dt = interval.minDate(); 
                dt <= interval.maxDate(); dt = dt.addMonths( stepSize ) )
            {
                if ( !dt.isValid() )
                {   
#if DEBUG_ENGINE >= 1
                    qDebug() << "Invalid date in: " << interval.minDate()
                        << interval.maxDate();
#endif  
                    break;
                }   

                majorTicks += qwtFromDateTime( dt );

                if ( minStepDays > 0 )
                {
                    for ( int days = minStepDays; 
                        days < 30; days += minStepDays )
                    {
                        const double tick = qwtFromDateTime( dt.addDays( days ) );
 
                        if ( days == 15 && minStepDays != 15 )
                            mediumTicks += tick;
                        else
                            minorTicks += tick;
                    }
                }
                else if ( minStepSize > 0.0 )
                {
                    const int numMinorSteps = qRound( stepSize / minStepSize );

                    for ( int i = 1; i < numMinorSteps; i++ )
                    {
                        const double minorValue =
                            qwtFromDateTime( dt.addMonths( i * minStepSize ) );

                        if ( ( numMinorSteps % 2 == 0 ) && ( i == numMinorSteps / 2 ) )
                            mediumTicks += minorValue;
                        else
                            minorTicks += minorValue;
                    }
                }
            }
        }
        else if ( intervalType == TimeDate::Year )
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

            bool dateBC = interval.minDate().date().year() < -1;

            for ( QDateTime dt = interval.minDate(); dt <= interval.maxDate();
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
                    qDebug() << "Invalid date in: " << interval.minDate()
                        << interval.maxDate();
#endif
                    break;
                }

                majorTicks += qwtFromDateTime( dt );

                for ( int i = 1; i < numMinorSteps; i++ )
                {
                    const int months = qRound( i * 12 * minStepSize );
                    const double minorValue = 
                        qwtFromDateTime( dt.addMonths( months ) );

                    const bool isMedium = ( numMinorSteps > 2 ) &&
                        ( numMinorSteps % 2 == 0 ) &&
                        ( i == numMinorSteps / 2 );

                    if ( isMedium )
                        mediumTicks += minorValue;
                    else
                        minorTicks += minorValue;
                }
            }   
        }

        scaleDiv.setInterval( qwtFromDateTime( interval.minDate() ),
            qwtFromDateTime( interval.maxDate() ) );

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

