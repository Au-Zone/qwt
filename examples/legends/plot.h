#ifndef _PLOT_H_
#define _PLOT_H_

#include <qwt_plot.h>

class Settings;
class LegendItem;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot( QWidget *parent = NULL );

public Q_SLOTS:
    void applySettings( const Settings & );

private:
    void insertCurve();

    LegendItem *d_legendItem;
};

#endif
