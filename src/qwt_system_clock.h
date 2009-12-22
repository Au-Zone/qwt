/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SYSTEM_CLOCK_H
#define QWT_SYSTEM_CLOCK_H

#include "qwt_global.h"

class QWT_EXPORT QwtSystemClock
{
public:
    QwtSystemClock();
    virtual ~QwtSystemClock();
    
    bool isValid() const;

    void start();
    double restart();
    double elapsed() const;

    static double precision();

private:
	class PrivateData;
	PrivateData *d_data;
};

#endif
