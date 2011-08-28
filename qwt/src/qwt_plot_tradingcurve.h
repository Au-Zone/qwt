/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_TRADING_CURVE_H
#define QWT_PLOT_TRADING_CURVE_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"
#include "qwt_series_data.h"

class QWT_EXPORT QwtPlotTradingCurve: public QwtPlotSeriesItem<QwtOHLCSample>
{
public:

    enum SymbolStyle
    {
        NoSymbol = -1,

        Bar,
        CandleStick,

        UserSymbol = 100
    };

    explicit QwtPlotTradingCurve( const QString &title = QString::null );
    explicit QwtPlotTradingCurve( const QwtText &title );

    virtual ~QwtPlotTradingCurve();

    virtual int rtti() const;

    void setSamples( const QVector<QwtOHLCSample> & );

    void setSymbolStyle( SymbolStyle style );
    SymbolStyle symbolStyle() const;

    virtual void drawSeries( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to ) const;

    virtual QRectF boundingRect() const;
    virtual void drawLegendIdentifier( QPainter *, const QRectF & ) const;

protected:

    void init();

    virtual void drawSymbols( QPainter *,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to ) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
