#include <qwt_plot.h>
#include <qwt_double_interval.h>
#include "clock.h"

class QwtPlotCurve;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget * = NULL);

public slots:
    void append(double elapsed, double value);
    void setIntervalLength(double);

protected:
    virtual void resizeEvent(QResizeEvent *);
    virtual void timerEvent(QTimerEvent *);

private:
    void updateCurve();
    void incrementInterval();

    QwtPlotCurve *d_curve;
    int d_paintedPoints;

    QwtDoubleInterval d_interval;
    int d_timerId;

    Clock d_clock;
};
