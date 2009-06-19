#include <qapplication.h>
#include "mainwindow.h"
#include "signalgenerator.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow window;
    window.resize(800,400);

	SignalGenerator signalGenerator;
	signalGenerator.setFrequency(window.frequency());
	signalGenerator.setAmplitude(window.amplitude());

	app.connect(&app, SIGNAL(aboutToQuit()), &signalGenerator, SLOT(quit()));

	window.connect(&window, SIGNAL(frequencyChanged(double)),
		&signalGenerator, SLOT(setFrequency(double)));
	window.connect(&window, SIGNAL(amplitudeChanged(double)),
		&signalGenerator, SLOT(setAmplitude(double)));
	window.connect(&window, SIGNAL(signalIntervalChanged(int)),
		&signalGenerator, SLOT(setSignalInterval(int)));

	window.connect(&signalGenerator, SIGNAL(value(double, double)),
		&window, SLOT(appendValue(double, double)));

#if QT_VERSION < 0x040000
    app.setMainWidget(&window);
#endif

    window.show();

	signalGenerator.start();
    return app.exec(); 
}
