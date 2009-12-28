#ifndef _QWT_SAMPLE_THREAD_H_
#define _QWT_SAMPLE_THREAD_H_

#include <qthread.h>

class QwtSampleThread: public QThread
{
    Q_OBJECT

public:
    virtual ~QwtSampleThread();
    
public slots:
    void setInterval(double interval);
    void stop();

public:
    double interval() const;

protected:
    explicit QwtSampleThread(QObject *parent = NULL);

    virtual void run();
    virtual void sample(double elapsed) = 0;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
