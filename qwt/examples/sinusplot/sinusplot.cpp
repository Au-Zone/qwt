#include <qapplication.h>
#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_series_data.h>
#include <qwt_text.h>
#include <qwt_math.h>
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
        QwtSyntheticPointData(100),
        d_y(y)
    {
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

    // Set axes 
    setAxisTitle(xBottom, "x -->");
    setAxisScale(xBottom, 0.0, 10.0);

    setAxisTitle(yLeft, "y -->");
    setAxisScale(yLeft, -1.0, 1.0);
    
    // Insert new curves
    QwtPlotCurve *cSin = new QwtPlotCurve("y = sin(x)");
    cSin->setRenderHint(QwtPlotItem::RenderAntialiased);
	cSin->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
    cSin->setPen(QPen(Qt::red));
    cSin->attach(this);

    QwtPlotCurve *cCos = new QwtPlotCurve("y = cos(x)");
    cCos->setRenderHint(QwtPlotItem::RenderAntialiased);
	cCos->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
    cCos->setPen(QPen(Qt::blue));
    cCos->attach(this);

    // Create sin and cos data
    cSin->setData(new FunctionData(::sin));
    cCos->setData(new FunctionData(::cos));

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
    mX->setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);
    mX->setLabelOrientation(Qt::Vertical);
    mX->setLineStyle(QwtPlotMarker::VLine);
    mX->setLinePen(QPen(Qt::black, 0, Qt::DashDotLine));
    mX->setXValue(2.0 * M_PI);
    mX->attach(this);
}

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    Plot plot;
    plot.resize(600,400);
    plot.show();

    return a.exec(); 
}
