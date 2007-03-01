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

class QWT_EXPORT QwtPlotIntervalCurve: public QwtPlotSeriesItem<QwtIntervalSample>
{
public:
    enum CurveStyle
    {
        NoCurve,
        Tube,

        UserTube = 100
    };

    enum SymbolStyle
    {
        NoSymbol,
        Bar,
    
        UserSymbol
    };

    explicit QwtPlotIntervalCurve(const QString &title = QString::null);
    explicit QwtPlotIntervalCurve(const QwtText &title);

    virtual ~QwtPlotIntervalCurve();

    virtual int rtti() const;

    void setData(const QwtArray<QwtIntervalSample> &data);
    void setData(const QwtSeriesData<QwtIntervalSample> &data);

    void setPen(const QPen &);
    const QPen &pen() const;

    void setBrush(const QBrush &);
    const QBrush &brush() const;

    void setCurveStyle(CurveStyle style);
    CurveStyle curveStyle() const;

    void setSymbolStyle(SymbolStyle);
    const SymbolStyle& symbolStyle() const;

    virtual void draw(QPainter *p, 
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRect &) const;

    virtual void draw(QPainter *p, 
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        int from, int to) const;

    virtual void updateLegend(QwtLegend *) const;

protected:

    void init();

    virtual void drawTube(QPainter *p, 
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        int from, int to) const;

    virtual void drawSymbols(QPainter *p, 
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        int from, int to) const;

    virtual void drawBar(QPainter *painter, int sampleId,
        const QPoint& from, const QPoint& to) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
