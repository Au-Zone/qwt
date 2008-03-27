/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_RADIAL_PLOT_CURVE_H
#define QWT_RADIAL_PLOT_CURVE_H

#include "qwt_global.h"
#include "qwt_data.h"
#include "qwt_radial_plot_item.h"

class QPainter;
class QwtSymbol;

class QWT_EXPORT QwtRadialPlotCurve: public QwtRadialPlotItem
{
public:
    enum CurveStyle
    {
        NoCurve,
        Lines,
        UserCurve = 100
    };

    explicit QwtRadialPlotCurve();
    explicit QwtRadialPlotCurve(const QwtText &title);
    explicit QwtRadialPlotCurve(const QString &title);

    virtual ~QwtRadialPlotCurve();

    virtual int rtti() const;

    void setData(const QwtData &data);
    QwtData &data();
    const QwtData &data() const;

    int dataSize() const;
    inline double distance(int i) const;
    inline double angle(int i) const;

    void setPen(const QPen &);
    const QPen &pen() const;

    void setStyle(CurveStyle style);
    CurveStyle style() const;

    void setSymbol(const QwtSymbol &s);
    const QwtSymbol& symbol() const;

    virtual void draw(QPainter *p, 
        const QwtScaleMap &distanceMap, const QwtScaleMap &angleMap,
        const QRect &) const;

    virtual void draw(QPainter *p,
        const QPoint &center,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        int from, int to) const;

protected:

    void init();

    virtual void drawCurve(QPainter *p, int style,
        const QPoint &center,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        int from, int to) const;

    virtual void drawSymbols(QPainter *p, const QwtSymbol &,
        const QPoint &center,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        int from, int to) const;

    void drawLines(QPainter *p, const QPoint &center,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        int from, int to) const;

private:
    QwtData *d_points;

    class PrivateData;
    PrivateData *d_data;
};

//! \return the the curve data
inline QwtData &QwtRadialPlotCurve::data()
{
    return *d_points;
}

//! \return the the curve data
inline const QwtData &QwtRadialPlotCurve::data() const
{
    return *d_points;
}

/*!
    \param i index
    \return x-value at position i
*/
inline double QwtRadialPlotCurve::distance(int i) const 
{ 
    return d_points->x(i); 
}

/*!
    \param i index
    \return y-value at position i
*/
inline double QwtRadialPlotCurve::angle(int i) const 
{ 
    return d_points->y(i); 
}

#endif
