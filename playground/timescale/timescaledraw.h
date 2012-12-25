#ifndef _QWT_TIME_SCALE_DRAW_H_
#define _QWT_TIME_SCALE_DRAW_H_ 1

#include "qwt_global.h"
#include "qwt_scale_draw.h"
#include "qwt_date.h"

class QWT_EXPORT QwtDateTimeScaleDraw: public QwtScaleDraw
{
public:
    QwtDateTimeScaleDraw();
    virtual ~QwtDateTimeScaleDraw();

    virtual QwtText label( double ) const;

protected:
    virtual QString format( const QwtScaleDiv & ) const;
    virtual QString format( QwtDate::IntervalType ) const;

private:
	class PrivateData;
	PrivateData *d_data;
};

#endif
