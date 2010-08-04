/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_INTERVAL_CURVE_H
#define QWT_PLOT_INTERVAL_CURVE_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"
#include "qwt_series_data.h"

class QwtIntervalSymbol;

/*!
  \brief QwtPlotIntervalCurve represents a series of samples, where each value
         is associated with an interval ( \f$[y1,y2] = f(x)\f$ ).

  The representation depends on the style() and an optional symbol()
  that is displayed for each interval. QwtPlotIntervalCurve might be used
  to disply error bars or the area between 2 curves.
*/

class QWT_EXPORT QwtPlotIntervalCurve: public QwtPlotSeriesItem<QwtIntervalSample>
{
public:
    /*! 
        Curve styles. 

         - NoCurve\n
           Don't draw a curve. Note: This doesn't affect the symbols.

         - Tube\n
           Build 2 curves from the upper and lower limits of the intervals
           and draw them with the pen(). The area between the curves is 
           filled with the brush(). 

         - UserCurve\n
           Styles >= UserCurve are reserved for derived
           classes of QwtPlotIntervalCurve that overload drawSeries() with
           additional application specific curve types.

        The default setting is Tube.

        \sa setStyle(), style()
    */

    enum CurveStyle
    {
        NoCurve,

        Tube,

        UserCurve = 100
    };

    explicit QwtPlotIntervalCurve(const QString &title = QString::null);
    explicit QwtPlotIntervalCurve(const QwtText &title);

    virtual ~QwtPlotIntervalCurve();

    virtual int rtti() const;

    void setSamples(const QVector<QwtIntervalSample> &);

    void setPen(const QPen &);
    const QPen &pen() const;

    void setBrush(const QBrush &);
    const QBrush &brush() const;

    void setStyle(CurveStyle style);
    CurveStyle style() const;

    void setSymbol(const QwtIntervalSymbol *);
    const QwtIntervalSymbol *symbol() const;

    virtual void drawSeries(QPainter *p, 
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to) const;

    virtual QRectF boundingRect() const;
    virtual void drawLegendIdentifier(QPainter *, const QRectF &) const;

protected:

    void init();

    virtual void drawTube(QPainter *, 
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        int from, int to) const;

    virtual void drawSymbols(QPainter *, const QwtIntervalSymbol &, 
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        int from, int to) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
