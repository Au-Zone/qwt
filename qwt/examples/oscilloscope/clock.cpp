#include "clock.h"

Clock::Clock()
{
    stop();
}

#if USE_QTIME

bool Clock::isValid() const { return d_time.isValid() };
void Clock::start() { d_time.start(); }
void Clock::stop() { d_time.stop(); }
double Clock::restart() { return d_time.restart(); }
double Clock::elapsed() { return d_time.elapsed(); }

#else

static inline double msecsTo(
    const struct timespec &t1, const struct timespec &t2) 
{
    return (t2.tv_sec - t1.tv_sec) * 1e3
            + (t2.tv_nsec - t1.tv_nsec) * 1e-6;
}

bool Clock::isValid() const
{
    return d_timeStamp.tv_sec > 0 && d_timeStamp.tv_nsec > 0;
}


void Clock::start()
{
    ::clock_gettime(CLOCK_MONOTONIC, &d_timeStamp);
}

void Clock::stop()
{
    d_timeStamp.tv_sec = d_timeStamp.tv_nsec = 0;
}

double Clock::restart()
{
    struct timespec timeStamp;
    ::clock_gettime(CLOCK_MONOTONIC, &timeStamp);

    double elapsed = 0.0;
    if ( isValid() )
        elapsed = msecsTo(d_timeStamp, timeStamp);

    d_timeStamp = timeStamp;
    return elapsed;
}

double Clock::elapsed() const
{
    double elapsed = 0.0;
    if ( isValid() )
    {
        struct timespec timeStamp;
        ::clock_gettime(CLOCK_MONOTONIC, &timeStamp);

        elapsed = msecsTo(d_timeStamp, timeStamp);
    }
    return elapsed;
}

#endif
