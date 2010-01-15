#include <qwt_plot.h>
#include <qrect.h>

class QwtPlotSvgItem;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget * = NULL);

public slots:
    void loadSVG();

private:
    void rescale();

    QwtPlotSvgItem *d_mapItem;
    const QRectF d_mapRect;
};
