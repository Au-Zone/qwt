#include <qapplication.h>
#include <qwt_plot.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_legend.h>
#include <qwt_bar.h>
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

class Plot : public QwtPlot
{
public:
    Plot();
};


Plot::Plot()
{
    const int nPoints = 30;

    setTitle("A Plot with Error Bars");
    setCanvasBackground(QColor(Qt::darkGray));

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajPen(QPen(Qt::white, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(this);

    // Insert new curves
    QwtPlotCurve *curve = new QwtPlotCurve("y = sin(x)");
#if QT_VERSION >= 0x040000
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
#endif
    curve->setStyle(QwtPlotCurve::NoCurve);

    QwtSymbol symbol;
    symbol.setStyle(QwtSymbol::XCross);
    symbol.setSize(5);
    symbol.setPen(QPen(Qt::black));
    curve->setSymbol(symbol);

    curve->setData(CurveData(::sin, QwtDoubleInterval(0.0, 1.0), nPoints));
    curve->attach(this);

    QwtPlotIntervalCurve *errorCurve = new QwtPlotIntervalCurve("Error Curve");
#if QT_VERSION >= 0x040000
    errorCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
#endif
    errorCurve->setPen(QPen(Qt::white));

    QColor bg(Qt::white);
#if QT_VERSION >= 0x040000
    bg.setAlpha(150);
#endif
    errorCurve->setBrush(QBrush(bg));

    errorCurve->setCurveStyle(QwtPlotIntervalCurve::Tube);

    QwtBar errorBar(QwtBar::IntervalBar);
    errorBar.setWidth(7);
    errorBar.setPen(QPen(Qt::red));
#if 1
    errorCurve->setBar(errorBar);
#endif

    QwtArray<QwtIntervalSample> errorSamples(nPoints);
    for ( int i = 0; i < nPoints; i++ )
    {
        QwtDoublePoint p = curve->sample(i);
        errorSamples[i].value = p.x();
        errorSamples[i].interval.setMinValue(0.9 * p.y());
        errorSamples[i].interval.setMaxValue(1.1 * p.y());
        errorSamples[i].interval = errorSamples[i].interval.normalized();
    }
    errorCurve->setData(errorSamples);
    errorCurve->attach(this);

    // ----------------------------
    QwtPlotIntervalCurve *errorCurve2 = new QwtPlotIntervalCurve("Error Curve");
    errorCurve2->setOrientation(Qt::Horizontal);
    errorCurve2->setCurveStyle(QwtPlotIntervalCurve::NoCurve);

    errorBar.setPen(QPen(Qt::darkCyan));
    errorCurve2->setBar(errorBar);

    QwtArray<QwtIntervalSample> errorSamples2(nPoints);
    for ( int i = 0; i < nPoints; i++ )
    {
        QwtDoublePoint p = curve->sample(i);
        errorSamples2[i].value = p.y();
        errorSamples2[i].interval.setMinValue(p.x() - 0.03);
        errorSamples2[i].interval.setMaxValue(p.x() + 0.02);
        errorSamples2[i].interval = errorSamples2[i].interval.normalized();
    }
    errorCurve2->setData(errorSamples2);
    errorCurve2->attach(this);
    replot();

    QwtPlotZoomer* zoomer = new QwtPlotZoomer(canvas());
    zoomer->setRubberBandPen(QColor(Qt::green));
    zoomer->setTrackerPen(QColor(Qt::white));
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
