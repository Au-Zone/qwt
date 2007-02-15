#include <stdlib.h>
#include <qapplication.h>
#include <qpen.h>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_series_data.h>
#include "histogram_item.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QwtPlot plot;
    plot.setCanvasBackground(QColor(Qt::white));
    plot.setTitle("Histogram");

    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->enableYMin(true);
    grid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(&plot);

    HistogramItem *histogram = new HistogramItem();
    histogram->setColor(Qt::darkCyan);

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

    histogram->setData(QwtIntervalSeriesData(samples));
    histogram->attach(&plot);

    plot.setAxisScale(QwtPlot::yLeft, 0.0, 100.0);
    plot.setAxisScale(QwtPlot::xBottom, 0.0, pos);
    plot.replot();

#if QT_VERSION < 0x040000
    a.setMainWidget(&plot);
#endif

    plot.resize(600,400);
    plot.show();

    return a.exec(); 
}
