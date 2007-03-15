/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_AXISMANAGER_H
#define QWT_PLOT_AXISMANAGER_H 1

#include "qwt_global.h"
#include <qobject.h>

class QwtPlotCanvas;
class QwtPlot;
class QResizeEvent;

class QWT_EXPORT QwtPlotAxisManager: public QObject
{
    Q_OBJECT

public:
    enum AspectRatioMode
    {
        KeepAspectRatio,
        KeepRectOfInterest
    };

    explicit QwtPlotAxisManager(QwtPlotCanvas *);
    virtual ~QwtPlotAxisManager();

    void setAxis(int xAxis, int yAxis);
    int xAxis() const;
    int yAxis() const;
    
    void setAspectRatioMode(AspectRatioMode);
    AspectRatioMode aspectRatioMode() const;

    QwtPlotCanvas *canvas();
    const QwtPlotCanvas *canvas() const;

    QwtPlot *plot();
    const QwtPlot *plot() const;

    virtual bool eventFilter(QObject *, QEvent *);

protected:
    virtual void widgetResizeEvent(QResizeEvent *);

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
