#include "signalgenerator.h"
#include "signaldata.h"
#include <qdatetime.h>
#include <clock.h>
#include <math.h>
#include <qdebug.h>

SignalGenerator::SignalGenerator(QObject *parent):
    QThread(parent),
    d_frequency(5.0),
    d_amplitude(20.0),
    d_signalInterval(5),
	d_isStopped(true)
{
}

void SignalGenerator::setSignalInterval(double interval)
{
    if ( interval < 0.0 )
        interval = 0.0;

    d_signalInterval = interval;
}

double SignalGenerator::signalInterval() const
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
    int counter = 0;

    SignalData &data = SignalData::instance();

    d_clock.start();
	d_isStopped = false;
    while(!d_isStopped)
    {
        const double elapsed = d_clock.elapsed();
        if ( d_frequency > 0.0 )
        {
            const double timeStamp = elapsed / 1000.0;
            const double v = readValue(timeStamp);
            data.append(QwtDoublePoint(timeStamp, v));

            if ( counter++ % 100 == 0 )
            {
                //qDebug() << d_clock.elapsed() - elapsed;
            }
        }

        const double msecs = 
            d_signalInterval - (d_clock.elapsed() - elapsed);

        if ( msecs > 0.0 )
            usleep(qRound(1000.0 * msecs));
    }
}

void SignalGenerator::stop()
{
	d_isStopped = true;
}

double SignalGenerator::readValue(double timeStamp) const
{
    const double period = 1.0 / d_frequency;

    const double x = ::fmod(timeStamp, period);
    const double v = d_amplitude * ::sin(x / period * 2 * M_PI);

    return v;
}
