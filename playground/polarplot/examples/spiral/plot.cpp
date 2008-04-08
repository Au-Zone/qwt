#include <qpen.h>
#include <qwt_data.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_polar_rect.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_curve.h>
#include <qwt_polar_panner.h>
#include <qwt_polar_magnifier.h>
#include <qwt_scale_engine.h>
#include "plot.h"

class Data: public QwtData
{
public:
    Data(const QwtDoubleInterval &radialInterval, 
            const QwtDoubleInterval &azimuthInterval, size_t size):
        d_radialInterval(radialInterval),
        d_azimuthInterval(azimuthInterval),
        d_size(size)
    {
    }

    virtual size_t size() const
    {
        return d_size;
    }

protected:
    QwtDoubleInterval d_radialInterval;
    QwtDoubleInterval d_azimuthInterval;
    size_t d_size;
};

class SpiralData: public Data
{
public:
    SpiralData(const QwtDoubleInterval &radialInterval, 
            const QwtDoubleInterval &azimuthInterval, size_t size):
        Data(radialInterval, azimuthInterval, size)
    {
    }

    virtual QwtData *copy() const
    {
        return new SpiralData(d_radialInterval, d_azimuthInterval, d_size);
    }

    virtual double x(size_t i) const
    {
        const double step = 4 * d_azimuthInterval.width() / d_size;
        return d_azimuthInterval.minValue() + i * step;
    }

    virtual double y(size_t i) const
    {
        const double step = d_radialInterval.width() / d_size;
        return d_radialInterval.minValue() + i * step;
    }
};

class RoseData: public Data
{
public:
    RoseData(const QwtDoubleInterval &radialInterval, 
            const QwtDoubleInterval &azimuthInterval, size_t size):
        Data(radialInterval, azimuthInterval, size)
    {
    }

    virtual QwtData *copy() const
    {
        return new RoseData(d_radialInterval, d_azimuthInterval, d_size);
    }

    virtual double x(size_t i) const
    {
        const double step = d_azimuthInterval.width() / d_size;
        return d_azimuthInterval.minValue() + i * step;
    }

    virtual double y(size_t i) const
    {
        const double a = x(i) / 360.0 * M_PI;
        return d_radialInterval.maxValue() * qwtAbs(::sin(4 * a));
    }
};


Plot::Plot(QWidget *parent):
    QwtPolarPlot(QwtText("Polar Plot Demo"), parent)
{
    setAutoReplot(false);
    setCanvasBackground(Qt::darkBlue);

    const QwtDoubleInterval radialInterval(0.001, 10.0);
    const QwtDoubleInterval azimuthInterval(0.0, 360.0);
    const int numPoints = 200;

    // scales 
    setScale(QwtPolar::Azimuth, 
        azimuthInterval.minValue(), azimuthInterval.maxValue(), 
        azimuthInterval.width() / 12 );

    setScaleMaxMinor(QwtPolar::Azimuth, 2);
#if 1
    setScale(QwtPolar::Radius, 
        radialInterval.minValue(), radialInterval.maxValue());
#else
    setScale(QwtPolar::Radius, 
        radialInterval.maxValue(), radialInterval.minValue());
#endif
#if 0
    setScaleEngine(QwtPolar::Radius,
        new QwtLog10ScaleEngine());
#endif

    QwtPolarPanner *panner = new QwtPolarPanner(canvas());
    panner->setScaleEnabled(QwtPolar::Radius, true);
    panner->setScaleEnabled(QwtPolar::Azimuth, true);

    (void) new QwtPolarMagnifier(canvas());

    // grids, axes 

    d_grid = new QwtPolarGrid();
    d_grid->setPen(QPen(Qt::white));
    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
    {
        d_grid->showGrid(scaleId);
        d_grid->showMinorGrid(scaleId);

        QPen minorPen(Qt::gray);
        minorPen.setStyle(Qt::DotLine);
        d_grid->setMinorGridPen(scaleId, minorPen);
    }
    d_grid->setAxisPen(QwtPolar::AxisAzimuth, QPen(Qt::black));

    d_grid->showAxis(QwtPolar::AxisAzimuth, true);
    d_grid->showAxis(QwtPolar::AxisLeft, false);
    d_grid->showAxis(QwtPolar::AxisRight, true);
    d_grid->showAxis(QwtPolar::AxisTop, true);
    d_grid->showAxis(QwtPolar::AxisBottom, false);
    d_grid->showGrid(QwtPolar::Azimuth, true);
    d_grid->showGrid(QwtPolar::Radius, true);
    d_grid->attach(this);

    // curves

    d_spiralCurve = new QwtPolarCurve("Spiral");
    d_spiralCurve->setStyle(QwtPolarCurve::Lines);
    d_spiralCurve->setPen(QPen(Qt::yellow, 2));
    d_spiralCurve->setSymbol( QwtSymbol(QwtSymbol::Rect, 
        QBrush(Qt::yellow), QPen(Qt::white), QSize(3, 3)) );
    d_spiralCurve->setData(SpiralData(radialInterval, azimuthInterval, numPoints));
    d_spiralCurve->attach(this);

    d_roseCurve = new QwtPolarCurve("Rose");
    d_roseCurve->setStyle(QwtPolarCurve::Lines);
    d_roseCurve->setPen(QPen(Qt::red, 2));
    d_roseCurve->setSymbol( QwtSymbol(QwtSymbol::Rect, 
        QBrush(Qt::cyan), QPen(Qt::white), QSize(3, 3)) );
    d_roseCurve->setData(RoseData(radialInterval, azimuthInterval, numPoints));
    d_roseCurve->attach(this);

#if 1
    QwtLegend *legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box|QFrame::Sunken);
#endif
    insertLegend(legend,  QwtPolarPlot::BottomLegend);
}

PlotSettings Plot::settings() const
{
    PlotSettings s;
    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
    {
        s.majorGrid[scaleId] = d_grid->isGridVisible(scaleId);
        s.minorGrid[scaleId] = d_grid->isMinorGridVisible(scaleId);
    }
    for ( int axisId = 0; axisId < QwtPolar::AxesCount; axisId++ )
        s.axis[axisId] = d_grid->isAxisVisible(axisId);

    s.antialiasing = d_grid->testRenderHint(
        QwtPolarItem::RenderAntialiased );
    s.spiralData = d_spiralCurve->isVisible();
    s.roseData = d_roseCurve->isVisible();

    return s;
}

void Plot::applySettings(const PlotSettings& s)
{
    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
    {
        d_grid->showGrid(scaleId, s.majorGrid[scaleId]);
        d_grid->showMinorGrid(scaleId, s.minorGrid[scaleId]);
    }

    for ( int axisId = 0; axisId < QwtPolar::AxesCount; axisId++ )
        d_grid->showAxis(axisId, s.axis[axisId]);

    d_grid->setRenderHint(
        QwtPolarItem::RenderAntialiased, s.antialiasing);
    d_spiralCurve->setRenderHint(
        QwtPolarItem::RenderAntialiased, s.antialiasing);
    d_spiralCurve->setVisible(s.spiralData);
    d_roseCurve->setRenderHint(
        QwtPolarItem::RenderAntialiased, s.antialiasing);
    d_roseCurve->setVisible(s.roseData);

    replot();
}
