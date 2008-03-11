/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_scale_engine.h"
#include "qwt_scale_div.h"
#include "qwt_scale_draw.h"
#include "qwt_round_scale_draw.h"
#include "qwt_polar_plot.h"

class QwtPolarPlot::AxisData
{
public:
    bool isEnabled;
    bool doAutoScale;

    double minValue;
    double maxValue;
    double stepSize;

    int maxMajor;
    int maxMinor;

    QwtScaleDiv scaleDiv;
    QwtScaleEngine *scaleEngine;
    QwtAbstractScaleDraw *scaleDraw;
};

class QwtPolarPlot::PrivateData
{
public:
    bool visibleBackground;
    Shadow frameShadow;
    Shadow canvasFrameShadow;
    double origin;

    bool autoReplot;
};

static inline bool axisValid(int axis)
{
    return (axis >= 0 && axis < QwtPolarPlot::AxisCnt);
    
}

QwtPolarPlot::QwtPolarPlot( QWidget *parent):
    QWidget(parent)
{
    initPlot();
}

#if QT_VERSION < 0x040000
QwtPolarPlot::QwtPolarPlot( QWidget *parent, const char *name):
    QWidget(parent, name)
{
    initPlot();
}
#endif

QwtPolarPlot::~QwtPolarPlot()
{
    delete d_data;
}

void QwtPolarPlot::setAutoReplot(bool enable)
{
    d_data->autoReplot = enable;
}

bool QwtPolarPlot::autoReplot() const
{
    return d_data->autoReplot;
}

void QwtPolarPlot::setFrameShadow(Shadow shadow)
{
    if ( shadow != d_data->frameShadow )
    {
        d_data->frameShadow = shadow;
        update();
    }
}

QwtPolarPlot::Shadow QwtPolarPlot::frameShadow() const
{
    return d_data->frameShadow;
}


void QwtPolarPlot::setCanvasFrameShadow(Shadow shadow)
{
    if ( shadow != d_data->canvasFrameShadow )
    {
        d_data->canvasFrameShadow = shadow;
        update();
    }
}

QwtPolarPlot::Shadow QwtPolarPlot::canvasFrameShadow() const
{
    return d_data->canvasFrameShadow;
}

bool QwtPolarPlot::hasVisibleBackground() const
{
    return d_data->visibleBackground;
}

void QwtPolarPlot::showBackground(bool enable)
{
    if ( enable != d_data->visibleBackground )
    {
        d_data->visibleBackground = enable;
        updateMask();
    }
}

QwtScaleMap QwtPolarPlot::canvasMap(int axisId) const
{
    QwtScaleMap map;
    if ( !axisValid(axisId) )
        return map;

    map.setTransformation(axisScaleEngine(axisId)->transformation());

    const QwtScaleDiv *sd = axisScaleDiv(axisId);
    map.setScaleInterval(sd->lBound(), sd->hBound());

    if ( axisId == OuterAxis )
    {
        map.setPaintInterval(0, 5760); // 16 * 360, see QPainter
    }
    else
    {
        const QRect r = canvasContentsRect();
        switch(axisId)
        {
            case TopAxis:
            {
                map.setPaintInterval(r.center().y(), r.top());
                break;
            }
            case BottomAxis:
            {
                map.setPaintInterval(r.center().y(), r.bottom());
                break;
            }
            case LeftAxis:
            {
                map.setPaintInterval(r.center().x(), r.left());
                break;
            }
            case RightAxis:
            {
                map.setPaintInterval(r.center().x(), r.right());
                break;
            }
            default:;
        }
    }

    return map;
}


void QwtPolarPlot::enableAxis(Axis axisId, bool enable)
{
    if ( !axisValid(axisId) )
        return;

    if ( d_axisData[axisId]->isEnabled != enable )
    {
        d_axisData[axisId]->isEnabled = enable;
        autoRefresh();
    }
}

bool QwtPolarPlot::axisEnabled(Axis axisId) const
{
    if ( !axisValid(axisId) )
        return false;

    return d_axisData[axisId]->isEnabled;
}


void QwtPolarPlot::setAxisMaxMinor(int axisId, int maxMinor)
{
    if (!axisValid(axisId))
        return;

    if ( maxMinor < 0 )
        maxMinor = 0;
    if ( maxMinor > 100 )
        maxMinor = 100;

    AxisData &d = *d_axisData[axisId];

    if ( maxMinor != d.maxMinor )
    {
        d.maxMinor = maxMinor;
        d.scaleDiv.invalidate();
        autoRefresh();
    }
}

int QwtPolarPlot::axisMaxMinor(int axisId) const
{
    if (!axisValid(axisId))
        return 0;

    return d_axisData[axisId]->maxMinor;
}

int QwtPolarPlot::axisMaxMajor(int axisId) const
{
    if (!axisValid(axisId))
        return 0;

    return d_axisData[axisId]->maxMajor;
}

void QwtPolarPlot::setAxisMaxMajor(int axisId, int maxMajor)
{
    if (!axisValid(axisId))
        return;

    if ( maxMajor < 1 )
        maxMajor = 1;
    if ( maxMajor > 1000 )
        maxMajor = 10000;

    AxisData &d = *d_axisData[axisId];
    if ( maxMajor != d.maxMinor )
    {
        d.maxMajor = maxMajor;
        d.scaleDiv.invalidate();
        autoRefresh();
    }

}

QwtScaleEngine *QwtPolarPlot::axisScaleEngine(int axisId)
{
    if (!axisValid(axisId))
        return NULL;

    return d_axisData[axisId]->scaleEngine;
}

const QwtScaleEngine *QwtPolarPlot::axisScaleEngine(int axisId) const
{
    if (!axisValid(axisId))
        return NULL;

    return d_axisData[axisId]->scaleEngine;
}

void QwtPolarPlot::setAxisScaleEngine(
    int axisId, QwtScaleEngine *scaleEngine)
{
    if (!axisValid(axisId) || scaleEngine == NULL )
        return;

    AxisData &d = *d_axisData[axisId];

    delete d.scaleEngine;
    d.scaleEngine = scaleEngine;

    d.scaleDiv.invalidate();

    autoRefresh();
}


void QwtPolarPlot::setAxisScale(int axisId, 
    double min, double max, double stepSize)
{
    if (!axisValid(axisId))
        return;

    AxisData &d = *d_axisData[axisId];

    d.doAutoScale = false;
    d.scaleDiv.invalidate();

    d.minValue = min;
    d.maxValue = max;
    d.stepSize = stepSize;

    autoRefresh();
}

void QwtPolarPlot::setAxisScaleDiv(int axisId, const QwtScaleDiv &scaleDiv)
{
    if (!axisValid(axisId))
        return;

    AxisData &d = *d_axisData[axisId];

    d.doAutoScale = false;
    d.scaleDiv = scaleDiv;

    autoRefresh();
}


void QwtPolarPlot::setAxisScaleDraw(int axisId, QwtScaleDraw *scaleDraw)
{
    if (!axisValid(axisId) || scaleDraw == NULL || axisId == OuterAxis)
        return;

    AxisData &d = *d_axisData[axisId];

    if ( scaleDraw != d.scaleDraw )
    {
        delete d.scaleDraw;
        d.scaleDraw = scaleDraw;
        autoRefresh();
    }
}

void QwtPolarPlot::setOuterAxisScaleDraw(QwtRoundScaleDraw *scaleDraw)
{
	if ( scaleDraw == NULL )
		return;

    AxisData &d = *d_axisData[OuterAxis];

    if ( scaleDraw != d.scaleDraw )
    {
        delete d.scaleDraw;
        d.scaleDraw = scaleDraw;
        autoRefresh();
    }
}


const QwtScaleDiv *QwtPolarPlot::axisScaleDiv(int axisId) const
{
    if (!axisValid(axisId))
        return NULL;

    return &d_axisData[axisId]->scaleDiv;
}

QwtScaleDiv *QwtPolarPlot::axisScaleDiv(int axisId)
{
    if (!axisValid(axisId))
        return NULL;

    return &d_axisData[axisId]->scaleDiv;
}


const QwtScaleDraw *QwtPolarPlot::axisScaleDraw(int axisId) const
{
    if (!axisValid(axisId) || axisId == OuterAxis)
        return NULL;

    return (QwtScaleDraw *)d_axisData[axisId]->scaleDraw;
}

QwtScaleDraw *QwtPolarPlot::axisScaleDraw(int axisId)
{
    if (!axisValid(axisId) || axisId == OuterAxis)
        return NULL;

    return (QwtScaleDraw *)d_axisData[axisId]->scaleDraw;
}


const QwtRoundScaleDraw *QwtPolarPlot::outerAxisScaleDraw() const
{
    return (QwtRoundScaleDraw *)d_axisData[OuterAxis]->scaleDraw;
}

QwtRoundScaleDraw *QwtPolarPlot::outerAxisScaleDraw()
{
    return (QwtRoundScaleDraw *)d_axisData[OuterAxis]->scaleDraw;
}


void QwtPolarPlot::setOrigin(double origin)
{
    if ( d_data->origin != origin )
    {
        d_data->origin = origin;
        autoRefresh();
    }
}

double QwtPolarPlot::origin() const
{
    return d_data->origin;
}


QRect QwtPolarPlot::boundingRect() const
{
    return QRect();
}

QRect QwtPolarPlot::contentsRect() const
{
    return QRect();
}

QRect QwtPolarPlot::canvasContentsRect() const
{
    return QRect();
}

QRect QwtPolarPlot::scaleContentsRect() const
{
    return QRect();
}

QSize QwtPolarPlot::sizeHint() const
{
    return QSize();
}

QSize QwtPolarPlot::minimumSizeHint() const
{
    return QSize();
}

void QwtPolarPlot::paintEvent(QPaintEvent *)
{
}

void QwtPolarPlot::resizeEvent(QResizeEvent *)
{
}

void QwtPolarPlot::updateMask()
{
}

void QwtPolarPlot::drawFrame(QPainter *)
{
}

void QwtPolarPlot::drawCanvas(QPainter *) const
{
}

void QwtPolarPlot::drawItems(QPainter *, const QRect &,
    const QwtScaleMap[AxisCnt]) const
{
}

void QwtPolarPlot::initPlot()
{
    d_data = new PrivateData;

    d_data->autoReplot = false;
    d_data->visibleBackground = true;
    d_data->frameShadow = Plain;
    d_data->canvasFrameShadow = Sunken;
    d_data->origin = 90.0;
    
    initAxesData();
    
    setSizePolicy(QSizePolicy::MinimumExpanding,
        QSizePolicy::MinimumExpanding);
}

void QwtPolarPlot::initAxesData()
{
    for( int axisId = 0; axisId < AxisCnt; axisId++)
    {
        d_axisData[axisId] = new AxisData;
        AxisData &d = *d_axisData[axisId];

        d.doAutoScale = true;

        d.minValue = 0.0;
        d.maxValue = 1000.0;
        d.stepSize = 0.0;

        d.maxMinor = 5;
        d.maxMajor = 8;

        d.scaleEngine = new QwtLinearScaleEngine;

        d.scaleDiv.invalidate();

		if ( axisId == OuterAxis )
		{
    		d.scaleDraw = new QwtRoundScaleDraw();
		}
		else
		{
    		QwtScaleDraw *scaleDraw = new QwtScaleDraw();
			switch(axisId)
			{
				case RightAxis:
				case LeftAxis:
					scaleDraw->setAlignment(QwtScaleDraw::BottomScale);
					break;
				case TopAxis:
				case BottomAxis:
					scaleDraw->setAlignment(QwtScaleDraw::LeftScale);
					break;
			}
			d.scaleDraw = scaleDraw;
		}
    }
}

void QwtPolarPlot::autoRefresh()
{
}

void QwtPolarPlot::replot()
{
}
