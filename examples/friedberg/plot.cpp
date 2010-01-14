#include <qwt_plot_zoomer.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_interval_symbol.h>
#include <qwt_symbol.h>
#include <qwt_series_data.h>
#include <qwt_text.h>
#include <qwt_scale_draw.h>
#include <qdatetime.h>
#include "plot.h"
#include "friedberg2007.h"

class Grid: public QwtPlotGrid
{
public:
    Grid()
    {
        enableXMin(true);
        setMajPen(QPen(Qt::white, 0, Qt::DotLine));
        setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    }

    virtual void updateScaleDiv(const QwtScaleDiv &xMap,
        const QwtScaleDiv &yMap)
    {
        QwtValueList ticks[QwtScaleDiv::NTickTypes];
        
        ticks[QwtScaleDiv::MajorTick] = xMap.ticks(QwtScaleDiv::MediumTick);
        ticks[QwtScaleDiv::MinorTick] = xMap.ticks(QwtScaleDiv::MinorTick);

        QwtPlotGrid::updateScaleDiv(
            QwtScaleDiv(xMap.lowerBound(), xMap.upperBound(), ticks),
            yMap );
    }
};

class YearScaleDraw: public QwtScaleDraw
{
public:
    YearScaleDraw()
    {
        setTickLength(QwtScaleDiv::MajorTick, 0);
        setTickLength(QwtScaleDiv::MinorTick, 0);
        setTickLength(QwtScaleDiv::MediumTick, 6);

        setLabelRotation(-60.0);
        setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        setSpacing(15);
    }

    virtual QwtText label(double value) const
    {
        return QDate::longMonthName(int(value / 30) + 1);
    }
};

Plot::Plot()
{
    setTitle("Temperature of Friedberg/Germany");
    setCanvasBackground(QColor(Qt::darkGray));

    setAxisTitle(QwtPlot::xBottom, "2007");
    setAxisScaleDiv(QwtPlot::xBottom, yearScaleDiv());
    setAxisScaleDraw(QwtPlot::xBottom, new YearScaleDraw());

    setAxisTitle(QwtPlot::yLeft, 
        QString("Temperature [%1C]").arg(QChar(0x00B0)) );
    // grid
    QwtPlotGrid *grid = new Grid;
    grid->attach(this);

    insertLegend(new QwtLegend(), QwtPlot::RightLegend);

    const int numDays = 365;
    QwtArray<QwtDoublePoint> averageData(numDays);
    QwtArray<QwtIntervalSample> rangeData(numDays);

    for ( int i = 0; i < numDays; i++ )
    {
        const Temperature &t = friedberg2007[i];
        averageData[i] = QwtDoublePoint(double(i), t.averageValue);
        rangeData[i] = QwtIntervalSample( double(i),
            QwtDoubleInterval(t.minValue, t.maxValue) );
    }

    insertCurve("Average", averageData, Qt::black);
    insertErrorBars("Range", rangeData, Qt::blue);

    setMode(QwtPlotIntervalCurve::NoCurve);

    QwtPlotZoomer* zoomer = new QwtPlotZoomer(canvas());
    zoomer->setRubberBandPen(QColor(Qt::black));
    zoomer->setTrackerPen(QColor(Qt::black));
}

QwtScaleDiv Plot::yearScaleDiv() const
{
    const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    QwtValueList ticks[QwtScaleDiv::NTickTypes];

    QwtValueList &mediumTicks = ticks[QwtScaleDiv::MediumTick];
    mediumTicks += 0.0;
    for ( uint i = 0; i < sizeof(days) / sizeof(days[0]); i++ )
        mediumTicks += mediumTicks.last() + days[i];

    QwtValueList &minorTicks = ticks[QwtScaleDiv::MinorTick];
    for ( int i = 1; i <= 365; i += 7 )
        minorTicks += i;

    QwtValueList &majorTicks = ticks[QwtScaleDiv::MajorTick];
    for ( int i = 0; i < 12; i++ )
        majorTicks += i * 30 + 15;

    QwtScaleDiv scaleDiv(mediumTicks.first(), mediumTicks.last() + 1, ticks);
    return scaleDiv;
}

void Plot::insertCurve(const QString& title, 
    const QwtArray<QwtDoublePoint>& samples, const QColor &color)
{
    d_curve = new QwtPlotCurve(title);
#if QT_VERSION >= 0x040000
    d_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
#endif
    d_curve->setStyle(QwtPlotCurve::NoCurve);
    d_curve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol);

    QwtSymbol symbol;
    symbol.setStyle(QwtSymbol::XCross);
    symbol.setSize(4);
    symbol.setPen(QPen(color));
    d_curve->setSymbol(symbol);

    d_curve->setSamples(samples);
    d_curve->attach(this);
}

void Plot::insertErrorBars(
    const QString &title,
    const QwtArray<QwtIntervalSample>& samples, 
    const QColor &color)
{
    d_intervalCurve = new QwtPlotIntervalCurve(title);
#if QT_VERSION >= 0x040000
    d_intervalCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
#endif
    d_intervalCurve->setPen(QPen(Qt::white));

    QColor bg(color);
#if QT_VERSION >= 0x040000
    bg.setAlpha(150);
#endif
    d_intervalCurve->setBrush(QBrush(bg));
    d_intervalCurve->setCurveStyle(QwtPlotIntervalCurve::Tube);

    d_intervalCurve->setSamples(samples);
    d_intervalCurve->attach(this);
}

void Plot::setMode(QwtPlotIntervalCurve::CurveStyle style)
{
    if ( style == QwtPlotIntervalCurve::Tube )
    {
        d_intervalCurve->setCurveStyle(QwtPlotIntervalCurve::Tube);
        d_intervalCurve->setSymbol(QwtIntervalSymbol());
    }
    else
    {
        d_intervalCurve->setCurveStyle(QwtPlotIntervalCurve::NoCurve);

        QwtIntervalSymbol errorBar(QwtIntervalSymbol::Bar);
        errorBar.setWidth(7);
        errorBar.setPen(QPen(d_intervalCurve->brush().color()));

        d_intervalCurve->setSymbol(errorBar);
    }
}
