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

class QwtDateTimeScaleDraw::PrivateData
{
public:
};

QwtDateTimeScaleDraw::QwtDateTimeScaleDraw()
{
	d_data = new PrivateData();
}

QwtDateTimeScaleDraw::~QwtDateTimeScaleDraw()
{
	delete d_data;
}

QwtText QwtDateTimeScaleDraw::label( double value ) const
{
    const QDateTime dt = QwtDate::toDateTime( value, Qt::LocalTime );

    // the format string should be cached !!!
    return dt.toString( format( scaleDiv() ) );
}

QString QwtDateTimeScaleDraw::format( const QwtScaleDiv &scaleDiv ) const
{
    const QList<double> ticks = scaleDiv.ticks( QwtScaleDiv::MajorTick );

    QList<QDateTime> dates;
    for ( int i = 0; i < ticks.size(); i++ )
        dates += QwtDate::toDateTime( ticks[i], Qt::LocalTime );

    return format( qwtIntervalType( dates ) );
}

QString QwtDateTimeScaleDraw::format( QwtDate::IntervalType intervalType ) const
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

