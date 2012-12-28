#include "qwt_timescale_draw.h"

class QwtDateTimeScaleDraw::PrivateData
{
public:
    PrivateData( Qt::TimeSpec spec ):
        timeSpec( spec ),
        utcOffset( 0 ),
        week0Type( QwtDate::FirstThursday )
    {
    }

    Qt::TimeSpec timeSpec;
    int utcOffset;
    QwtDate::Week0Type week0Type;
};

QwtDateTimeScaleDraw::QwtDateTimeScaleDraw( Qt::TimeSpec timeSpec )
{
    d_data = new PrivateData( timeSpec );
}

QwtDateTimeScaleDraw::~QwtDateTimeScaleDraw()
{
    delete d_data;
}

void QwtDateTimeScaleDraw::setTimeSpec( Qt::TimeSpec timeSpec )
{
    d_data->timeSpec = timeSpec;
}

void QwtDateTimeScaleDraw::setUtcOffset( int seconds )
{
    d_data->utcOffset = seconds;
}

int QwtDateTimeScaleDraw::utcOffset() const
{
    return d_data->utcOffset;
}

void QwtDateTimeScaleDraw::setWeek0Type( QwtDate::Week0Type week0Type )
{
    d_data->week0Type = week0Type;
}

QwtDate::Week0Type QwtDateTimeScaleDraw::week0Type() const
{
    return d_data->week0Type;
}

QwtText QwtDateTimeScaleDraw::label( double value ) const
{
    // fmt should be cached !!!
    const QString fmt = format( intervalType( scaleDiv() ) );
    return toDateTime( value ).toString( fmt );
}

QwtDate::IntervalType QwtDateTimeScaleDraw::intervalType( 
    const QwtScaleDiv &scaleDiv ) const
{
    const QList<double> ticks = scaleDiv.ticks( QwtScaleDiv::MajorTick );

    int intvType = QwtDate::Year;

    for ( int i = 0; i < ticks.size(); i++ )
    {
        const QDateTime dt = toDateTime( ticks[i] );
        for ( int j = QwtDate::Second; j <= intvType; j++ )
        {
#if 1
            if ( j == QwtDate::Week )
                continue;
#endif

            const QDateTime dt0 = QwtDate::floor( dt, 
                static_cast<QwtDate::IntervalType>( j ) );

            if ( dt0 != dt )
            {
                intvType = j - 1;
                break;
            }
        }

        if ( intvType == QwtDate::Millisecond )
            break;
    }

    return static_cast<QwtDate::IntervalType>( intvType );
}

QString QwtDateTimeScaleDraw::format( 
    QwtDate::IntervalType intervalType ) const
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

QDateTime QwtDateTimeScaleDraw::toDateTime( double value ) const
{
    QDateTime dt = QwtDate::toDateTime( value, d_data->timeSpec );
    if ( d_data->timeSpec == Qt::OffsetFromUTC )
        dt.setUtcOffset( d_data->utcOffset );

    return dt;
}

