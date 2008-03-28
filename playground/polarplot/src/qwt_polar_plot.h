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

#include <qwidget.h>
#include "qwt_global.h"
#include "qwt_polar.h"
#include "qwt_double_interval.h"
#include "qwt_scale_map.h"
#include "qwt_polar_itemdict.h"

class QwtRoundScaleDraw;
class QwtScaleEngine;
class QwtScaleDiv;

class QWT_EXPORT QwtPolarPlot: public QWidget, public QwtPolarItemDict
{
    Q_OBJECT

public:
    explicit QwtPolarPlot( QWidget *parent = NULL);
#if QT_VERSION < 0x040000
    explicit QwtPolarPlot( QWidget *parent, const char *name);
#endif

    virtual ~QwtPolarPlot();

    void setCanvasBackground(const QBrush&);
    const QBrush &canvasBackground() const;

    void setAutoReplot(bool tf = true); 
    bool autoReplot() const;

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

    QwtScaleMap scaleMap(int scaleId) const;

    void unzoom();
    QwtDoubleRect zoomRect() const;

    virtual void polish();
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

public slots:
    virtual void replot();
    void autoRefresh();
    void setZoomRect(const QwtDoubleRect &);

protected:
    virtual bool event(QEvent *);
    virtual void paintEvent(QPaintEvent *);

    virtual void drawCanvas(QPainter *) const;

    virtual void drawItems(QPainter *painter, 
        const QwtScaleMap &radialMap, const QwtScaleMap &azimuthMap,
        const QwtDoublePoint &pole, double radius,
        const QwtDoubleRect &canvasRect) const;

    void updateScale(int scaleId);

    QwtDoubleRect polarRect() const;

private:
    void initPlot();

    class ScaleData;
    class PrivateData;
    PrivateData *d_data;
};

#endif
