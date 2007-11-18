/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_HISTOGRAM_H
#define QWT_PLOT_HISTOGRAM_H

#include <qglobal.h>
#include <qcolor.h>

#include "qwt_plot_seriesitem.h" 

class QwtIntervalData;
class QwtColumnSymbol;
class QString;

class QwtPlotHistogram: public QwtPlotSeriesItem<QwtIntervalSample>
{
public:
    explicit QwtPlotHistogram(const QString &title = QString::null);
    explicit QwtPlotHistogram(const QwtText &title);
    virtual ~QwtPlotHistogram();

    void setColor(const QColor &);
    QColor color() const;

    virtual int rtti() const;

    virtual QwtDoubleRect boundingRect() const;

    virtual void draw(QPainter *, const QwtScaleMap &xMap, 
        const QwtScaleMap &yMap, const QRect &) const;

    void setBaseline(double reference);
    double baseline() const;

    enum HistogramAttribute
    {
        Auto = 0,
        Xfy = 1
    };

    void setHistogramAttribute(HistogramAttribute, bool on = true);
    bool testHistogramAttribute(HistogramAttribute) const;

    void setSymbol(const QwtColumnSymbol&);
    const QwtColumnSymbol& symbol() const;

protected:
    virtual const QwtColumnSymbol *adjustedSymbol(const QwtIntervalSample &,
        const QwtColumnSymbol &) const;

private:
    void init();

    class PrivateData;
    PrivateData *d_data;
};

#endif
