#include <qwt_plot.h>

class QwtWheel;
class QwtPlotMarker;

class Plot: public QwtPlot
{
    Q_OBJECT
public:
    Plot( QWidget *parent = NULL );
    virtual bool eventFilter( QObject *, QEvent * );

public Q_SLOTS:
    void updateMarker( int axis, double base );

#ifndef QT_NO_PRINTER 
    void printPlot();
#endif

private Q_SLOTS:
    void scrollLeftAxis( double );

private:
    QwtPlotMarker *d_marker;
    QwtWheel *d_wheel;
};
