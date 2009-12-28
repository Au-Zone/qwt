#include <qwt_sample_thread.h>

class SignalGenerator: public QwtSampleThread
{
    Q_OBJECT

public:
    SignalGenerator(QObject *parent = NULL);

    double frequency() const;
    double amplitude() const;

public slots:
    void setAmplitude(double);
    void setFrequency(double);

protected:
    virtual void sample(double elapsed);

private:
    virtual double value(double timeStamp) const;

    double d_frequency;
    double d_amplitude;
};
