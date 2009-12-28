#ifndef _QWT_SAMPLING_THREAD_H_
#define _QWT_SAMPLING_THREAD_H_

#include <qthread.h>

class QwtSamplingThread: public QThread
{
    Q_OBJECT

public:
    virtual ~QwtSamplingThread();
    
public slots:
    void setInterval(double interval);
    void stop();

public:
    double interval() const;

protected:
    explicit QwtSamplingThread(QObject *parent = NULL);

    virtual void run();
    virtual void sample(double elapsed) = 0;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
