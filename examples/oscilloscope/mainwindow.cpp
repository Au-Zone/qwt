#include "mainwindow.h"
#include "plot.h"
#include "knob.h"
#include "wheelbox.h"
#include <qwt_scale_engine.h>
#include <qlabel.h>
#include <qlayout.h>

MainWindow::MainWindow(QWidget *parent):
    QWidget(parent)
{
    const double intervalLength = 10.0; // seconds

    d_plot = new Plot(this);
    d_plot->setIntervalLength(intervalLength);

    d_amplitudeKnob = new Knob("Amplitude", 0.0, 200.0, this);
    d_amplitudeKnob->setValue(50.0);
    
    d_frequencyKnob = new Knob("Frequency [Hz]", 0.1, 20.0, this);
    d_frequencyKnob->setValue(5.0);

    d_intervalWheel = new WheelBox("Displayed [s]", 1.0, 100.0, 1.0, this);
    d_intervalWheel->setValue(intervalLength);

    d_timerWheel = new WheelBox("Signal Interval [ms]", 0.0, 100.0, 1.0, this);
    d_timerWheel->setValue(5.0);

    QVBoxLayout* vLayout1 = new QVBoxLayout();
    vLayout1->addWidget(d_intervalWheel);
    vLayout1->addWidget(d_timerWheel);
    vLayout1->addStretch(10);
    vLayout1->addWidget(d_amplitudeKnob);
    vLayout1->addWidget(d_frequencyKnob);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(d_plot, 10);
    layout->addLayout(vLayout1);

    connect(d_amplitudeKnob, SIGNAL(valueChanged(double)),
		SIGNAL(amplitudeChanged(double)));
    connect(d_frequencyKnob, SIGNAL(valueChanged(double)),
		SIGNAL(frequencyChanged(double)));
    connect(d_timerWheel, SIGNAL(valueChanged(double)),
		SLOT(timerWheelChanged(double)));

    connect(d_intervalWheel, SIGNAL(valueChanged(double)),
        d_plot, SLOT(setIntervalLength(double)) );
}

void MainWindow::appendValue(double time, double value)
{
	d_plot->append(time, value);
}

double MainWindow::frequency() const
{
	return d_frequencyKnob->value();
}

double MainWindow::amplitude() const
{
	return d_amplitudeKnob->value();
}

void MainWindow::timerWheelChanged(double value)
{
	emit signalIntervalChanged(qRound(value));
}
