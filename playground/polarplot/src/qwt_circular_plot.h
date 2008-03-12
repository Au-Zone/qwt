/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_CIRCULAR_PLOT_H
#define QWT_CIRCULAR_PLOT_H 1

#include <qframe.h>
#include "qwt_global.h"
#include "qwt_scale_map.h"

class QwtRoundScaleDraw;
class QwtScaleEngine;
class QwtScaleDraw;
class QwtScaleDiv;

class QWT_EXPORT QwtCircularPlot: public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool visibleBackground READ hasVisibleBackground WRITE showBackground)
    Q_PROPERTY(double origin READ origin WRITE setOrigin)

public:

    explicit QwtCircularPlot( QWidget *parent = NULL);
#if QT_VERSION < 0x040000
    explicit QwtCircularPlot( QWidget *parent, const char *name);
#endif

    virtual ~QwtCircularPlot();

    bool hasVisibleBackground() const;
    void showBackground(bool);

    void setCanvasBackground (const QColor &c);
    const QColor& canvasBackground() const;

    void setAutoReplot(bool tf = true); 
    bool autoReplot() const;

    void enableScale(bool enable = true);
    bool isScaleEnabled() const;

    void setScaleMaxMinor(int maxMinor);
    int scaleMaxMinor() const;

    int scaleMaxMajor() const;
    void setScaleMaxMajor(int maxMajor);

    QwtScaleEngine *scaleEngine();
    const QwtScaleEngine *scaleEngine() const;
    void setScaleEngine(QwtScaleEngine *);

    void setScale(double min, double max, double step = 0);

    void setScaleDraw(QwtRoundScaleDraw *);
    const QwtRoundScaleDraw *scaleDraw() const;
    QwtRoundScaleDraw *scaleDraw();

    void setScaleDiv(const QwtScaleDiv &);
    const QwtScaleDiv *scaleDiv() const;
    QwtScaleDiv *scaleDiv();

	QwtScaleMap scaleMap() const;

    double origin() const;

    QRect boundingRect() const;
    QRect canvasRect() const;

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

public slots:
    virtual void setOrigin(double);

    virtual void replot();
    void autoRefresh();

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);

    virtual void updateMask();
    virtual void drawCanvas(QPainter *) const = 0;

	virtual void drawScale(QPainter *, const QRect &) const;
	virtual void drawCanvas(QPainter *, const QRect &) const;

	void updateScale();

private:
    void init();
	void drawAll(QPainter *) const;

    class ScaleData;
    class PrivateData;
    PrivateData *d_data;
};

#endif
