/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_POLAR_PLOT_H
#define QWT_POLAR_PLOT_H 1

#include <qframe.h>
#include "qwt_global.h"
#include "qwt_scale_map.h"
#include "qwt_circular_plot.h"

class QwtScaleDraw;

class QWT_EXPORT QwtPolarPlot: public QwtCircularPlot
{
    Q_OBJECT

public:
    enum Axis 
    { 
        LeftAxis,
        RightAxis,
        BottomAxis,
        TopAxis,

        AxisCnt
    };

    explicit QwtPolarPlot( QWidget *parent = NULL);
#if QT_VERSION < 0x040000
    explicit QwtPolarPlot( QWidget *parent, const char *name);
#endif

    virtual ~QwtPolarPlot();

    virtual QwtScaleMap canvasMap(int axisId) const;

    void enableAxis(int axisId, bool enable = true);
    bool axisEnabled(int axisId) const;

    void setAxisMaxMinor(int axisId, int maxMinor);
    int axisMaxMajor(int axisId) const;
    void setAxisMaxMajor(int axisId, int maxMajor);
    int axisMaxMinor(int axisId) const;

    void setAxisScaleEngine(int axisId, QwtScaleEngine *);
    QwtScaleEngine *axisScaleEngine(int axisId);
    const QwtScaleEngine *axisScaleEngine(int axisId) const;

    void setAxisScale(int axisId, double min, double max, double step = 0);

    void setAxisScaleDiv(int axisId, const QwtScaleDiv &);
    const QwtScaleDiv *axisScaleDiv(int axisId) const;
    QwtScaleDiv *axisScaleDiv(int axisId);

    void setAxisScaleDraw(int axisId, QwtScaleDraw *);
    const QwtScaleDraw *axisScaleDraw(int axisId) const;
    QwtScaleDraw *axisScaleDraw(int axisId);

    void setAxisAutoScale(int axisId);
    bool axisAutoScale(int axisId) const;

public slots:
    virtual void replot();

protected:
    virtual void drawCanvas(QPainter *, const QRect&) const;
    virtual void drawAxis(QPainter *, const QRect&, int axisId) const;

private:
    void initPlot();

    class AxisData;
    class PrivateData;
    PrivateData *d_data;
};

#endif
