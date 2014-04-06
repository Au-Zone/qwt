#include <qwt_plot.h>

class QwtWheel;
class QwtPlotMarker;
class QwtPlotCurve;

class Plot: public QwtPlot
{
    Q_OBJECT
public:
    Plot( QWidget *parent = NULL );
    virtual bool eventFilter( QObject *, QEvent * );

public Q_SLOTS:
    void updateMarker( int axis, double base );
    void legendChecked( const QVariant &, bool on );
    void setOverlaying( bool );

#ifndef QT_NO_PRINTER 
    void printPlot();
#endif

private Q_SLOTS:
    void scrollLeftAxis( double );

private:
    void showCurve( QwtPlotItem *, bool on );

    QwtPlotMarker *d_marker;
    QwtPlotCurve *d_curve;
    QwtWheel *d_wheel;
};
