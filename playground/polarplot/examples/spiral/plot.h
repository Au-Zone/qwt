#ifndef _PLOT_H_
#define _PLOT_H_ 1

#include <qwt_polar_plot.h>

class QwtPolarGrid;
class QwtPolarCurve;

class PlotSettings
{
public:
    bool majorGrid[QwtPolar::ScaleCount];
    bool minorGrid[QwtPolar::ScaleCount];
    bool axis[QwtPolar::AxesCount];
    bool antialiasing;
};

class Plot: public QwtPolarPlot
{
    Q_OBJECT

public:
    Plot(QWidget * = NULL);
    PlotSettings settings() const;

public slots:
    void applySettings(const PlotSettings &);

private:
    QwtPolarGrid *d_grid;
    QwtPolarCurve *d_curve;
};

#endif


