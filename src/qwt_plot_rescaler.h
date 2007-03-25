/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_RESCALER_H
#define QWT_PLOT_RESCALER_H 1

#include "qwt_global.h"
#include "qwt_double_rect.h"
#include <qobject.h>

class QwtPlotCanvas;
class QwtPlot;
class QResizeEvent;

class QWT_EXPORT QwtPlotRescaler: public QObject
{
public:
    enum AxisResizeMode
    {
        KeepReferenceScale,
        KeepRatio
    };

    enum ExpandingDirection
    {
        ExpandUp,
        ExpandDown,
        ExpandBoth
    };

    explicit QwtPlotRescaler(QwtPlotCanvas *);
    virtual ~QwtPlotRescaler();

    void manageAxis(int axis, bool on = true);
    bool isAxisManaged(int axis) const;
    
    void setReferenceAxisResizeMode(AxisResizeMode);
    AxisResizeMode referenceAxisResizeMode() const;

    void setReferenceAxis(int axis);
    int referenceAxis() const;

    void setScaleRatio(double ratio);
    void setScaleRatio(int axis, double ratio);
    double scaleRatio(int axis) const;

    void setExpandingDirection(ExpandingDirection);
    void setExpandingDirection(int axis, ExpandingDirection);
    ExpandingDirection expandingDirection(int axis) const;

    QwtPlotCanvas *canvas();
    const QwtPlotCanvas *canvas() const;

    QwtPlot *plot();
    const QwtPlot *plot() const;

    virtual bool eventFilter(QObject *, QEvent *);

    void rescale() const;

protected:
    virtual void canvasResizeEvent(QResizeEvent *);

    virtual QwtDoubleInterval expandScale( int axis, 
        const QSize &oldSize, const QSize &newSize) const;
 
    virtual QwtDoubleInterval syncScale(
        int axis, const QwtDoubleInterval& reference,
        const QSize &size) const; 

    virtual void updateScales(
        QwtDoubleInterval intervals[QwtPlot::axisCnt]) const;

    Qt::Orientation orientation(int axis) const;
    QwtDoubleInterval interval(int axis) const;
    QwtDoubleInterval expandInterval(const QwtDoubleInterval &, 
        double width, ExpandingDirection) const;

private:
    class AxisData;
    class PrivateData;
    PrivateData *d_data;
};

#endif
