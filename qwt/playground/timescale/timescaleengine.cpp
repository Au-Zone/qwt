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
    d = up ? ::ceil( d ) : ::floor( d );

    return static_cast<int>( d * stepSize );
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

static QwtScaleDiv qwtDivideToSeconds( 
    const QDateTime &minDate, const QDateTime &maxDate,
    double stepSize, int maxMinSteps,
    QwtDate::IntervalType intervalType ) 
{
    // calculate the min step size
    double minStepSize = 0;

    if ( maxMinSteps > 1 ) 
    {
        minStepSize = qwtDivideMajorStep( stepSize, 
            maxMinSteps, intervalType );
    }

    bool daylightSaving = intervalType > QwtDate::Hour;
    if ( intervalType == QwtDate::Hour )
    {
        daylightSaving = stepSize > 1;
    }

    const double s = qwtMsecsForType( intervalType ) / 1000;
	const int secondsMajor = static_cast<int>( stepSize * s );
	const double secondsMinor = minStepSize * s;
	
    // UTC excludes daylight savings. So from the difference
    // of a date and its UTC counterpart we can find out
    // the daylight saving hours

    const double utcOffset = QwtDate::utcOffset( minDate );

    QList<double> majorTicks;
    QList<double> minorTicks;
    QList<double> mediumTicks;

    for ( QDateTime dt = minDate; dt <= maxDate; 
        dt = dt.addSecs( secondsMajor ) )
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
        {
            const double offset = utcOffset - QwtDate::utcOffset( dt );
            majorValue += offset * 1000.0;
        }

        if ( majorTicks.isEmpty() || majorTicks.last() != majorValue )
            majorTicks += majorValue;

        if ( secondsMinor > 0.0 )
        {
            const int numSteps = qFloor( secondsMajor / secondsMinor );

            for ( int i = 1; i < numSteps; i++ )
            {
                const QDateTime mt = dt.addMSecs( 
                    qRound64( i * secondsMinor * 1000 ) );

                double minorValue = QwtDate::toDouble( mt );
                if ( daylightSaving )
                {
                    const double offset = utcOffset - QwtDate::utcOffset( mt );
                    minorValue += offset * 1000.0;
                }

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

static QwtScaleDiv qwtDivideToMonths( 
    QDateTime &minDate, const QDateTime &maxDate,
    double stepSize, int maxMinSteps ) 
{
    // months are intervals with non 
    // equidistant ( in ms ) steps: we have to build the 
    // scale division manually

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
                stepSize, maxMinSteps, QwtDate::Month );
        }
    }

    QList<double> majorTicks;
    QList<double> mediumTicks;
    QList<double> minorTicks;

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

    QwtScaleDiv scaleDiv;
    scaleDiv.setInterval( QwtDate::toDouble( minDate ),
        QwtDate::toDouble( maxDate ) );

    scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );
    scaleDiv.setTicks( QwtScaleDiv::MediumTick, mediumTicks );
    scaleDiv.setTicks( QwtScaleDiv::MinorTick, minorTicks );

    return scaleDiv;
}

static QwtScaleDiv qwtDivideToYears( 
    const QDateTime &minDate, const QDateTime &maxDate,
    double stepSize, int maxMinSteps ) 
{
    QList<double> majorTicks;
    QList<double> mediumTicks;
    QList<double> minorTicks;

    double minStepSize = 0.0;

    if ( maxMinSteps > 1 )
    {
        minStepSize = qwtDivideMajorStep( 
            stepSize, maxMinSteps, QwtDate::Year );
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

        majorTicks += QwtDate::toDouble( dt );

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

    QwtScaleDiv scaleDiv;
    scaleDiv.setInterval( QwtDate::toDouble( minDate ),
        QwtDate::toDouble( maxDate ) );

    scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );
    scaleDiv.setTicks( QwtScaleDiv::MediumTick, mediumTicks );
    scaleDiv.setTicks( QwtScaleDiv::MinorTick, minorTicks );

    return scaleDiv;
}

class QwtDateTimeScaleEngine::PrivateData
{
public:
    PrivateData( Qt::TimeSpec spec ):
		timeSpec( spec ),
		utcOffset( 0 ),
		week0Type( QwtDate::FirstThursday ),
		maxWeeks( 4 )
    {
    }

    Qt::TimeSpec timeSpec;
    int utcOffset;
	QwtDate::Week0Type week0Type;
    int maxWeeks;
};      


QwtDateTimeScaleEngine::QwtDateTimeScaleEngine( Qt::TimeSpec timeSpec ):
	QwtLinearScaleEngine( 10 )
{
	d_data = new PrivateData( timeSpec );
}

QwtDateTimeScaleEngine::~QwtDateTimeScaleEngine()
{
	delete d_data;
}

void QwtDateTimeScaleEngine::setTimeSpec( Qt::TimeSpec timeSpec )
{
    d_data->timeSpec = timeSpec;
}

Qt::TimeSpec QwtDateTimeScaleEngine::timeSpec() const
{
    return d_data->timeSpec;
}

void QwtDateTimeScaleEngine::setUtcOffset( int seconds )
{
	d_data->utcOffset = seconds;
}

int QwtDateTimeScaleEngine::utcOffset() const
{
	return d_data->utcOffset;
}

void QwtDateTimeScaleEngine::setWeek0Type( QwtDate::Week0Type week0Type )
{
	d_data->week0Type = week0Type;
}

QwtDate::Week0Type QwtDateTimeScaleEngine::week0Type() const
{
	return d_data->week0Type;
}

void QwtDateTimeScaleEngine::setMaxWeeks( int weeks )
{
    d_data->maxWeeks = qMax( weeks, 0 );
}

int QwtDateTimeScaleEngine::maxWeeks() const
{
    return d_data->maxWeeks;
}

QwtDate::IntervalType QwtDateTimeScaleEngine::intervalType( 
    const QDateTime &minDate, const QDateTime &maxDate, 
    int maxSteps ) const
{
    const double jdMin = minDate.date().toJulianDay();
    const double jdMax = maxDate.date().toJulianDay();

    if ( ( jdMax - jdMin ) / 365 > maxSteps )
        return QwtDate::Year;

    const int months = qwtRoundedIntervalWidth( minDate, maxDate, QwtDate::Month );
    if ( months > maxSteps * 6 )
        return QwtDate::Year;

    const int days = qwtRoundedIntervalWidth( minDate, maxDate, QwtDate::Day );
    const int weeks = qwtRoundedIntervalWidth( minDate, maxDate, QwtDate::Week );

    if ( weeks > d_data->maxWeeks )
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

void QwtDateTimeScaleEngine::autoScale( int maxNumSteps,
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

    const QDateTime from = toDateTime( interval.minValue() );
    const QDateTime to = toDateTime( interval.maxValue() );

    if ( from.isValid() && to.isValid() )
    {
        if ( maxNumSteps < 1 )
            maxNumSteps = 1;

        const QwtDate::IntervalType intvType = 
            intervalType( from, to, maxNumSteps );

        stepSize = divideInterval( from, to, intvType, maxNumSteps );

        if ( stepSize != 0.0 && !testAttribute( QwtScaleEngine::Floating ) )
        {
            interval = align( interval, stepSize );

            const QDateTime d1 = QwtDate::floor(
                toDateTime( interval.minValue() ), intvType );

            const QDateTime d2 = QwtDate::ceil(
                toDateTime( interval.maxValue() ), intvType );

            interval.setMinValue( QwtDate::toDouble( d1 ) );
            interval.setMaxValue( QwtDate::toDouble( d2 ) );
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

double QwtDateTimeScaleEngine::divideInterval( 
    const QDateTime &from, const QDateTime &to,
    QwtDate::IntervalType intervalType, int numSteps ) const
{
    const double width = qwtIntervalWidth( from, to, intervalType );
    
    double stepSize = QwtScaleArithmetic::divideInterval( width, numSteps, 10 );

    return stepSize * qwtMsecsForType( intervalType );
}

QwtScaleDiv QwtDateTimeScaleEngine::divideScale( double x1, double x2,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
    if ( maxMajSteps < 1 )
        maxMajSteps = 1;

    stepSize = qAbs( stepSize );

    const double min = qMin( x1, x2 );
    const double max = qMax( x1, x2 );

    const QDateTime from = toDateTime( min );
    const QDateTime to = toDateTime( max );

    if ( from == to )
        return QwtScaleDiv();

    if ( stepSize > 0.0 )
    {
        // as interval types above hours are not equidistant
        // ( even days might have 23/25 hours because of daylight saving )
        // the stepSize is used as a hint only

        maxMajSteps = qCeil( ( max - min ) / stepSize );
    }

    const QwtDate::IntervalType intvType = 
        intervalType( from, to, maxMajSteps );

#if DEBUG_ENGINE >= 1
    qDebug() << "Divide: " << min << max << from << to << maxMajSteps 
        << "Type: " << ( int )intvType;
#endif

    QwtScaleDiv scaleDiv;

    if ( intvType == QwtDate::Millisecond )
    {
        // for milliseconds and below we use the decimal system
        scaleDiv = QwtLinearScaleEngine::divideScale( min, max,
            maxMajSteps, maxMinSteps, stepSize );
    }
    else
    {
        const QDateTime minDate = QwtDate::floor( from, intvType );
        const QDateTime maxDate = QwtDate::ceil( to, intvType );

        scaleDiv = divideTo( minDate, maxDate, 
            maxMajSteps, maxMinSteps, intvType );

        // scaleDiv has been calculated from an extended interval
        // adjusted to the step size. We have to shrink it again.

        scaleDiv = scaleDiv.bounded( min, max );
    }

    if ( x1 > x2 )
        scaleDiv.invert();

#if DEBUG_ENGINE >= 3
    qDebug() << scaleDiv;
#endif

    return scaleDiv;
}

QwtScaleDiv QwtDateTimeScaleEngine::divideTo( 
    const QDateTime &minDate, const QDateTime &maxDate,
    int maxMajorSteps, int maxMinorSteps,
    QwtDate::IntervalType intervalType ) const
{
    // calculate the step size
    const double stepSize = qwtDivideScale( 
        qwtIntervalWidth( minDate, maxDate, intervalType ), 
        maxMajorSteps, intervalType );

    // align minDate to the step size
    QDateTime dt0 = alignDate( minDate, stepSize, intervalType, false );
    if ( !dt0.isValid() )
    {
        // the floored date is out of the range of a 
        // QDateTime - we ceil instead.
        dt0 = alignDate( minDate, stepSize, intervalType, true );
    }

    QwtScaleDiv scaleDiv;

    if ( intervalType <= QwtDate::Week )
    {
        scaleDiv = qwtDivideToSeconds( dt0, maxDate, 
            stepSize, maxMinorSteps, intervalType );
    }
    else
    {
        if( intervalType == QwtDate::Month )
        {
            scaleDiv = qwtDivideToMonths( dt0, maxDate,
                stepSize, maxMinorSteps );
        }
        else if ( intervalType == QwtDate::Year )
        {
            scaleDiv = qwtDivideToYears( dt0, maxDate,
                stepSize, maxMinorSteps );
        }
    }

#if 1
    if ( dt0 > minDate && maxMinorSteps > 1 )
	{
		// we have to add minor ticks for the interval 
		// [ dt0 - stepSize -> dt0 ] we have lost because of 
		// ceiling instead of flooring. As we are usually
        // habe QwtDate::Year intervals QwtLinearScaleEngine
        // will result in something no too bad

		const double s0 = QwtDate::toDouble( dt0 );

		QList<double> majorTicks;
		majorTicks += s0 - stepSize;
		majorTicks += s0;

		QList<double> minorTicks;
		QList<double> mediumTicks;

		QwtLinearScaleEngine::buildMinorTicks(
			majorTicks, maxMinorSteps, stepSize,
			minorTicks, mediumTicks );

		if ( minorTicks.size() != 0 )
		{
			minorTicks += scaleDiv.ticks( QwtScaleDiv::MinorTick );
			scaleDiv.setTicks( QwtScaleDiv::MinorTick, minorTicks );
		}

        if ( mediumTicks.size() != 0 )
        {
            mediumTicks += scaleDiv.ticks( QwtScaleDiv::MediumTick );
            scaleDiv.setTicks( QwtScaleDiv::MediumTick, mediumTicks );
        } 
	}
#endif

    return scaleDiv;
}

QDateTime QwtDateTimeScaleEngine::alignDate( 
	const QDateTime &dateTime, double stepSize, 
	QwtDate::IntervalType intervalType, bool up ) const
{
    // what about: (year == 1582 && month == 10 && day > 4 && day < 15) ??

	QDateTime dt = dateTime;
	dt.setTime( QTime( 0, 0, 0 ) );

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
			// what date do we expect f.e. from an alignment of 10 days ??
#if 0
            const int d = qwtAlignValue(
                dateTime.date().dayOfYear(), stepSize, up );

            dt.setDate( QDate( dateTime.date().year(), 1, 1 ) );
            dt = dt.addDays( d );
#else
			const int d = qwtAlignValue( 
				dateTime.date().day(), stepSize, up );

			dt.setDate( QDate( dateTime.date().year(), 
				dateTime.date().month(), 1 ) );
			dt = dt.addDays( d - 1 );
#endif
        }
        case QwtDate::Week:
        {
            const QDate date = QwtDate::dateOfWeek0(
                dateTime.date().year(), d_data->week0Type );

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

QDateTime QwtDateTimeScaleEngine::toDateTime( double value ) const
{
    QDateTime dt = QwtDate::toDateTime( value, d_data->timeSpec );
    if ( !dt.isValid() )
    {
        const QDate date = ( value <= 0.0 ) 
			? QwtDate::minDate() : QwtDate::maxDate();

        dt = QDateTime( date, QTime( 0, 0 ), d_data->timeSpec );
    }

	if ( d_data->timeSpec == Qt::OffsetFromUTC )
		dt.setUtcOffset( d_data->utcOffset );

    return dt;
}

