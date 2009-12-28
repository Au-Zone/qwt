#include <qwt_plot.h>
#include <qwt_double_interval.h>
#include <qwt_system_clock.h>

class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotDirectPainter;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget * = NULL);
    virtual ~Plot();

    virtual void replot();

public slots:
    void setIntervalLength(double);

protected:
    virtual void resizeEvent(QResizeEvent *);
    virtual void timerEvent(QTimerEvent *);

private:
    void updateCurve();
    void incrementInterval();

    QwtPlotMarker *d_origin;
    QwtPlotCurve *d_curve;
    int d_paintedPoints;

    QwtPlotDirectPainter *d_directPainter;

    QwtDoubleInterval d_interval;
    int d_timerId;

    QwtSystemClock d_clock;
};
