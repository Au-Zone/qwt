#include <qapplication.h>
#include <qpalette.h>
#include <qwt_radial_plot.h>
#include <qwt_radial_plot_grid.h>

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QwtRadialPlot plot;
    plot.setCanvasBackground(QColor(Qt::white));

	QwtRadialPlotGrid *grid = new QwtRadialPlotGrid();
	grid->attach(&plot);

	//plot.setShape(QRegion::Ellipse);

#if QT_VERSION < 0x040000
    a.setMainWidget(&plot);
#endif
    plot.resize(600,400);
    plot.show();

    return a.exec();
}

