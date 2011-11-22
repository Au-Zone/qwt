#ifndef _PLOT_H_
#define _PLOT_H_

#include <qwt_plot.h>

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot( QWidget *parent );

public Q_SLOTS:
    void insertLegend();
    void insertCurve();
    void removeItem( QwtPlotItem * );
};

#endif
