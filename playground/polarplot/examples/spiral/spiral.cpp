#include <qapplication.h>
#include <qpalette.h>
#include <qwt_polar_plot.h>

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QwtPolarPlot plot;
    plot.setCanvasBackground(QColor(Qt::darkBlue));
	for ( int axisId = 0; axisId < QwtPolarPlot::AxisCnt; axisId++ )
		plot.enableAxis(axisId, true);
	plot.replot();

	//plot.showBackground(false);

#if QT_VERSION < 0x040000
    a.setMainWidget(&plot);
#endif
    plot.resize(600,400);
    plot.show();
    return a.exec();
}

