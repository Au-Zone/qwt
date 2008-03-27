#include <qpen.h>
#include <qwt_data.h>
#include <qwt_symbol.h>
#include <qwt_radial_plot_grid.h>
#include <qwt_radial_plot_curve.h>
#include "plot.h"

class Data: public QwtData
{
public:
    Data(const QwtDoubleInterval &interval, size_t size):
        d_interval(interval),
        d_size(size)
    {
    }

    virtual QwtData *copy() const
    {
        return new Data(d_interval, d_size);
    }

    virtual size_t size() const
    {
        return d_size;
    }

    virtual double x(size_t i) const
    {
        const double step = d_interval.width() / d_size;
        return d_interval.minValue() + i * step;
    }

    virtual double y(size_t i) const
    {
        const double step = 4 * 360.0 / d_size;
        return i * step;
    }
private:
    QwtDoubleInterval d_interval;
    size_t d_size;
};

Plot::Plot(QWidget *parent):
    QwtRadialPlot(parent)
{
    setAutoReplot(false);
    const QwtDoubleInterval interval(0.0, 1000.0);

    // scales 
    setScale(QwtRadialPlot::AngleScale, 0.0, 360.0, 45.0);
    setScaleMaxMinor(QwtRadialPlot::AngleScale, 2);
    setScale(QwtRadialPlot::DistanceScale, 
        interval.minValue(), interval.maxValue());
    setScaleMaxMinor(QwtRadialPlot::DistanceScale, 2);

    // grids, axes 

    d_grid = new QwtRadialPlotGrid();
    for ( int scaleId = 0; scaleId < QwtRadialPlot::ScaleCount; scaleId++ )
    {
        d_grid->showGrid(scaleId);
        d_grid->showMinorGrid(scaleId);

        QPen majorPen(Qt::black);
        majorPen.setStyle(Qt::SolidLine);
        d_grid->setMajorGridPen(scaleId, majorPen);

        QPen minorPen(Qt::black);
        minorPen.setStyle(Qt::DotLine);
        d_grid->setMinorGridPen(scaleId, minorPen);
    }

    d_grid->showAxis(QwtRadialPlotGrid::AngleAxis, true);
    d_grid->showAxis(QwtRadialPlotGrid::LeftAxis, false);
    d_grid->showAxis(QwtRadialPlotGrid::RightAxis, true);
    d_grid->showAxis(QwtRadialPlotGrid::TopAxis, true);
    d_grid->showAxis(QwtRadialPlotGrid::BottomAxis, false);
    d_grid->showGrid(QwtRadialPlot::AngleScale, true);
    d_grid->showGrid(QwtRadialPlot::DistanceScale, true);
    d_grid->attach(this);

    // curves

    d_curve = new QwtRadialPlotCurve();
    d_curve->setStyle(QwtRadialPlotCurve::Lines);
    d_curve->setPen(QPen(Qt::blue));
    d_curve->setSymbol( QwtSymbol(QwtSymbol::Rect, 
        QBrush(Qt::red), QPen(Qt::black), QSize(3, 3)) );
    d_curve->setData(Data(interval, 200));
    d_curve->attach(this);
}

PlotSettings Plot::settings() const
{
    PlotSettings s;
    for ( int scaleId = 0; scaleId < QwtRadialPlot::ScaleCount; scaleId++ )
    {
        s.majorGrid[scaleId] = d_grid->isGridVisible(scaleId);
        s.minorGrid[scaleId] = d_grid->isMinorGridVisible(scaleId);
    }
    for ( int axisId = 0; axisId < QwtRadialPlotGrid::AxesCount; axisId++ )
        s.axis[axisId] = d_grid->isAxisVisible(axisId);

    s.antialiasing = d_grid->testRenderHint(
        QwtRadialPlotItem::RenderAntialiased );

    return s;
}

void Plot::applySettings(const PlotSettings& s)
{
    for ( int scaleId = 0; scaleId < QwtRadialPlot::ScaleCount; scaleId++ )
    {
        d_grid->showGrid(scaleId, s.majorGrid[scaleId]);
        d_grid->showMinorGrid(scaleId, s.minorGrid[scaleId]);
    }

    for ( int axisId = 0; axisId < QwtRadialPlotGrid::AxesCount; axisId++ )
        d_grid->showAxis(axisId, s.axis[axisId]);

    d_grid->setRenderHint(
        QwtRadialPlotItem::RenderAntialiased, s.antialiasing);
    d_curve->setRenderHint(
        QwtRadialPlotItem::RenderAntialiased, s.antialiasing);

    replot();
}
