#include "signalgenerator.h"
#include "signaldata.h"
#include <math.h>

SignalGenerator::SignalGenerator(QObject *parent):
    QwtSampleThread(parent),
    d_frequency(5.0),
    d_amplitude(20.0)
{
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

void SignalGenerator::sample(double elapsed)
{
    if ( d_frequency > 0.0 )
    {
        const QwtDoublePoint s(elapsed, value(elapsed));
        SignalData::instance().append(s);
    }
}

double SignalGenerator::value(double timeStamp) const
{
    const double period = 1.0 / d_frequency;

    const double x = ::fmod(timeStamp, period);
    const double v = d_amplitude * ::sin(x / period * 2 * M_PI);

    return v;
}
