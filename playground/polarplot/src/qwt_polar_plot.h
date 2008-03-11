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

class QwtRoundScaleDraw;
class QwtScaleEngine;
class QwtScaleDraw;
class QwtScaleDiv;

class QWT_EXPORT QwtPolarPlot: public QWidget
{
    Q_OBJECT

    Q_ENUMS(Shadow)

    Q_PROPERTY(bool visibleBackground READ hasVisibleBackground WRITE showBackground)
    Q_PROPERTY(Shadow frameShadow READ frameShadow WRITE setFrameShadow)
    Q_PROPERTY(Shadow canvasFrameShadow 
        READ canvasFrameShadow WRITE setCanvasFrameShadow)
    Q_PROPERTY(double origin READ origin WRITE setOrigin)

public:
    /*!
        \brief Frame shadow

         Unfortunately it is not possible to use QFrame::Shadow
         as a property of a widget that is not derived from QFrame.
         The following enum is made for the designer only. It is safe
         to use QFrame::Shadow instead.
     */
    enum Shadow
    {
        Plain = QFrame::Plain,
        Raised = QFrame::Raised,
        Sunken = QFrame::Sunken
    };

    enum Axis 
    { 
        OuterAxis,

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

    void setFrameShadow(Shadow);
    Shadow frameShadow() const;

    void setCanvasFrameShadow(Shadow);
    Shadow canvasFrameShadow() const;

    bool hasVisibleBackground() const;
    void showBackground(bool);

    void setAutoReplot(bool tf = true); 
    bool autoReplot() const;

    virtual QwtScaleMap canvasMap(int axisId) const;

    void enableAxis(Axis axisId, bool enable);
    bool axisEnabled(Axis axisId) const;

    void setAxisMaxMinor(int axisId, int maxMinor);
    int axisMaxMajor(int axisId) const;
    void setAxisMaxMajor(int axisId, int maxMajor);
    int axisMaxMinor(int axisId) const;

    QwtScaleEngine *axisScaleEngine(int axisId);
    const QwtScaleEngine *axisScaleEngine(int axisId) const;
    void setAxisScaleEngine(int axisId, QwtScaleEngine *);

    void setAxisScale(int axisId, double min, double max, double step = 0);
    void setAxisScaleDiv(int axisId, const QwtScaleDiv &);

    void setAxisScaleDraw(int axisId, QwtScaleDraw *);
    void setOuterAxisScaleDraw(QwtRoundScaleDraw *);

    const QwtScaleDiv *axisScaleDiv(int axisId) const;
    QwtScaleDiv *axisScaleDiv(int axisId);

    const QwtScaleDraw *axisScaleDraw(int axisId) const;
    QwtScaleDraw *axisScaleDraw(int axisId);

    const QwtRoundScaleDraw *outerAxisScaleDraw() const;
    QwtRoundScaleDraw *outerAxisScaleDraw();

    virtual void setOrigin(double);
    double origin() const;

    QRect boundingRect() const;
    QRect contentsRect() const;
    QRect canvasContentsRect() const;
    QRect scaleContentsRect() const;

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

public slots:
    virtual void replot();
    void autoRefresh();

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);

    virtual void updateMask();

    virtual void drawFrame(QPainter *p);
    virtual void drawCanvas(QPainter *) const;
    virtual void drawItems(QPainter *, const QRect &,
        const QwtScaleMap maps[AxisCnt]) const;

private:
    void initPlot();
    void initAxesData();

    class AxisData;
    AxisData *d_axisData[AxisCnt];

    class PrivateData;
    PrivateData *d_data;
};

#endif
