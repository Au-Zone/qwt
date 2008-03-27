#include <qapplication.h>
#include <qpen.h>
#include <qwt_radial_plot.h>
#include <qwt_radial_plot_grid.h>

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QwtRadialPlot plot;
	plot.setScale(QwtRadialPlot::AngleScale, 0.0, 360.0, 45.0);
	plot.setScaleMaxMinor(QwtRadialPlot::AngleScale, 2);
	plot.setScale(QwtRadialPlot::DistanceScale, 0.0, 1000.0);
	plot.setScaleMaxMinor(QwtRadialPlot::DistanceScale, 2);

    QwtRadialPlotGrid *grid = new QwtRadialPlotGrid();
	for ( int scaleId = QwtRadialPlot::DistanceScale; 
		scaleId < QwtRadialPlot::AngleScale; scaleId++ )
	{
		QwtRadialPlot::Scale scale = (QwtRadialPlot::Scale)scaleId;
		grid->showGrid(scale);
		grid->showMinorGrid(scale);

		QPen majorPen(Qt::black);
		majorPen.setStyle(Qt::SolidLine);
		grid->setMajorGridPen(scale, majorPen);

		QPen minorPen(Qt::black);
		minorPen.setStyle(Qt::DotLine);
		grid->setMinorGridPen(scale, minorPen);
	}

	grid->showAxis(QwtRadialPlotGrid::AngleAxis, true);
	grid->showAxis(QwtRadialPlotGrid::LeftAxis, false);
	grid->showAxis(QwtRadialPlotGrid::RightAxis, true);
	grid->showAxis(QwtRadialPlotGrid::TopAxis, true);
	grid->showAxis(QwtRadialPlotGrid::BottomAxis, false);
	grid->showGrid(QwtRadialPlot::AngleScale, true);
	grid->showGrid(QwtRadialPlot::DistanceScale, true);
    grid->attach(&plot);

#if QT_VERSION < 0x040000
    a.setMainWidget(&plot);
#endif
    plot.show();
    plot.resize(600, 600);

    return a.exec();
}

