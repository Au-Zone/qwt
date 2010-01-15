#ifndef _QWT_SAMPLING_THREAD_H_
#define _QWT_SAMPLING_THREAD_H_

#include "qwt_global.h"
#include <qthread.h>

class QWT_EXPORT QwtSamplingThread: public QThread
{
    Q_OBJECT

public:
    virtual ~QwtSamplingThread();

    double interval() const;
    double elapsed() const;

public Q_SLOTS:
    void setInterval(double interval);
    void stop();

protected:
    explicit QwtSamplingThread(QObject *parent = NULL);

    virtual void run();
    virtual void sample(double elapsed) = 0;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
