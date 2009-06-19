#include "signalgenerator.h"
#include <qdatetime.h>
#include <qevent.h>
#include <clock.h>
#include <math.h>

SignalGenerator::SignalGenerator(QObject *parent):
    QThread(parent),
    d_frequency(5.0),
    d_amplitude(20.0),
	d_signalInterval(5),
	d_timerId(-1)
{
}

void SignalGenerator::setSignalInterval(int interval)
{
	if ( interval < 0 )
		interval = 0;

	if ( interval != d_signalInterval )
	{
		d_signalInterval = interval;
		if ( d_timerId >= 0 )
		{
			killTimer(d_timerId);
			d_timerId = startTimer(d_signalInterval);
		}
	}
}

int SignalGenerator::timerInterval() const
{
	return d_signalInterval;
}

void SignalGenerator::setFrequency(double frequency)
{
    d_frequency = frequency;
}

double SignalGenerator::frequency() const
{
    return d_frequency;
}

void SignalGenerator::setAmplitude(double amplitude)
{
    d_amplitude = amplitude;
}

double SignalGenerator::amplitude() const
{
    return d_amplitude;
}

void SignalGenerator::run()
{
    d_clock.start();
    d_timerId = startTimer(d_signalInterval);

	exec();
}

void SignalGenerator::timerEvent(QTimerEvent *event)
{
    if ( event->timerId() != d_timerId || d_frequency <= 0.0 )
        return;

    const double period = 1.0 / d_frequency;
    const double elapsed = d_clock.elapsed() / 1000.0;

    const double x = ::fmod(elapsed, period);
    const double v = d_amplitude * ::sin(x / period * 2 * M_PI);

    emit value(elapsed, v);
}
