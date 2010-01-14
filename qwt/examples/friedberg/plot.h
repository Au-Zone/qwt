#ifndef _PLOT_H_
#define _PLOT_H_

#include <qwt_plot.h>
#include <qwt_scale_div.h>
#include <qwt_plot_intervalcurve.h>

class QwtPlotCurve;

class Plot: public QwtPlot
{
public:
    Plot();

private:
    void insertCurve(const QString &title,
        const QwtArray<QwtDoublePoint> &, const QColor &);

    void insertErrorBars(const QString &title,
        const QwtArray<QwtIntervalSample> &,
        const QColor &color);

    void setMode(QwtPlotIntervalCurve::CurveStyle);

    QwtScaleDiv yearScaleDiv() const;

    QwtPlotIntervalCurve *d_intervalCurve;
    QwtPlotCurve *d_curve;
};

#endif
