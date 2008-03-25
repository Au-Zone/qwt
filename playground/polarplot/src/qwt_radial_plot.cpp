/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qpainter.h>
#include <qevent.h>
#include <qpaintengine.h>
#include "qwt_painter.h"
#include "qwt_math.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_div.h"
#include "qwt_round_scale_draw.h"
#include "qwt_round_scale_draw.h"
#include "qwt_radial_plot.h"
#if 1
#include <QDebug>
#endif

#if QT_VERSION >= 0x040000
static void setAntialiasing(QPainter *painter, bool on)
{
    QPaintEngine *engine = painter->paintEngine();
    if ( engine && engine->hasFeature(QPaintEngine::Antialiasing) )
        painter->setRenderHint(QPainter::Antialiasing, on);
}
#else
static void setAntialiasing(QPainter *, bool)
{
}
#endif

class QwtRadialPlot::ScaleData
{
public:
    ScaleData():
        scaleEngine(NULL)
    {
    }

    ~ScaleData()
    {
        delete scaleEngine;
    }

    bool doAutoScale;

    double minValue;
    double maxValue;
    double stepSize;

    int maxMajor;
    int maxMinor;

    QwtScaleDiv scaleDiv;
    QwtScaleEngine *scaleEngine;
    QwtDoubleInterval zoomedInterval;
};

class QwtRadialPlot::PrivateData
{
public:
    QRegion::RegionType shape;
    QBrush canvasBrush;

    bool autoReplot;
    double origin;

    bool isAngleScaleVisible;
    QwtRoundScaleDraw *angleScaleDraw;

    ScaleData scaleData[ScaleCount];
};

QwtRadialPlot::QwtRadialPlot( QWidget *parent):
    QWidget(parent)
{
    initPlot();
}

#if QT_VERSION < 0x040000
QwtRadialPlot::QwtRadialPlot( QWidget *parent, const char *name):
    QWidget(parent, name)
{
    initPlot();
}
#endif

QwtRadialPlot::~QwtRadialPlot()
{
    delete d_data;
}

void QwtRadialPlot::setAutoReplot(bool enable)
{
    d_data->autoReplot = enable;
}

bool QwtRadialPlot::autoReplot() const
{
    return d_data->autoReplot;
}

void QwtRadialPlot::showAngleScale(bool enable)
{
    if ( d_data->isAngleScaleVisible != enable )
    {
        d_data->isAngleScaleVisible = enable;
        update();
    }
}

bool QwtRadialPlot::isAngleScaleVisible() const
{
    return d_data->isAngleScaleVisible;
}

void QwtRadialPlot::setOrigin(double origin)
{
	// 0.0 -> 2 * PI
    if ( d_data->origin != origin )
    {
        d_data->origin = origin;
        autoRefresh();
    }
}

double QwtRadialPlot::origin() const
{
    return d_data->origin;
}

void QwtRadialPlot::setAngleScaleDraw(QwtRoundScaleDraw *scaleDraw)
{
    if ( scaleDraw == NULL || scaleDraw == d_data->angleScaleDraw )
        return;

    delete d_data->angleScaleDraw;
    d_data->angleScaleDraw = scaleDraw;
    autoRefresh();
}

const QwtRoundScaleDraw *QwtRadialPlot::angleScaleDraw() const
{
    return d_data->angleScaleDraw;
}

QwtRoundScaleDraw *QwtRadialPlot::angleScaleDraw()
{
    return d_data->angleScaleDraw;
}

void QwtRadialPlot::setScaleMaxMinor(int scaleId, int maxMinor)
{
    if ( scaleId < 0 || scaleId >= ScaleCount )
        return;

    if ( maxMinor < 0 )
        maxMinor = 0;
    if ( maxMinor > 100 )
        maxMinor = 100;

    ScaleData &scaleData = d_data->scaleData[scaleId];

    if ( maxMinor != scaleData.maxMinor )
    {
        scaleData.maxMinor = maxMinor;
        scaleData.scaleDiv.invalidate();
        autoRefresh();
    }
}

int QwtRadialPlot::scaleMaxMinor(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= ScaleCount )
        return 0;

    return d_data->scaleData[scaleId].maxMinor;
}

void QwtRadialPlot::setScaleMaxMajor(int scaleId, int maxMajor)
{
    if ( scaleId < 0 || scaleId >= ScaleCount )
        return;

    if ( maxMajor < 1 )
        maxMajor = 1;
    if ( maxMajor > 1000 )
        maxMajor = 10000;

    ScaleData &scaleData = d_data->scaleData[scaleId];
    if ( maxMajor != scaleData.maxMinor )
    {
        scaleData.maxMajor = maxMajor;
        scaleData.scaleDiv.invalidate();
        autoRefresh();
    }
}

int QwtRadialPlot::scaleMaxMajor(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= ScaleCount )
        return 0;

    return d_data->scaleData[scaleId].maxMajor;
}

QwtScaleEngine *QwtRadialPlot::scaleEngine(int scaleId)
{
    if ( scaleId < 0 || scaleId >= ScaleCount )
        return NULL;

    return d_data->scaleData[scaleId].scaleEngine;
}

const QwtScaleEngine *QwtRadialPlot::scaleEngine(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= ScaleCount )
        return NULL;

    return d_data->scaleData[scaleId].scaleEngine;
}

void QwtRadialPlot::setScaleEngine(int scaleId, QwtScaleEngine *scaleEngine)
{
    if ( scaleId < 0 || scaleId >= ScaleCount )
        return;

    ScaleData &scaleData = d_data->scaleData[scaleId];
    if (scaleEngine == NULL || scaleEngine == scaleData.scaleEngine )
        return;

    delete scaleData.scaleEngine;
    scaleData.scaleEngine = scaleEngine;

    scaleData.scaleDiv.invalidate();

    autoRefresh();
}

void QwtRadialPlot::setScale(int scaleId, 
    double min, double max, double stepSize)
{
    if ( scaleId < 0 || scaleId >= ScaleCount )
        return;

    ScaleData &scaleData = d_data->scaleData[scaleId];

    scaleData.scaleDiv.invalidate();

    scaleData.minValue = min;
    scaleData.maxValue = max;
    scaleData.stepSize = stepSize;
    scaleData.doAutoScale = false;

    autoRefresh();
}

void QwtRadialPlot::setScaleDiv(int scaleId, const QwtScaleDiv &scaleDiv)
{
    if ( scaleId < 0 || scaleId >= ScaleCount )
        return;

    ScaleData &scaleData = d_data->scaleData[scaleId];

    scaleData.scaleDiv = scaleDiv;
    scaleData.doAutoScale = false;

    autoRefresh();
}

const QwtScaleDiv *QwtRadialPlot::scaleDiv(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= ScaleCount )
        return NULL;

    return &d_data->scaleData[scaleId].scaleDiv;
}

QwtScaleDiv *QwtRadialPlot::scaleDiv(int scaleId)
{
    if ( scaleId < 0 || scaleId >= ScaleCount )
        return NULL;

    return &d_data->scaleData[scaleId].scaleDiv;
}

QwtScaleMap QwtRadialPlot::canvasMap(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= ScaleCount )
        return QwtScaleMap();

    QwtScaleMap map;
    map.setTransformation(scaleEngine(scaleId)->transformation());

    const QwtScaleDiv *sd = scaleDiv(scaleId);
    map.setScaleInterval(sd->lBound(), sd->hBound());

    if ( scaleId == AngleScale)
    {
		const double o = d_data->origin;
        map.setPaintXInterval(o, o + M_2PI); 
    }
    else
    {
        const int w = qwtMin(canvasRect().width(), canvasRect().height());
        map.setPaintInterval(canvasRect().center().x(), 
            canvasRect().center().x() + w / 2);
    }

    return map;
}

QSize QwtRadialPlot::sizeHint() const
{
    const int extent = scaleExtent();
    if ( extent > 0 )
    {
        const int d = 6 * (extent + 1);
        return QSize( d, d );
    }

    return QSize(400, 400); // something
}

QSize QwtRadialPlot::minimumSizeHint() const
{
    const int extent = scaleExtent();
    if ( extent > 0 )
    {
        const int d = 3 * (extent + 1);
        return QSize( d, d );
    }

    return QSize();
}

bool QwtRadialPlot::event(QEvent *e)
{
    bool ok = QWidget::event(e);
    switch(e->type())
    {
#if QT_VERSION >= 0x040000
        case QEvent::PolishRequest:
            polish();
            break;
#endif
        default:;
    }
    return ok;
}

void QwtRadialPlot::paintEvent(QPaintEvent *e)
{
    const QRect &ur = e->rect();
    if ( ur.isValid() )
    {
#if QT_VERSION < 0x040000
        QwtPaintBuffer paintBuffer(this, ur);
        QPainter &painter = *paintBuffer.painter();
#else
        QPainter painter(this);
#endif
        painter.save();
        drawCanvas(&painter, canvasRect());
        painter.restore();

        if ( isAngleScaleVisible() )
        {
            painter.save();
            drawAngleScale(&painter, boundingRect());
            painter.restore();
        }
    }
}

void QwtRadialPlot::initPlot()
{
#if QT_VERSION < 0x040000
    setWFlags(Qt::WNoAutoErase);
#endif

    d_data = new PrivateData;

    d_data->autoReplot = false;
    d_data->isAngleScaleVisible = true;
    d_data->angleScaleDraw = new QwtRoundScaleDraw();
    d_data->origin = 0.0;
    
    for ( int scaleId = DistanceScale; scaleId <= AngleScale; scaleId++ )
    {
        ScaleData &scaleData = d_data->scaleData[scaleId];
        
        if ( scaleId == AngleScale )
        {
#if 1
            scaleData.minValue = 0.0;
            scaleData.maxValue = 360.0;
            scaleData.stepSize = 15.0;
#endif
        }
        else
        {
            scaleData.minValue = 0.0;
            scaleData.maxValue = 1000.0;
            scaleData.stepSize = 0.0;
        }

        scaleData.doAutoScale = true;
        
        scaleData.maxMinor = 5;
        scaleData.maxMajor = 8;
        
        scaleData.scaleEngine = new QwtLinearScaleEngine;
        scaleData.scaleDiv.invalidate();

        updateScale(scaleId);
    }

    setSizePolicy(QSizePolicy::MinimumExpanding,
        QSizePolicy::MinimumExpanding);
}

void QwtRadialPlot::autoRefresh()
{
    if (d_data->autoReplot)
        replot();
}

void QwtRadialPlot::replot()
{
    for ( int scaleId = 0; scaleId < ScaleCount; scaleId++ )
        updateScale(scaleId);
}

void QwtRadialPlot::drawAngleScale(QPainter *painter, const QRect &rect) const
{
    QwtRoundScaleDraw *scaleDraw = d_data->angleScaleDraw;
    if ( scaleDraw == NULL )
        return;

    const QPoint center = rect.center();
    int radius = rect.width() / 2;
    radius -= scaleDraw->extent(QPen(), font()) - 1;

    painter->setFont(font());
    scaleDraw->setRadius(radius);
    scaleDraw->moveCenter(center);

    const double o = d_data->origin / M_PI * 180.0; // degree
    scaleDraw->setAngleRange(o + 90.0, o - 270.0);

#if QT_VERSION < 0x040000
    QColorGroup cg = colorGroup();

    const QColor textColor = cg.color(QColorGroup::Text);
    cg.setColor(QColorGroup::Foreground, textColor);

    scaleDraw->draw(painter, cg);
#else
    QPalette pal = palette();

    const QColor textColor = pal.color(QPalette::Text);
    pal.setColor(QPalette::Foreground, textColor); //ticks, backbone

    setAntialiasing(painter, true);
    scaleDraw->draw(painter, pal);
#endif
}

void QwtRadialPlot::drawCanvas(QPainter *painter, const QRect &rect) const
{
    QwtScaleMap maps[ScaleCount];
    for ( int scaleId = 0; scaleId < ScaleCount; scaleId++ )
        maps[scaleId] = canvasMap(scaleId);

    drawItems(painter, rect, maps);
}

void QwtRadialPlot::drawItems(QPainter *painter, const QRect &canvasRect,
        const QwtScaleMap map[ScaleCount]) const
{
    const QwtRadialPlotItemList& itmList = itemList();
    for ( QwtRadialPlotItemIterator it = itmList.begin();
        it != itmList.end(); ++it )
    {
        QwtRadialPlotItem *item = *it;
        if ( item && item->isVisible() )
        {
            painter->save();

#if QT_VERSION >= 0x040000
            painter->setRenderHint(QPainter::Antialiasing,
                item->testRenderHint(QwtRadialPlotItem::RenderAntialiased) );
#endif

            item->draw(painter,
                map[DistanceScale], map[AngleScale],
                canvasRect);

            painter->restore();
        }
    }
}

void QwtRadialPlot::updateScale(int scaleId)
{
    if ( scaleId < 0 || scaleId >= ScaleCount )
        return;

    ScaleData &d = d_data->scaleData[scaleId];
    if ( !d.scaleDiv.isValid() )
    {
        d.scaleDiv = d.scaleEngine->divideScale(
            d.minValue, d.maxValue,
            d.maxMajor, d.maxMinor, d.stepSize);
    }

    QwtRoundScaleDraw *scaleDraw = d_data->angleScaleDraw;
    if ( scaleDraw )
    {
        scaleDraw->setTransformation(d.scaleEngine->transformation());
        scaleDraw->setScaleDiv(d.scaleDiv);
    }

    const QwtRadialPlotItemList& itmList = itemList();

    QwtRadialPlotItemIterator it;

    for ( it = itmList.begin(); it != itmList.end(); ++it )
    {
        QwtRadialPlotItem *item = *it;
        item->updateScaleDiv( 
            *scaleDiv(DistanceScale), *scaleDiv(AngleScale));
    }
}

void QwtRadialPlot::polish()
{
    replot();

#if QT_VERSION < 0x040000
    QWidget::polish();
#endif
}

QRect QwtRadialPlot::boundingRect() const
{
    const int radius = qwtMin(width(), height()) / 2;

    QRect r(0, 0, 2 * radius, 2 * radius);
    r.moveCenter(rect().center());
    return r;
}

QRect QwtRadialPlot::canvasRect() const
{
    int scaleDist = scaleExtent();
#if 0
    if ( scaleDist > 0 )
        scaleDist++; // margin
#endif
    
    const QRect rect = boundingRect();
    return QRect(rect.x() + scaleDist, rect.y() + scaleDist,
        rect.width() - 2 * scaleDist, rect.height() - 2 * scaleDist);
}

int QwtRadialPlot::scaleExtent() const
{
    if ( isAngleScaleVisible() && d_data->angleScaleDraw )
        return d_data->angleScaleDraw->extent(QPen(), font());

    return 0;
}

