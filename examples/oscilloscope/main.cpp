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

#if 1
    app.connect(&app, SIGNAL(aboutToQuit()), 
        &signalGenerator, SLOT(quit()));
    app.connect(&app, SIGNAL(aboutToQuit()), 
        &signalGenerator, SLOT(terminate()));
#endif

    window.connect(&window, SIGNAL(frequencyChanged(double)),
        &signalGenerator, SLOT(setFrequency(double)));
    window.connect(&window, SIGNAL(amplitudeChanged(double)),
        &signalGenerator, SLOT(setAmplitude(double)));
    window.connect(&window, SIGNAL(signalIntervalChanged(int)),
        &signalGenerator, SLOT(setSignalInterval(int)));

#if QT_VERSION < 0x040000
    app.setMainWidget(&window);
#endif

    window.show();

    signalGenerator.start();
    return app.exec(); 
}
