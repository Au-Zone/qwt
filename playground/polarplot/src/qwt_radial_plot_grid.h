/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_RADIAL_PLOT_GRID_H
#define QWT_RADIAL_PLOT_GRID_H

#include "qwt_global.h"
#include "qwt_radial_plot_item.h"
#include "qwt_radial_plot.h"
#include "qwt_scale_div.h"

class QPainter;
class QPen;
class QwtScaleMap;
class QwtScaleDiv;
class QwtRoundScaleDraw;
class QwtScaleDraw;

class QWT_EXPORT QwtRadialPlotGrid: public QwtRadialPlotItem
{
public:
    enum Axis
    {
        AngleAxis,

        LeftAxis,
        RightAxis,
        TopAxis,
        BottomAxis,

        AxesCount
    };

    enum DisplayFlag
    {
        SmartOriginLabel = 1,
        HideMaxDistanceValue = 2,
        ClipAxisBackground = 4,
        SmartScaleDraw = 8,
    };

    explicit QwtRadialPlotGrid();
    virtual ~QwtRadialPlotGrid();

    virtual int rtti() const;

    void setDisplayFlag(DisplayFlag, bool on = true);
    bool testDisplayFlag(DisplayFlag) const;

    void showGrid(int scaleId, bool show = true);
    bool isGridVisible(int scaleId) const;

    void showMinorGrid(int scaleId, bool show = true);
    bool isMinorGridVisible(int scaleId) const;

    void showAxis(int axisId, bool show = true);
    bool isAxisVisible(int axisId) const;

    void setScaleDiv(int scaleId, const QwtScaleDiv &sx);
    QwtScaleDiv scaleDiv(int scaleId) const;

    void setPen(const QPen &p);

    void setMajorGridPen(const QPen &p);
    void setMajorGridPen(int scaleId, const QPen &p);
    QPen majorGridPen(int scaleId) const;

    void setMinorGridPen(const QPen &p);
    void setMinorGridPen(int scaleId, const QPen &p);
    QPen minorGridPen(int scaleId) const;

    virtual void draw(QPainter *p, 
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRect &rect) const;

    virtual void updateScaleDiv(const QwtScaleDiv&,
        const QwtScaleDiv&);

    virtual QRect canvasLayoutHint(const QRect &) const;

protected:
    void drawLines(QPainter *, const QPoint &center, int radius, 
        const QwtScaleMap &, const QwtValueList &) const;
    void drawCircles(QPainter *, const QPoint &,
        const QwtScaleMap &, const QwtValueList &) const;

    void drawAxis(QPainter *, int axisId) const;

private:
    void updateScaleDraws( const QwtScaleMap &, 
        const QwtScaleMap &, const QRect &) const;

private:
    class GridData;
    class AxisData;
    class PrivateData;
    PrivateData *d_data;
};

#endif
