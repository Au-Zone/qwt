#ifndef _DATA_PLOT_H
#define _DATA_PLOT_H 1

#include <qwt_plot.h>
#include <qdatetime.h>
#include <qbasictimer.h>

const int PLOT_SIZE = 200;

class DataPlot : public QwtPlot
{
    Q_OBJECT

public:
    DataPlot(QWidget* = NULL);

public slots:
    void setTimerInterval(int interval);

protected:
    virtual void timerEvent(QTimerEvent *e);

private:
    void updateValues();
    void alignScales();

    double d_x[PLOT_SIZE]; 
    double d_y[PLOT_SIZE]; 
    double d_z[PLOT_SIZE];

    QBasicTimer d_timer;
    QTime d_started;
};

#endif
