#ifndef _PLOT_H_
#define _PLOT_H_ 1

#include <qwt_plot.h>

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget *parent = NULL);
    virtual void updateLayout();

signals:
    void resized(double xRatio, double yRatio);
};

#endif


