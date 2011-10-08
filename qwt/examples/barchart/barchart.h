#ifndef _BAR_CHART_H_

#include <qwt_plot.h>

class QwtPlotBarChart;

class BarChart: public QwtPlot
{
    Q_OBJECT

public:
    BarChart( QWidget * = NULL );

    virtual bool eventFilter( QObject *, QEvent * );

public Q_SLOTS:
    void setMode( int );
    void setOrientation( int );
    void exportChart();

private:
    void populate();

    QwtPlotBarChart *d_barChartItem;
};

#endif
