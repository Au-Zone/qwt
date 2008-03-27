/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_RADIAL_PLOT_H
#define QWT_RADIAL_PLOT_H 1

#include <qwidget.h>
#include "qwt_global.h"
#include "qwt_double_interval.h"
#include "qwt_scale_map.h"
#include "qwt_radial_plot_dict.h"

class QwtRoundScaleDraw;
class QwtScaleEngine;
class QwtScaleDiv;

class QWT_EXPORT QwtRadialPlot: public QWidget, public QwtRadialPlotDict
{
    Q_OBJECT

public:
    enum Scale
    {
        DistanceScale,
        AngleScale,

        ScaleCount
    };

    explicit QwtRadialPlot( QWidget *parent = NULL);
#if QT_VERSION < 0x040000
    explicit QwtRadialPlot( QWidget *parent, const char *name);
#endif

    virtual ~QwtRadialPlot();

    void setCanvasBackground(const QBrush&);
    const QBrush &canvasBackground() const;

    void setAutoReplot(bool tf = true); 
    bool autoReplot() const;

    double origin() const;

    void setAutoScale(int scaleId, bool enable);
    bool autoScale(int scaleId) const;

    void setScaleMaxMinor(int scaleId, int maxMinor);
    int scaleMaxMinor(int scaleId) const;

    int scaleMaxMajor(int scaleId) const;
    void setScaleMaxMajor(int scaleId, int maxMajor);

    QwtScaleEngine *scaleEngine(int scaleId);
    const QwtScaleEngine *scaleEngine(int scaleId) const;
    void setScaleEngine(int scaleId, QwtScaleEngine *);

    void setScale(int scaleId, double min, double max, double step = 0);

    void setScaleDiv(int scaleId, const QwtScaleDiv &);
    const QwtScaleDiv *scaleDiv(int scaleId) const;
    QwtScaleDiv *scaleDiv(int scaleId);

    QwtScaleMap canvasMap(int scaleId) const;

    virtual void polish();
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

public slots:
    void setOrigin(double);

    virtual void replot();
    void autoRefresh();

protected:
    virtual bool event(QEvent *);
    virtual void paintEvent(QPaintEvent *);

    virtual void drawCanvas(QPainter *, const QRect &) const;
    virtual void drawItems(QPainter *, const QRect &,
        const QwtScaleMap map[ScaleCount]) const;

    void updateScale(int scaleId);

    QRect boundingRect() const;
    QRect canvasRect() const;

private:
    void initPlot();

    class ScaleData;
    class PrivateData;
    PrivateData *d_data;
};

#endif
