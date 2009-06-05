#include "signalgenerator.h"
#include <qdatetime.h>
#include <math.h>

SignalGenerator::SignalGenerator(QObject *parent):
	QObject(parent)
{
	d_startTime.start();
	startTimer(5);
}

void SignalGenerator::timerEvent(QTimerEvent *)
{
	const int waveLength = 500; // ms
	const double amplitude = 5.0;

	const double x = d_startTime.elapsed() % waveLength;

	const double v = amplitude * ::sin(x / waveLength * 2 * M_PI);
	emit value(v);
}
