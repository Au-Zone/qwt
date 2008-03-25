#include <qapplication.h>
#include <qwt_radial_plot.h>
#include <qwt_radial_plot_grid.h>

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QwtRadialPlot plot;
	//plot.setOrigin(M_PI / 2.0);

    QwtRadialPlotGrid *grid = new QwtRadialPlotGrid();
    grid->attach(&plot);

#if QT_VERSION < 0x040000
    a.setMainWidget(&plot);
#endif
    plot.show();
    plot.resize(600, 600);

    return a.exec();
}

