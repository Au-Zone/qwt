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

protected:
    virtual void run();

private:
    Clock d_clock;

    double d_frequency;
    double d_amplitude;

    int d_signalInterval;
};
