#include <qapplication.h>
#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_series_data.h>
#include <qwt_text.h>
#include <math.h>

//-----------------------------------------------------------------
//              simple.cpp
//
//      A simple example which shows how to use QwtPlot connected 
//      to a data class without any storage, calculating each values 
//      on the fly.
//-----------------------------------------------------------------

class FunctionData: public QwtSyntheticPointData
{
public:
    FunctionData(double(*y)(double)):
        QwtSyntheticPointData(500),
        d_y(y)
    {
    }

    virtual QwtSeriesData<QwtDoublePoint> *copy() const
    {
        return new FunctionData(d_y);
    }

    virtual double y(double x) const
    {
        return d_y(x);
    }

private:
    double(*d_y)(double);
};

class Plot : public QwtPlot
{
public:
    Plot();
};


Plot::Plot()
{
    setTitle("A Simple QwtPlot Demonstration");
    insertLegend(new QwtLegend(), QwtPlot::RightLegend);

    // Set axis titles
    setAxisTitle(xBottom, "x -->");
    setAxisTitle(yLeft, "y -->");
    
    // Insert new curves
    QwtPlotCurve *cSin = new QwtPlotCurve("y = sin(x)");
#if QT_VERSION >= 0x040000
    cSin->setRenderHint(QwtPlotItem::RenderAntialiased);
#endif
    cSin->setPen(QPen(Qt::red));
    cSin->attach(this);

    QwtPlotCurve *cCos = new QwtPlotCurve("y = cos(x)");
#if QT_VERSION >= 0x040000
    cCos->setRenderHint(QwtPlotItem::RenderAntialiased);
#endif
    cCos->setPen(QPen(Qt::blue));
    cCos->attach(this);

    // Create sin and cos data
    cSin->setData(FunctionData(::sin));
    cCos->setData(FunctionData(::cos));

    // Insert markers
    
    //  ...a horizontal line at y = 0...
    QwtPlotMarker *mY = new QwtPlotMarker();
    mY->setLabel(QString::fromLatin1("y = 0"));
    mY->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    mY->setLineStyle(QwtPlotMarker::HLine);
    mY->setYValue(0.0);
    mY->attach(this);

    //  ...a vertical line at x = 2 * pi
    QwtPlotMarker *mX = new QwtPlotMarker();
    mX->setLabel(QString::fromLatin1("x = 2 pi"));
    mX->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    mX->setLineStyle(QwtPlotMarker::VLine);
    mX->setXValue(6.284);
    mX->attach(this);
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
