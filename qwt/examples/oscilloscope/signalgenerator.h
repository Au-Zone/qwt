#include <qobject.h>
#include <qdatetime.h>
#include "clock.h"

class SignalGenerator: public QObject
{
    Q_OBJECT

public:
    SignalGenerator(QObject *parent = NULL);

    double frequency() const;
    double amplitude() const;

public slots:
    void setAmplitude(double);
    void setFrequency(double);

signals:
    void value(double elapsed, double value);

protected:
    virtual void timerEvent(QTimerEvent *);
    Clock d_clock;

    double d_frequency;
    double d_amplitude;
};
