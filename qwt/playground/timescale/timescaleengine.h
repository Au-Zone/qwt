#ifndef _TIME_SCALE_ENGINE_H_
#define _TIME_SCALE_ENGINE_H_ 1

#include "qwt_date.h"
#include <qwt_scale_engine.h>

class TimeScaleEngine: public QwtLinearScaleEngine
{
public:
    TimeScaleEngine( Qt::TimeSpec d_timeSpec = Qt::LocalTime );
    virtual ~TimeScaleEngine();

    void setTimeSpec( Qt::TimeSpec );
    Qt::TimeSpec timeSpec() const;

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

private:
    QwtScaleDiv divideTo( 
        const QDateTime &, const QDateTime &,
        int maxMajSteps, int maxMinSteps,
        QwtDate::IntervalType intervalType ) const;

private:
    Qt::TimeSpec d_timeSpec;
    int d_maxWeeks;
};

#endif
