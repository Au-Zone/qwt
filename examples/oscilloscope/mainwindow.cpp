#include "mainwindow.h"
#include "plot.h"
#include "signalgenerator.h"
#include "knob.h"
#include <qwt_wheel.h>
#include <qwt_scale_engine.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qlayout.h>

MainWindow::MainWindow(QWidget *parent):
    QWidget(parent)
{
    const double intervalLength = 10.0; // seconds

    SignalGenerator *signalGenerator = new SignalGenerator(this);

    Plot *plot = new Plot(this);
    plot->setIntervalLength(intervalLength);

    Knob *amplitude = new Knob("Amplitude", 0.0, 200.0, this);
    amplitude->setValue(signalGenerator->amplitude());
    
    Knob *frequency = new Knob("Frequency Hz", 0.1, 20.0, this);
    frequency->setValue(signalGenerator->frequency());

    QLCDNumber *interval = new QLCDNumber(this);
    interval->setFixedHeight(50);
    interval->setSegmentStyle(QLCDNumber::Filled);
    interval->display(intervalLength);
    
    QwtWheel *wheel = new QwtWheel(this);
    wheel->setOrientation(Qt::Vertical);
    wheel->setFixedSize(20, interval->height() - 10);
    wheel->setRange(1.0, 100.0, 1.0);
    wheel->setValue(intervalLength);

    QHBoxLayout* hLayout1 = new QHBoxLayout();
    hLayout1->addWidget(interval, 10);
    hLayout1->addWidget(wheel);

    QVBoxLayout* vLayout1 = new QVBoxLayout();
    vLayout1->addWidget(amplitude);
    vLayout1->addWidget(frequency);
    vLayout1->addLayout(hLayout1);
    vLayout1->addStretch(10);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(plot, 10);
    layout->addLayout(vLayout1);

    connect(signalGenerator, SIGNAL(value(double, double)),
        plot, SLOT(append(double, double)) );

    connect(amplitude, SIGNAL(valueChanged(double)),
        signalGenerator, SLOT(setAmplitude(double)) );
    connect(frequency, SIGNAL(valueChanged(double)),
        signalGenerator, SLOT(setFrequency(double)) );
    connect(wheel, SIGNAL(valueChanged(double)),
        interval, SLOT(display(double)) );
    connect(wheel, SIGNAL(valueChanged(double)),
        plot, SLOT(setIntervalLength(double)) );
}
