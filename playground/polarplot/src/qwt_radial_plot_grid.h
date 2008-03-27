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

    void showGrid(QwtRadialPlot::Scale, bool show = true);
    bool isGridVisible(QwtRadialPlot::Scale) const;

    void showMinorGrid(QwtRadialPlot::Scale, bool show = true);
    bool isMinorGridVisible(QwtRadialPlot::Scale) const;

	void showAxis(Axis, bool show = true);
	bool isAxisVisisble(Axis) const;

    void setScaleDiv(QwtRadialPlot::Scale, const QwtScaleDiv &sx);
    QwtScaleDiv scaleDiv(QwtRadialPlot::Scale) const;

    void setPen(const QPen &p);

    void setMajorGridPen(const QPen &p);
    void setMajorGridPen(QwtRadialPlot::Scale, const QPen &p);
    QPen majorGridPen(QwtRadialPlot::Scale) const;

    void setMinorGridPen(const QPen &p);
    void setMinorGridPen(QwtRadialPlot::Scale, const QPen &p);
    QPen minorGridPen(QwtRadialPlot::Scale) const;

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
