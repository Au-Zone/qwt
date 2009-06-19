#ifndef _CLOCK_H_
#define _CLOCK_H_

#if defined(Q_WS_WIN)
#define USE_QTIME 1
#endif

#if USE_QTIME
// less accurate !!
#include <qdatetime.h>
#else
#include <time.h>
#endif

class Clock
{
public:
    Clock();
    
    bool isValid() const;

    void start();
    void stop();

    double restart();
    double elapsed() const;

private:
#if USE_QTIME
    QTime d_time;
#else
    struct timespec d_timeStamp;
#endif
};

#endif
