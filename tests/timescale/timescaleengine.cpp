#include "timescaleengine.h"
#include <qwt_interval.h>
#include <qwt_math.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <cassert>

#define DEBUG_ENGINE 0

static inline double qwtDivideInterval( double intervalSize, int numSteps, 
	const double limits[], size_t numLimits )
{
    const double v = intervalSize / numSteps;

	for ( uint i = 0; i < numLimits - 1; i++ )
	{
		if ( v <= limits[i] )
			return limits[i];
	}

	return limits[ numSteps - 1 ];
}

static inline double qwtDivideMonths(
    double intervalSize, int numSteps )
{
	static double limits[] = { 1, 2, 3, 4, 6, 12 };

	return qwtDivideInterval( intervalSize, numSteps,
		limits, sizeof( limits ) / sizeof( double ) );
}

static inline double qwtDivideWeeks(
    double intervalSize, int numSteps )
{
	static double limits[] = { 1, 2, 4, 8, 12, 26, 52 };

	return qwtDivideInterval( intervalSize, numSteps,
		limits, sizeof( limits ) / sizeof( double ) );
}

static inline double qwtDivideDays(
    double intervalSize, int numSteps )
{
    const double v = intervalSize / numSteps;
	if ( v <= 5.0 )
		return qCeil( v );

	return qCeil( v / 7.0 ) * 7.0;
}

static inline double qwtDivideHours(
    double intervalSize, int numSteps )
{
	static double limits[] = { 1, 2, 3, 4, 6, 12, 24 };

	return qwtDivideInterval( intervalSize, numSteps,
		limits, sizeof( limits ) / sizeof( double ) );
}

static inline double qwtDivide60(
    double intervalSize, int numSteps )
{
	static double limits[] = { 1, 2, 5, 10, 15, 20, 30, 60 };
    
	return qwtDivideInterval( intervalSize, numSteps,
		limits, sizeof( limits ) / sizeof( double ) );
}   

TimeScaleEngine::TimeScaleEngine()
{
}

TimeScaleEngine::~TimeScaleEngine()
{
}


TimeDate::IntervalType TimeScaleEngine::intervalType( double min, double max ) const
{
	const QDateTime from = qwtToDateTime( min );
	const QDateTime to = qwtToDateTime( max );

	const int days = from.daysTo( to );

	if ( days > 2 * 365 )
		return TimeDate::Year;

	if ( days >= 62 )
		return TimeDate::Month;

	if ( days > 10 )
	{
		int month1 = from.date().month();
		int month2 = to.date().month();
		if ( month2 < month1 )
			month2 += 12;

		return ( month2 - month1 >= 2 ) ? TimeDate::Month : TimeDate::Week;
	}

	if ( days > 2 )
		return TimeDate::Day;

	const int seconds = from.secsTo( to );

	if ( seconds > 3 * 3600 )
		return TimeDate::Hour;

	if ( seconds > 3 * 60 )
		return TimeDate::Minute;

	if ( seconds >= 2 )
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

	double min = qMin( x1, x2 );
	double max = qMax( x1, x2 );

	if ( !qwtToDateTime( min ).isValid() )
	{
		qDebug() << "Invalid: " << min << qwtToDateTime( min );
		return QwtScaleDiv();
	}

	if ( !qwtToDateTime( max ).isValid() )
	{
		qDebug() << "Invalid: " << max << qwtToDateTime( max );
		return QwtScaleDiv();
	}

#if DEBUG_ENGINE >= 1
	qDebug() << "Divide: " << min << max << qwtToDateTime( min ) << qwtToDateTime( max ) << maxMajSteps;
#endif
	QwtScaleDiv scaleDiv;

	switch ( intervalType( min, max ) )
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
			scaleDiv = QwtLinearScaleEngine::divideScale( x1, x2,
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

#if DEBUG_ENGINE >= 1
	qDebug() << scaleDiv;
#endif
	return scaleDiv;
}


QwtScaleDiv TimeScaleEngine::divideToSeconds( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
	Q_UNUSED( maxMinSteps );

	QDateTime from = qwtToDateTime( min );
	from = qwtCeilDate( from, TimeDate::Second );

	const QDateTime to = qwtToDateTime( max );

    stepSize = qAbs( stepSize );
    if ( stepSize == 0.0 )
    {
		const QDateTime dt = qwtFloorDate( to, TimeDate::Second );

        stepSize = qwtDivide60( from.secsTo( dt ), maxMajSteps );

#if DEBUG_ENGINE >= 2
		qDebug() << "Seconds: " << from.secsTo( dt ) << " -> " << stepSize;
#endif
    }

	QList<double> majorTicks;
	while ( from <= to )
	{
		majorTicks += qwtFromDateTime( from );
		from = from.addMSecs( qRound64( stepSize * 1000 ) );
	}

	QwtScaleDiv scaleDiv;
	scaleDiv.setInterval( min, max );
	scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );

	return scaleDiv;
}

QwtScaleDiv TimeScaleEngine::divideToMinutes( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
	Q_UNUSED( maxMinSteps );

	QDateTime from = qwtToDateTime( min );
	from = qwtCeilDate( from, TimeDate::Minute );

	const QDateTime to = qwtToDateTime( max );

    stepSize = qAbs( stepSize );
    if ( stepSize == 0.0 )
    {
		const QDateTime dt = qwtFloorDate( to, TimeDate::Minute );

        stepSize = qwtDivide60( from.secsTo( dt ) / 60, maxMajSteps );
#if DEBUG_ENGINE >= 2
		qDebug() << "Minutes: " << from.secsTo( dt ) / 60 << " -> " << stepSize;
#endif
    }

	QList<double> majorTicks;
	while ( from <= to )
	{
		majorTicks += qwtFromDateTime( from );
		from = from.addSecs( qRound( 60 * stepSize ) );
	}

	QwtScaleDiv scaleDiv;
	scaleDiv.setInterval( min, max );
	scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );

	return scaleDiv;
}

QwtScaleDiv TimeScaleEngine::divideToHours( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
	Q_UNUSED( maxMinSteps );

	QDateTime from = qwtToDateTime( min );
	from = qwtCeilDate( from, TimeDate::Hour );

	const QDateTime to = qwtToDateTime( max );

    stepSize = qAbs( stepSize );
    if ( stepSize == 0.0 )
    {
		const QDateTime dt = qwtFloorDate( to, TimeDate::Hour );
        stepSize = qwtDivideHours( from.secsTo( dt ) / 3600, maxMajSteps );

#if DEBUG_ENGINE >= 2
		qDebug() << "Hours: " << from.secsTo( dt ) / 3600 << " -> " << stepSize;
#endif
    }

	QList<double> majorTicks;
	while ( from <= to )
	{
		majorTicks += qwtFromDateTime( from );
		from = from.addSecs( 3600 * stepSize );
	}

	QwtScaleDiv scaleDiv;
	scaleDiv.setInterval( min, max );
	scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );

	return scaleDiv;
}

QwtScaleDiv TimeScaleEngine::divideToDays( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
	Q_UNUSED( maxMinSteps );

	QDateTime from = qwtToDateTime( min );
	from = qwtCeilDate( from, TimeDate::Day );

	const QDateTime to = qwtToDateTime( max );

    stepSize = qAbs( stepSize );
    if ( stepSize == 0.0 )
    {
		const QDateTime dt = qwtFloorDate( to, TimeDate::Day );
        stepSize = qwtDivideDays( from.daysTo( dt ), maxMajSteps );

#if DEBUG_ENGINE >= 2
		qDebug() << "Days: " << from.daysTo( dt ) << " -> " << stepSize;
#endif
    }

	QList<double> majorTicks;
	while ( from <= to )
	{
		majorTicks += qwtFromDateTime( from );
		from = from.addSecs( qRound( 24 * 3600 * stepSize ) );
	}

	QwtScaleDiv scaleDiv;
	scaleDiv.setInterval( min, max );
	scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );

	return scaleDiv;
}

QwtScaleDiv TimeScaleEngine::divideToWeeks( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
	Q_UNUSED( maxMinSteps );

	QDateTime from = qwtToDateTime( min );
	from = qwtCeilDate( from, TimeDate::Week );

	const QDateTime to = qwtToDateTime( max );

    stepSize = qAbs( stepSize );
    if ( stepSize == 0.0 )
    {
		const QDateTime dt = qwtFloorDate( to, TimeDate::Week );

        const int weeks = qFloor( from.daysTo( dt ) / 7 );
        stepSize = qwtDivideWeeks( weeks, maxMajSteps );

#if DEBUG_ENGINE >= 2
		qDebug() << "Weeks: " << weeks << " -> " << stepSize;
#endif
    }

	QList<double> majorTicks;
	while ( from <= to )
	{
		majorTicks += qwtFromDateTime( from );
		from = from.addDays( qCeil( stepSize * 7 ) );
	}

	QwtScaleDiv scaleDiv;
	scaleDiv.setInterval( min, max );
	scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );

	return scaleDiv;
}

QwtScaleDiv TimeScaleEngine::divideToMonths( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
	Q_UNUSED( maxMinSteps );

	if ( min > max )
		qSwap( min, max );

	QDateTime from = qwtToDateTime( min );
	from = qwtCeilDate( from, TimeDate::Month );

	const QDateTime to = qwtToDateTime( max );

    stepSize = qAbs( stepSize );
    if ( stepSize == 0.0 )
    {
		const QDateTime dt = qwtFloorDate( to, TimeDate::Month );

		const double months = 12 * ( dt.date().year() - from.date().year() ) 
			+ dt.date().month() - from.date().month();

        stepSize = qwtDivideMonths( months, maxMajSteps );

#if DEBUG_ENGINE >= 2
		qDebug() << "Months: " << months << " -> " << stepSize;
#endif
    }

	stepSize = qMax( stepSize, 1.0 );

	QList<double> majorTicks;
    while ( from <= to )
    {
        majorTicks += qwtFromDateTime( from );
        from = from.addMonths( stepSize );
    }

	QwtScaleDiv scaleDiv;
	scaleDiv.setInterval( min, max );
	scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );

	return scaleDiv;
}

QwtScaleDiv TimeScaleEngine::divideToYears( double min, double max,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
	Q_UNUSED( maxMinSteps );

	if ( min > max )
		qSwap( min, max );

	QDateTime from = qwtToDateTime( min );
	from = qwtCeilDate( from, TimeDate::Year );

	const QDateTime to = qwtToDateTime( max );

    stepSize = qAbs( stepSize );
    if ( stepSize == 0.0 )
    {
		const QDateTime dt = qwtFloorDate( to, TimeDate::Year );

		const int years = dt.date().year() - from.date().year();
        stepSize = divideInterval( years, maxMajSteps );

#if DEBUG_ENGINE >= 2
		qDebug() << "Years: " << dt.date() << from.date() << years << " -> " << stepSize;
#endif
    }

	QList<double> majorTicks;
    while ( from <= to )
	{
        majorTicks += qwtFromDateTime( from );
        from = from.addMonths( qRound( stepSize * 12 ) );
    }	

	QwtScaleDiv scaleDiv;
	scaleDiv.setInterval( min, max );
	scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );

	return scaleDiv;
}

QwtScaleTransformation *TimeScaleEngine::transformation() const
{
	return QwtLinearScaleEngine::transformation();
}

