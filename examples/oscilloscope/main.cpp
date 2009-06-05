#include <qapplication.h>
#include "plot.h"
#include "signalgenerator.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    Plot plot;
#if QT_VERSION < 0x040000
    a.setMainWidget(&plot);
#endif

    plot.resize(600,400);
    plot.show();

	SignalGenerator signalGenerator;
	QObject::connect( &signalGenerator, SIGNAL(value(double)), 
		&plot, SLOT(append(double)) );

    return a.exec(); 
}
