#ifndef _PLOT_H_
#define _PLOT_H_

#include <qwt_plot.h>
#include <qwt_scale_div.h>
#include <qwt_series_data.h>

class QwtPlotCurve;
class QwtPlotIntervalCurve;

class Plot: public QwtPlot
{
	Q_OBJECT

public:
	enum Mode
	{
		Bars,
		Tube
	};

    Plot(QWidget * = NULL);

public slots:
    void setMode(int);

private:
    void insertCurve(const QString &title,
        const QwtArray<QwtDoublePoint> &, const QColor &);

    void insertErrorBars(const QString &title,
        const QwtArray<QwtIntervalSample> &,
        const QColor &color);


    QwtScaleDiv yearScaleDiv() const;

    QwtPlotIntervalCurve *d_intervalCurve;
    QwtPlotCurve *d_curve;
};

#endif
