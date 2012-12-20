#include "timescaledraw.h"
#include <qdatetime.h>

static QwtDate::IntervalType qwtIntervalType( 
    const QList<QDateTime> &dateTimes )
{
    for ( int type = QwtDate::Second; type <= QwtDate::Year; type++ )
    {
        if ( type == QwtDate::Week )
            type++;

        const QwtDate::IntervalType intervalType = 
            static_cast<QwtDate::IntervalType>( type );

        for ( int i = 0; i < dateTimes.size(); i++ )
        {
            if ( QwtDate::floor( dateTimes[i], intervalType ) != dateTimes[i] )
            {
                return static_cast<QwtDate::IntervalType>( type - 1 );
            }
        }
    }

    return QwtDate::Year;
}

TimeScaleDraw::TimeScaleDraw()
{
}

TimeScaleDraw::~TimeScaleDraw()
{
}

QwtText TimeScaleDraw::label( double value ) const
{
    const QDateTime dt = QwtDate::toDateTime( value );

    // the format string should be cached !!!
    return dt.toString( format( scaleDiv() ) );
}

QString TimeScaleDraw::format( const QwtScaleDiv &scaleDiv ) const
{
    const QList<double> ticks = scaleDiv.ticks( QwtScaleDiv::MajorTick );

    QList<QDateTime> dates;
    for ( int i = 0; i < ticks.size(); i++ )
        dates += QwtDate::toDateTime( ticks[i] );

    return format( qwtIntervalType( dates ) );
}

QString TimeScaleDraw::format( QwtDate::IntervalType intervalType ) const
{
    QString format;

    switch( intervalType )
    {
        case QwtDate::Year:
        {
            format = "yyyy";
            break;
        }
        case QwtDate::Month:
        {
            format = "MMM yyyy";
            break;
        }
        case QwtDate::Week:
        case QwtDate::Day:
        {
            format = "ddd dd MMM yyyy";
            break;
        }
        case QwtDate::Hour:
        case QwtDate::Minute:
        {
            format = "hh:mm\nddd dd MMM yyyy";
            break;
        }
        case QwtDate::Second:
        {
            format = "hh:mm:ss\nddd dd MMM yyyy";
            break;
        }
        case QwtDate::Millisecond:
        default:
        {
            format = "hh:mm:ss:zzz\nddd dd MMM yyyy";
        }
    }

    return format;
}

