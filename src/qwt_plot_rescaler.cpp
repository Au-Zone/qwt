/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#include <qevent.h>
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"
#include "qwt_scale_div.h"
#include "qwt_double_interval.h"
#include "qwt_plot_rescaler.h"

class QwtPlotRescaler::AxisData
{
public:
    AxisData():
        isManaged(true),
        expandingDirection(QwtPlotRescaler::ExpandUp),
        ratio(1.0)
    {
    }

    bool isManaged;
    QwtPlotRescaler::ExpandingDirection expandingDirection;
    double ratio;
};

class QwtPlotRescaler::PrivateData
{
public:
    PrivateData():
        referenceAxis(QwtPlot::xBottom),
        referenceAxisResizeMode(QwtPlotRescaler::KeepRatio)
    {
    }

    int referenceAxis;
    AxisResizeMode referenceAxisResizeMode;
    QwtPlotRescaler::AxisData axisData[QwtPlot::axisCnt];
};

QwtPlotRescaler::QwtPlotRescaler(QwtPlotCanvas *canvas):
    QObject(canvas)
{
    d_data = new PrivateData;
    canvas->installEventFilter(this);
}

QwtPlotRescaler::~QwtPlotRescaler()
{
    delete d_data;
}

void QwtPlotRescaler::manageAxis(int axis, bool on)
{
    if ( axis >= 0 && axis < QwtPlot::axisCnt )
        d_data->axisData[axis].isManaged = on;
}

bool QwtPlotRescaler::isAxisManaged(int axis) const
{
    if ( axis < 0 || axis >= QwtPlot::axisCnt )
        return false;

    return d_data->axisData[axis].isManaged;
}

void QwtPlotRescaler::setReferenceAxisResizeMode(AxisResizeMode mode)
{
    d_data->referenceAxisResizeMode = mode;
}

QwtPlotRescaler::AxisResizeMode 
QwtPlotRescaler::referenceAxisResizeMode() const
{
    return d_data->referenceAxisResizeMode;
}

void QwtPlotRescaler::setReferenceAxis(int axis)
{
    d_data->referenceAxis = axis;
}

int QwtPlotRescaler::referenceAxis() const
{
    return d_data->referenceAxis;
}

void QwtPlotRescaler::setExpandingDirection(
    ExpandingDirection direction)
{
    for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
        setExpandingDirection(axis, direction);
}

void QwtPlotRescaler::setExpandingDirection(
    int axis, ExpandingDirection direction)
{
    if ( axis >= 0 && axis < QwtPlot::axisCnt )
        d_data->axisData[axis].expandingDirection = direction;
}

void QwtPlotRescaler::setScaleRatio(double ratio)
{
    for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
        setScaleRatio(axis, ratio);
}

void QwtPlotRescaler::setScaleRatio(int axis, double ratio)
{
    if ( ratio > 0.0 && axis >= 0 && axis < QwtPlot::axisCnt )
        d_data->axisData[axis].ratio = ratio;
}

double QwtPlotRescaler::scaleRatio(int axis) const
{
    if ( axis >= 0 && axis < QwtPlot::axisCnt )
        return d_data->axisData[axis].ratio;

    return 1.0;
}

QwtPlotRescaler::ExpandingDirection
QwtPlotRescaler::expandingDirection(int axis) const
{
    if ( axis >= 0 && axis < QwtPlot::axisCnt )
        return d_data->axisData[axis].expandingDirection;

    return ExpandBoth;
}

QwtPlotCanvas *QwtPlotRescaler::canvas()
{
    QObject *o = parent();
    if ( o && o->inherits("QwtPlotCanvas") )
        return (QwtPlotCanvas *)o;

    return NULL;
}

const QwtPlotCanvas *QwtPlotRescaler::canvas() const
{
    return ((QwtPlotRescaler *)this)->canvas();
}

QwtPlot *QwtPlotRescaler::plot()
{
    QObject *w = canvas();
    if ( w )
    {
        w = w->parent();
        if ( w && w->inherits("QwtPlot") )
            return (QwtPlot *)w;
    }

    return NULL;
}

const QwtPlot *QwtPlotRescaler::plot() const
{
    return ((QwtPlotRescaler *)this)->plot();
}

bool QwtPlotRescaler::eventFilter(QObject *o, QEvent *e)
{
    if ( o && o == canvas() )
    {
        if ( e->type() == QEvent::Resize )
            canvasResizeEvent((QResizeEvent *)e);
    }

    return false;
}

void QwtPlotRescaler::canvasResizeEvent(QResizeEvent* e)
{
    if ( ! ( e->oldSize().isValid() && e->size().isValid() ) )
        return;

    const int fw = 2 * canvas()->frameWidth();
    const QSize newSize = e->size() - QSize(fw, fw);
    const QSize oldSize = e->oldSize() - QSize(fw, fw);

    QwtDoubleInterval intervals[QwtPlot::axisCnt];
    for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
        intervals[axis] = interval(axis);

    const int refAxis = referenceAxis();
    intervals[refAxis] = expandScale(refAxis, oldSize, newSize);

    for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
    {
        if ( isAxisManaged(axis) && axis != refAxis )
            intervals[axis] = syncScale(axis, intervals[refAxis], newSize);
    }

    updateScales(intervals);
}

void QwtPlotRescaler::rescale() const
{
    QwtDoubleInterval intervals[QwtPlot::axisCnt];
    for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
        intervals[axis] = interval(axis);

    const int refAxis = referenceAxis();
    for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
    {
        if ( isAxisManaged(axis) && axis != refAxis )
        {
            intervals[axis] = syncScale(axis, 
                intervals[refAxis], canvas()->contentsRect().size());
        }
    }

    updateScales(intervals);
}

QwtDoubleInterval QwtPlotRescaler::expandScale( int axis,
    const QSize &oldSize, const QSize &newSize) const
{
    const QwtDoubleInterval oldInterval = interval(axis);

    QwtDoubleInterval expanded = oldInterval;
    if ( referenceAxisResizeMode() == KeepRatio )
    {
        double width = oldInterval.width();
        if ( orientation(axis) == Qt::Horizontal )
            width *= double(newSize.width()) / oldSize.width();
        else
            width *= double(newSize.height()) / oldSize.height();

        expanded = expandInterval(oldInterval, 
            width, expandingDirection(axis));
    }

    return expanded;
}

QwtDoubleInterval QwtPlotRescaler::syncScale(int axis, 
    const QwtDoubleInterval& reference, const QSize &size) const 
{
    double dist;
    if ( orientation(referenceAxis()) == Qt::Horizontal )
        dist = reference.width() / size.width();
    else
        dist = reference.width() / size.height();

    if ( orientation(axis) == Qt::Horizontal )
        dist *= size.width();
    else
        dist *= size.height();

    dist *= scaleRatio(axis);

    QwtDoubleInterval intv = interval(axis);
    intv = expandInterval(intv, dist, expandingDirection(axis));

    return intv;
}

void QwtPlotRescaler::updateScales(
    QwtDoubleInterval intervals[QwtPlot::axisCnt]) const
{
    QwtPlot *plt = (QwtPlot *)plot();

    const bool doReplot = plt->autoReplot();
    plt->setAutoReplot(false);

    for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
    {
        if ( isAxisManaged(axis) )
        {
            double v1 = intervals[axis].minValue();
            double v2 = intervals[axis].maxValue();

            plt->setAxisScale(axis, v1, v2);
        }
    }

    plt->setAutoReplot(doReplot);
    plt->replot();
}

Qt::Orientation QwtPlotRescaler::orientation(int axis) const
{
    if ( axis == QwtPlot::yLeft || axis == QwtPlot::yRight )
        return Qt::Vertical;

    return Qt::Horizontal;
}

QwtDoubleInterval QwtPlotRescaler::interval(int axis) const
{
    if ( axis < 0 || axis >= QwtPlot::axisCnt )
        return QwtDoubleInterval();

    const QwtPlot *plt = plot();

    const double v1 = plt->axisScaleDiv(axis)->lBound();
    const double v2 = plt->axisScaleDiv(axis)->hBound();

    return QwtDoubleInterval(v1, v2);
}

QwtDoubleInterval QwtPlotRescaler::expandInterval(
    const QwtDoubleInterval &interval, double width,    
    ExpandingDirection direction) const
{
    QwtDoubleInterval expanded = interval;

    switch(direction)
    {
        case ExpandUp:
            expanded.setMinValue(interval.minValue());
            expanded.setMaxValue(interval.minValue() + width);
            break;
        case ExpandDown:
            expanded.setMaxValue(interval.maxValue());
            expanded.setMinValue(interval.maxValue() - width);
            break;
        case ExpandBoth:
        default:
            expanded.setMinValue(interval.minValue() +
                interval.width() / 2.0 - width / 2.0);
            expanded.setMaxValue(expanded.minValue() + width);
    }
    return expanded;
}
