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
    signalGenerator.setSignalInterval(window.signalInterval());

    window.connect(&window, SIGNAL(frequencyChanged(double)),
        &signalGenerator, SLOT(setFrequency(double)));
    window.connect(&window, SIGNAL(amplitudeChanged(double)),
        &signalGenerator, SLOT(setAmplitude(double)));
    window.connect(&window, SIGNAL(signalIntervalChanged(double)),
        &signalGenerator, SLOT(setSignalInterval(double)));

    window.show();

    signalGenerator.start();

    bool ok = app.exec(); 

	signalGenerator.stop();
	signalGenerator.wait(1000);

	return ok;
}
