#include "signalgenerator.h"
#include <qdatetime.h>
#include <clock.h>
#include <math.h>

SignalGenerator::SignalGenerator(QObject *parent):
    QObject(parent),
    d_frequency(5.0),
    d_amplitude(20.0)
{
    d_clock.start();
    startTimer(5);
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

void SignalGenerator::timerEvent(QTimerEvent *)
{
    if ( d_frequency <= 0.0 )
        return;

    const double period = 1.0 / d_frequency;
    const double elapsed = d_clock.elapsed() / 1000.0;

    const double x = ::fmod(elapsed, period);
    const double v = d_amplitude * ::sin(x / period * 2 * M_PI);

    emit value(elapsed, v);
}
