#ifndef _PLOT_H_
#define _PLOT_H_ 1

#include <qwt_plot.h>
#include <qwt_system_clock.h>

class QwtPlotCurve;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    enum FunctionType
    {
        NoFunction = -1,

        Wave,
        Noise
    };

    Plot(QWidget* = NULL);

public slots:
    void setTimerInterval(int);
    void setNumPoints(int);
    void setFunctionType(int);

protected:
    virtual void timerEvent(QTimerEvent *e);

private:
    void alignScales();

    QwtPlotCurve *d_curve;

    QwtSystemClock d_clock;
    double d_interval;

    int d_timerId;
};

#endif
