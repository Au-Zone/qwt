#include <qwt_plot.h>

class QwtPlotCurve;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget * = NULL);

public slots:
	void reset();
	void append(double value);

protected:
	virtual void resizeEvent(QResizeEvent *);
	virtual void timerEvent(QTimerEvent *);

private:
	void updateStatistics();

	QwtPlotCurve *d_curve;
	int d_numPoints;
	int d_timerId;
};
