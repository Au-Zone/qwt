#include <qpen.h>
#include <qwt_radial_plot_grid.h>
#include "plot.h"

Plot::Plot(QWidget *parent):
    QwtRadialPlot(parent)
{
    setAutoReplot(false);

    // scales 
    setScale(QwtRadialPlot::AngleScale, 0.0, 360.0, 45.0);
    setScaleMaxMinor(QwtRadialPlot::AngleScale, 2);
    setScale(QwtRadialPlot::DistanceScale, 0.0, 1000.0);
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

    replot();
}
