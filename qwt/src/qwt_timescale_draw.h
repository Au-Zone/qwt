#ifndef _QWT_TIME_SCALE_DRAW_H_
#define _QWT_TIME_SCALE_DRAW_H_ 1

#include "qwt_global.h"
#include "qwt_scale_draw.h"
#include "qwt_date.h"

class QWT_EXPORT QwtDateTimeScaleDraw: public QwtScaleDraw
{
public:
    QwtDateTimeScaleDraw( Qt::TimeSpec = Qt::LocalTime );
    virtual ~QwtDateTimeScaleDraw();

    void setTimeSpec( Qt::TimeSpec );
    Qt::TimeSpec timeSpec() const;

    void setUtcOffset( int seconds );
    int utcOffset() const;

    void setWeek0Type( QwtDate::Week0Type );
    QwtDate::Week0Type week0Type() const;

    virtual QwtText label( double ) const;

    QDateTime toDateTime( double ) const;

    virtual QwtDate::IntervalType 
        intervalType( const QwtScaleDiv & ) const;

protected:
    virtual QString format( QwtDate::IntervalType ) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
