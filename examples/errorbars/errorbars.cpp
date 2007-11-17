#include <qapplication.h>
#include <qwt_plot.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_legend.h>
#include <qwt_interval_symbol.h>
#include <qwt_symbol.h>
#include <qwt_series_data.h>
#include <qwt_text.h>
#include <math.h>

class CurveData: public QwtSeriesData<QwtDoublePoint>
{
public:
    CurveData(double(*y)(double), 
            const QwtDoubleInterval& interval, size_t size):
        d_interval(interval.normalized()),
        d_size(size),
        d_y(y)
    {
    }

    virtual QwtSeriesData<QwtDoublePoint> *copy() const
    {
        return new CurveData(d_y, d_interval, d_size);
    }

    virtual size_t size() const
    {
        return d_size;
    }

    virtual QwtDoublePoint sample(size_t i) const
    {
        if ( d_size == 0 || !d_interval.isValid() )
            return QwtDoublePoint();

        const double dist = d_interval.width() / d_size;

        const double x = d_interval.minValue() + dist * i;
        const double y = d_y(x);

        return QwtDoublePoint(x, y);
    }

    virtual QwtDoubleRect boundingRect() const
    {
        return qwtBoundingRect(*this);
    }

private:
    QwtDoubleInterval d_interval;
    size_t d_size;
    double(*d_y)(double);
};

class Plot: public QwtPlot
{
public:
    Plot();

private:
    void insertCurve(const QString &title, 
        const QwtSeriesData<QwtDoublePoint> &, const QColor &);
    void insertErrorBars(const QwtSeriesData<QwtDoublePoint> &, 
        Qt::Orientation, const QColor &, bool showTube);
};


Plot::Plot()
{
    const QwtDoubleInterval interval(-1.5, 1.5);
    const int numPoints(30);

    setTitle("A Plot with Error Bars");
    setCanvasBackground(QColor(Qt::darkGray));

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajPen(QPen(Qt::white, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(this);

    // sinus
    const CurveData sinusData(::cosh, interval, numPoints);
    insertCurve("y = sin(x)", sinusData, Qt::black );
    insertErrorBars(sinusData, Qt::Vertical, Qt::red, true);

    // cosinus
    const CurveData tangensData(::sinh, interval, numPoints);
    insertCurve("y = sinh(x)", tangensData, Qt::black );
    insertErrorBars(tangensData, Qt::Horizontal, Qt::blue, true);

    QwtPlotZoomer* zoomer = new QwtPlotZoomer(canvas());
    zoomer->setRubberBandPen(QColor(Qt::black));
    zoomer->setTrackerPen(QColor(Qt::black));

    insertLegend(new QwtLegend(), QwtPlot::RightLegend);
}

void Plot::insertCurve(const QString& title, 
    const QwtSeriesData<QwtDoublePoint>& data, const QColor &color)
{
    QwtPlotCurve *curve = new QwtPlotCurve(title);
#if QT_VERSION >= 0x040000
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
#endif
    curve->setStyle(QwtPlotCurve::NoCurve);

    QwtSymbol symbol;
    symbol.setStyle(QwtSymbol::XCross);
    symbol.setSize(4);
    symbol.setPen(QPen(color));
    curve->setSymbol(symbol);

    curve->setData(data);
    curve->attach(this);
}

void Plot::insertErrorBars(const QwtSeriesData<QwtDoublePoint>& data, 
    Qt::Orientation orientation, const QColor &color, bool showTube)
{
    QwtPlotIntervalCurve *errorCurve = new QwtPlotIntervalCurve();
#if QT_VERSION >= 0x040000
    errorCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
#endif
    errorCurve->setOrientation(orientation);
    errorCurve->setPen(QPen(Qt::white));

    if ( showTube )
    {
        QColor bg(Qt::white);
#if QT_VERSION >= 0x040000
        bg.setAlpha(150);
#endif
        errorCurve->setBrush(QBrush(bg));
        errorCurve->setCurveStyle(QwtPlotIntervalCurve::Tube);
    }
    else
    {
        errorCurve->setCurveStyle(QwtPlotIntervalCurve::NoCurve);
    }

    QwtIntervalSymbol errorBar(QwtIntervalSymbol::Bar);
    errorBar.setWidth(7);
    errorBar.setPen(QPen(color));
    errorCurve->setSymbol(errorBar);

    QwtArray<QwtIntervalSample> errorSamples(data.size());
    for ( uint i = 0; i < data.size(); i++ )
    {
        const QwtDoublePoint point = data.sample(i);

        QwtIntervalSample &sample = errorSamples[i];
        if ( orientation == Qt::Vertical )
        {
            sample.value = point.x();
            sample.interval.setMinValue(0.9 * point.y());
            sample.interval.setMaxValue(1.1 * point.y());
        }
        else
        {
            sample.value = point.y();
            sample.interval.setMinValue(0.9 * point.x());
            sample.interval.setMaxValue(1.1 * point.x());
        }
        sample.interval = sample.interval.normalized();
    }
    errorCurve->setData(errorSamples);
    errorCurve->attach(this);
}

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    Plot plot;
#if QT_VERSION < 0x040000
    a.setMainWidget(&plot);
#endif
    plot.resize(600,400);
    plot.show();
    return a.exec(); 
}
