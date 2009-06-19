#include <qobject.h>
#include <qdatetime.h>
#include <qthread.h>
#include "clock.h"

class SignalGenerator: public QThread
{
    Q_OBJECT

public:
    SignalGenerator(QObject *parent = NULL);

    double frequency() const;
    double amplitude() const;
    int timerInterval() const;

public slots:
    void setAmplitude(double);
    void setFrequency(double);
    void setSignalInterval(int); // ms

signals:
    void value(double elapsed, double value);

protected:
	virtual void run(); 
    virtual void timerEvent(QTimerEvent *);

private:
    Clock d_clock;

    double d_frequency;
    double d_amplitude;

	int d_signalInterval;
	int d_timerId;
};
