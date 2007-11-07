#include <qapplication.h>
#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_legend.h>
#include <qwt_bar.h>
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
    setCanvasBackground(QColor(Qt::darkBlue));

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
    curve->setPen(QPen(Qt::yellow));
    curve->setData(CurveData(::sin, QwtDoubleInterval(0.0, 1.0), nPoints));
    curve->attach(this);

    QwtPlotIntervalCurve *errorCurve = new QwtPlotIntervalCurve("Error Curve");
#if QT_VERSION >= 0x040000
    errorCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
#endif
    errorCurve->setPen(QPen(Qt::darkGray));

    errorCurve->setBrush(QBrush(Qt::lightGray));
    errorCurve->setCurveStyle(QwtPlotIntervalCurve::Tube);
    //errorCurve->setCurveStyle(QwtPlotIntervalCurve::NoCurve);

	QwtBar errorBar(QwtBar::ErrorBar);
	errorBar.setWidth(3);
	errorBar.setPen(QPen(Qt::red));
    errorCurve->setBar(errorBar);

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
