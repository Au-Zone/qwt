#include <qpen.h>
#include <qwt_data.h>
#include <qwt_symbol.h>
#include <qwt_polar_rect.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_curve.h>
#include <qwt_polar_panner.h>
#include <qwt_polar_magnifier.h>
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

    virtual QwtData *copy() const
    {
        return new Data(d_radialInterval, d_azimuthInterval, d_size);
    }

    virtual size_t size() const
    {
        return d_size;
    }

    virtual double x(size_t i) const
    {
        const double step = d_radialInterval.width() / d_size;
        return d_radialInterval.minValue() + i * step;
    }

    virtual double y(size_t i) const
    {
        const double step = 4 * d_azimuthInterval.width() / d_size;
        return d_azimuthInterval.minValue() + i * step;
    }

private:
    QwtDoubleInterval d_radialInterval;
    QwtDoubleInterval d_azimuthInterval;
    size_t d_size;
};

Plot::Plot(QWidget *parent):
    QwtPolarPlot(parent)
{
    setAutoReplot(false);

    const QwtDoubleInterval radialInterval(0.0, 10.0);
    const QwtDoubleInterval azimuthInterval(0.0, 360.0);

    // scales 
    setScale(QwtPolar::Azimuth, 
        azimuthInterval.minValue(), azimuthInterval.maxValue(), 
        azimuthInterval.width() / 12 );

    setScaleMaxMinor(QwtPolar::Azimuth, 2);
    setScale(QwtPolar::Radius, 
        radialInterval.minValue(), radialInterval.maxValue());

#if 1
	//setZoomRect(QwtPolarRect(1.0, M_PI / 3.0, 7.0, 4.0));
#if 0
	const QwtDoubleRect zr(-1.0, -1.0, 5.0, 4.0);
	setZoomRect(zr);
#endif

	QwtPolarPanner *panner = new QwtPolarPanner(this);
	panner->setScaleEnabled(QwtPolar::Radius, true);
	panner->setScaleEnabled(QwtPolar::Azimuth, true);

	(void) new QwtPolarMagnifier(this);
#endif

    // grids, axes 

    d_grid = new QwtPolarGrid();
    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
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

    d_grid->showAxis(QwtPolar::AxisAzimuth, true);
    d_grid->showAxis(QwtPolar::AxisLeft, false);
    d_grid->showAxis(QwtPolar::AxisRight, true);
    d_grid->showAxis(QwtPolar::AxisTop, true);
    d_grid->showAxis(QwtPolar::AxisBottom, false);
    d_grid->showGrid(QwtPolar::Azimuth, true);
    d_grid->showGrid(QwtPolar::Radius, true);
    d_grid->attach(this);

    // curves

    d_curve = new QwtPolarCurve();
    d_curve->setStyle(QwtPolarCurve::Lines);
    d_curve->setPen(QPen(Qt::blue));
    d_curve->setSymbol( QwtSymbol(QwtSymbol::Rect, 
        QBrush(Qt::red), QPen(Qt::black), QSize(3, 3)) );
    d_curve->setData(Data(radialInterval, azimuthInterval, 200));
    d_curve->attach(this);
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
	s.spiralData = d_curve->isVisible();

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
    d_curve->setRenderHint(
        QwtPolarItem::RenderAntialiased, s.antialiasing);
	d_curve->setVisible(s.spiralData);

    replot();
}
