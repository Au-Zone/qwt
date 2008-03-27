#ifndef _PLOT_H_
#define _PLOT_H_ 1

#include <qwt_radial_plot.h>
#include <qwt_radial_plot_grid.h>
#include <qwt_radial_plot_curve.h>

class PlotSettings
{
public:
    bool majorGrid[QwtRadialPlot::ScaleCount];
    bool minorGrid[QwtRadialPlot::ScaleCount];
    bool axis[QwtRadialPlotGrid::AxesCount];
    bool antialiasing;
};

class Plot: public QwtRadialPlot
{
    Q_OBJECT

public:
    Plot(QWidget * = NULL);
    PlotSettings settings() const;

public slots:
    void applySettings(const PlotSettings &);

private:
    QwtRadialPlotGrid *d_grid;
    QwtRadialPlotCurve *d_curve;
};

#endif


