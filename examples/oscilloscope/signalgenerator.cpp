#include "signalgenerator.h"
#include "signaldata.h"
#include <qdatetime.h>
#include <clock.h>
#include <math.h>

SignalGenerator::SignalGenerator(QObject *parent):
    QThread(parent),
    d_frequency(5.0),
    d_amplitude(20.0),
    d_signalInterval(5)
{
    d_clock.start();
}

void SignalGenerator::setSignalInterval(int interval)
{
    if ( interval < 0 )
        interval = 0;

    d_signalInterval = interval;
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
    //while ( !wait(d_signalInterval) )
    while(true)
    {
        if ( d_frequency > 0 )
        {
            const double period = 1.0 / d_frequency;
            const double elapsed = d_clock.elapsed() / 1000.0;

            const double x = ::fmod(elapsed, period);
            const double v = d_amplitude * ::sin(x / period * 2 * M_PI);

            SignalData::instance().append(QwtDoublePoint(elapsed, v));
        }

        msleep(d_signalInterval);
    }
}
