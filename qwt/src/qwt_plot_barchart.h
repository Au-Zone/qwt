/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_BAR_CHART_H
#define QWT_PLOT_BAR_CHART_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"
#include "qwt_series_data.h"

class QwtColumnRect;

class QWT_EXPORT QwtPlotBarChart: public QwtPlotSeriesItem<QwtSetSample>
{
public:
    enum ChartStyle
    {
        Stacked,
        Grouped
    };

    explicit QwtPlotBarChart( const QString &title = QString::null );
    explicit QwtPlotBarChart( const QwtText &title );

    virtual ~QwtPlotBarChart();

    virtual int rtti() const;

    void setSamples( const QVector<QwtSetSample> & );
    void setSamples( const QVector< QVector<double> > & );
    void setSamples( const QVector<double> & );

    void setStyle( ChartStyle style );
    ChartStyle style() const;

    void setBarWidth( double );
    double barWidth() const;

    void setBaseline( double );
    double baseline() const;

    virtual void drawSeries( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to ) const;

    virtual QRectF boundingRect() const;
    virtual void drawLegendIdentifier( QPainter *, const QRectF & ) const;

protected:
    virtual void drawSample( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, const QwtSetSample& sample ) const;

    virtual void drawBar( QPainter *, int index, const QwtColumnRect & ) const;

    void drawStackedBars( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, const QwtSetSample& sample ) const;

    void drawGroupedBars( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, const QwtSetSample& sample ) const;

    void init();

    class PrivateData;
    PrivateData *d_data;
};

#endif
