/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GPL License, Version 2.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_POLAR_CANVAS_H
#define QWT_POLAR_CANVAS_H 1

#include <qframe.h>
#include "qwt_global.h"

class QwtPolarPlot;

class QWT_EXPORT QwtPolarCanvas: public QFrame
{
    Q_OBJECT

public:
    explicit QwtPolarCanvas(QwtPolarPlot *);
    virtual ~QwtPolarCanvas();

    QwtPolarPlot *plot();
    const QwtPolarPlot *plot() const;

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void drawContents(QPainter *);

#if 0
    class PrivateData;
    PrivateData *d_data;
#endif
};

#endif
