/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_sample_thread.h"
#include "qwt_system_clock.h"

class QwtSampleThread::PrivateData
{
public:
    QwtSystemClock clock;

    double interval;
    bool isStopped;
};


QwtSampleThread::QwtSampleThread(QObject *parent):
    QThread(parent)
{
    d_data = new PrivateData;
    d_data->interval = 1000; // 1 second
    d_data->isStopped = true;
}

QwtSampleThread::~QwtSampleThread()
{
    delete d_data;
}

void QwtSampleThread::setInterval(double interval)
{
    if ( interval < 0.0 )
        interval = 0.0;

    d_data->interval = interval;
}

double QwtSampleThread::interval() const
{
    return d_data->interval;
}

void QwtSampleThread::stop()
{
    d_data->isStopped = true;
}

void QwtSampleThread::run()
{
    d_data->clock.start();
    d_data->isStopped = false;

    while(!d_data->isStopped)
    {
        const double elapsed = d_data->clock.elapsed();
        sample(elapsed / 1000.0);

        if ( d_data->interval > 0.0 )
        {
            const double msecs =
                d_data->interval - (d_data->clock.elapsed() - elapsed);

            if ( msecs > 0.0 )
                usleep(qRound(1000.0 * msecs));
        }
    }
}
