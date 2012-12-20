#ifndef _TIME_SCALE_ENGINE_H_
#define _TIME_SCALE_ENGINE_H_ 1

#include "timedate.h"
#include <qwt_scale_engine.h>

class TimeScaleEngine: public QwtLinearScaleEngine
{
public:
    TimeScaleEngine();
    virtual ~TimeScaleEngine();

    void setMaxWeeks( int );
    int maxWeeks() const;

    virtual void autoScale( int maxNumSteps,
        double &x1, double &x2, double &stepSize ) const;

    virtual QwtScaleDiv divideScale( double x1, double x2,
        int maxMajSteps, int maxMinSteps,
        double stepSize = 0.0 ) const;

    virtual QwtTransform *transformation() const;

    virtual TimeDate::IntervalType intervalType( 
        double x1, double x2, int maxSteps ) const;

protected:
    virtual double divideInterval(
        const QDateTime &, const QDateTime &,
        TimeDate::IntervalType, int numSteps ) const;

private:
    QwtScaleDiv divideTo( double min, double max,
        int maxMajSteps, int maxMinSteps,
        TimeDate::IntervalType intervalType ) const;

private:
    Qt::TimeSpec d_timeSpec;
    int d_maxWeeks;
};

#endif
