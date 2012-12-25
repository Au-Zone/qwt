#ifndef _QWT_TIME_SCALE_ENGINE_H_
#define _QWT_TIME_SCALE_ENGINE_H_ 1

#include "qwt_date.h"
#include "qwt_scale_engine.h"

class QWT_EXPORT QwtDateTimeScaleEngine: public QwtLinearScaleEngine
{
public:
    QwtDateTimeScaleEngine( Qt::TimeSpec d_timeSpec = Qt::LocalTime );
    virtual ~QwtDateTimeScaleEngine();

    void setTimeSpec( Qt::TimeSpec );
    Qt::TimeSpec timeSpec() const;

    void setUtcOffset( int seconds );
    int utcOffset() const;

	void setWeek0Type( QwtDate::Week0Type );
	QwtDate::Week0Type week0Type() const;
	
    void setMaxWeeks( int );
    int maxWeeks() const;

    virtual void autoScale( int maxNumSteps,
        double &x1, double &x2, double &stepSize ) const;

    virtual QwtScaleDiv divideScale( 
        double x1, double x2,
        int maxMajSteps, int maxMinSteps,
        double stepSize = 0.0 ) const;

    virtual QwtDate::IntervalType intervalType( 
        const QDateTime &, const QDateTime &, int maxSteps ) const;

protected:
    virtual double divideInterval(
        const QDateTime &, const QDateTime &,
        QwtDate::IntervalType, int numSteps ) const;

	QDateTime toDateTime( double ) const;

private:
    QwtScaleDiv divideTo( const QDateTime &, const QDateTime &,
        int maxMajorSteps, int maxMinorSteps, 
		QwtDate::IntervalType ) const;

	QDateTime alignDate( const QDateTime &, double stepSize,
    	QwtDate::IntervalType, bool up ) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
