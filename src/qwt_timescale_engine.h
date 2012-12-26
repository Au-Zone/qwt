#ifndef _QWT_TIME_SCALE_ENGINE_H_
#define _QWT_TIME_SCALE_ENGINE_H_ 1

#include "qwt_date.h"
#include "qwt_scale_engine.h"

/*!
  \brief A scale engine for date/time values

  QwtDateTimeScaleEngine builds scales from a time intervals.
  Together with QwtDateTimeScaleDraw it can be used for
  axes according to date/time values.

  Years, months, weeks, days, hours and minutes are organized
  in steps with non constant intervals. QwtDateTimeScaleEngine
  classifies intervals and aligns the boundaries and tick positions
  according to this classification.

  QwtDateTimeScaleEngine supports representations depending
  on Qt::TimeSpec specifications. The valid range for scales
  is limited by the range of QDateTime, that differs 
  between Qt5 and Qt5.
  
  Date/time values are expected as the number of milliseconds since
  1970-01-01T00:00:00 Universal Coordinated Time - also known
  as "The Epoch", that can be converted to QDateTime using 
  QwtDate::toDateTime().

  \sa QwtDate, QwtTimeIntervalScaleEngine
*/
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
        int maxMajorSteps, int maxMinorSteps,
        double stepSize = 0.0 ) const;

    virtual QwtDate::IntervalType intervalType( 
        const QDateTime &, const QDateTime &, int maxSteps ) const;

    QDateTime toDateTime( double ) const;

protected:
    virtual double autoScaleStepSize(
        const QDateTime &, const QDateTime &,
        QwtDate::IntervalType, int numSteps ) const;

    virtual QDateTime alignDate( const QDateTime &, double stepSize,
        QwtDate::IntervalType, bool up ) const;

private:
    QwtScaleDiv buildScaleDiv( const QDateTime &, const QDateTime &,
        int maxMajorSteps, int maxMinorSteps, 
        QwtDate::IntervalType ) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
