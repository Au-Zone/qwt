#include <stdlib.h>
#include <qapplication.h>
#include <qpen.h>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_histogram.h>
#include <qwt_column_symbol.h>
#include <qwt_series_data.h>

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QwtPlot plot;
    plot.setCanvasBackground(QColor(Qt::gray));
    plot.setTitle("Histogram");

    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->enableYMin(true);
    grid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(&plot);

    QwtPlotHistogram *histogram = new QwtPlotHistogram();
    histogram->setStyle(QwtPlotHistogram::Columns);
    histogram->setPen(QPen(Qt::black));
    histogram->setBrush(QBrush(Qt::gray));
#if 0
    //histogram->setBaseline(50.0);
    histogram->setOrientation(Qt::Horizontal);
#endif

#if 1
    QwtColumnSymbol symbol(QwtColumnSymbol::Box);
    symbol.setFrameStyle(QFrame::Panel | QFrame::Raised);
    symbol.setLineWidth(2);
    symbol.setPalette(QPalette(Qt::magenta));
    histogram->setSymbol(symbol);
#endif

#if 0
    const int numValues = 20;

    QwtArray<QwtIntervalSample> samples(numValues);

    double pos = 0.0;
    for ( int i = 0; i < (int)samples.size(); i++ )
    {
        const int width = 5 + rand() % 15;
        const int value = rand() % 100;

        samples[i].interval = QwtDoubleInterval(pos, pos + double(width));
        samples[i].value = value; 

        pos += width;
    }
#else
    QwtArray<QwtIntervalSample> samples(3);
    samples[0].interval = QwtDoubleInterval(0, 20);
    samples[0].value = 40;
    samples[1].interval = QwtDoubleInterval(20.01, 50);
    samples[1].interval.setBorderFlags(QwtDoubleInterval::ExcludeBorders);
    samples[1].value = 80;
    samples[2].interval = QwtDoubleInterval(50, 80);
    samples[2].value = 30;
#endif

    histogram->setData(QwtIntervalSeriesData(samples));
    histogram->attach(&plot);

#if 0
    plot.setAxisScale(QwtPlot::yLeft, 0.0, 100.0);
    plot.setAxisScale(QwtPlot::xBottom, 0.0, pos);
#endif
    plot.replot();

#if QT_VERSION < 0x040000
    a.setMainWidget(&plot);
#endif

    plot.resize(600,400);
    plot.show();

    return a.exec(); 
}
