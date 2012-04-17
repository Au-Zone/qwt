#ifndef _BAR_CHART_H_

#include <qwt_plot.h>
#include <qstringlist.h>

class QwtPlotBarChart;

class BarChart: public QwtPlot
{
    Q_OBJECT

public:
    BarChart( QWidget * = NULL );

public Q_SLOTS:
    void setOrientation( int );
    void exportChart();

private:
    void populate();

    QwtPlotBarChart *d_barChartItem;
    QStringList d_distros;
};

#endif
