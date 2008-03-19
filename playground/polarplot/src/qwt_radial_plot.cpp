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
#include "qwt_scale_engine.h"
#include "qwt_scale_div.h"
#include "qwt_round_scale_draw.h"
#include "qwt_round_scale_draw.h"
#include "qwt_radial_plot.h"
#if 1
#include <QDebug>
#endif

class QwtRadialPlot::ScaleData
{
public:
    ScaleData():
        scaleEngine(NULL),
        scaleDraw(NULL)
    {
    }

    ~ScaleData()
    {
        delete scaleEngine;
        delete scaleDraw;
    }

    bool doAutoScale;

    double minValue;
    double maxValue;
    double stepSize;

    int maxMajor;
    int maxMinor;

    QwtScaleDiv scaleDiv;
    QwtScaleEngine *scaleEngine;

    // only for angle scales
    bool isVisible;
    QwtRoundScaleDraw *scaleDraw;
    double origin;
};

class QwtRadialPlot::PrivateData
{
public:
    QRegion::RegionType shape;
    QBrush canvasBrush;

    bool autoReplot;
    ScaleData scaleData[ScaleCount];
};

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

void QwtRadialPlot::setCanvasBackground(const QBrush &brush)
{
    if ( brush != d_data->canvasBrush )
    {
        d_data->canvasBrush = brush;
        autoRefresh();
    }
}

const QBrush& QwtRadialPlot::canvasBackground() const
{
    return d_data->canvasBrush;
}

void QwtRadialPlot::setAutoReplot(bool enable)
{
    d_data->autoReplot = enable;
}

bool QwtRadialPlot::autoReplot() const
{
    return d_data->autoReplot;
}

QRegion::RegionType QwtRadialPlot::shape() const
{
    return d_data->shape;
}

void QwtRadialPlot::setShape(QRegion::RegionType shape)
{
    if ( shape != d_data->shape )
    {
        d_data->shape = shape;
        updateMask();
    }
}

void QwtRadialPlot::showAngleScale(int scaleId, bool enable)
{
    if ( !isAngleScale(scaleId) )
        return;

    ScaleData &scaleData = d_data->scaleData[scaleId];
    if ( scaleData.isVisible != enable )
    {
        if ( enable )
        {
            // for now, we can only display 1 scale
            for ( int i = AngleScale1; i <= AngleScale4; i++ )
                d_data->scaleData[i].isVisible = false;
        }
            
        scaleData.isVisible = enable;
        update();
    }
}

bool QwtRadialPlot::isAngleScaleVisible(int scaleId) const
{
    if ( !isAngleScale(scaleId) )
        return false;

    return d_data->scaleData[scaleId].isVisible;
}

void QwtRadialPlot::setOrigin(int scaleId, double origin)
{
    if ( !isAngleScale(scaleId) )
        return;

    ScaleData &scaleData = d_data->scaleData[scaleId];
    if ( scaleData.origin != origin )
    {
        scaleData.origin = origin;
        autoRefresh();
    }
}

double QwtRadialPlot::origin(int scaleId) const
{
    if ( !isAngleScale(scaleId) )
        return 0.0;

    return d_data->scaleData[scaleId].origin;
}

void QwtRadialPlot::setAngleScaleDraw(int scaleId, QwtRoundScaleDraw *scaleDraw)
{
    if ( !isAngleScale(scaleId) || scaleDraw == NULL )
        return;

    ScaleData &scaleData = d_data->scaleData[scaleId];
    if ( scaleDraw != scaleData.scaleDraw )
    {
        delete scaleData.scaleDraw;
        scaleData.scaleDraw = scaleDraw;
        autoRefresh();
    }
}

const QwtRoundScaleDraw *QwtRadialPlot::angleScaleDraw(int scaleId) const
{
    if ( !isAngleScale(scaleId) )
        return NULL;

    return d_data->scaleData[scaleId].scaleDraw;
}

QwtRoundScaleDraw *QwtRadialPlot::angleScaleDraw(int scaleId)
{
    if ( !isAngleScale(scaleId) )
        return NULL;

    return d_data->scaleData[scaleId].scaleDraw;
}

void QwtRadialPlot::setScaleMaxMinor(int scaleId, int maxMinor)
{
    if ( !isValidScale(scaleId) )
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
    if ( !isValidScale(scaleId) )
        return 0;

    return d_data->scaleData[scaleId].maxMinor;
}

void QwtRadialPlot::setScaleMaxMajor(int scaleId, int maxMajor)
{
    if ( !isValidScale(scaleId) )
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
    if ( !isValidScale(scaleId) )
        return 0;

    return d_data->scaleData[scaleId].maxMajor;
}

QwtScaleEngine *QwtRadialPlot::scaleEngine(int scaleId)
{
    if ( !isValidScale(scaleId) )
        return NULL;

    return d_data->scaleData[scaleId].scaleEngine;
}

const QwtScaleEngine *QwtRadialPlot::scaleEngine(int scaleId) const
{
    if ( !isValidScale(scaleId) )
        return NULL;

    return d_data->scaleData[scaleId].scaleEngine;
}

void QwtRadialPlot::setScaleEngine(int scaleId, QwtScaleEngine *scaleEngine)
{
    if ( !isValidScale(scaleId) )
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
    if ( !isValidScale(scaleId) )
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
    if ( !isValidScale(scaleId) )
        return;

    ScaleData &scaleData = d_data->scaleData[scaleId];

    scaleData.scaleDiv = scaleDiv;
    scaleData.doAutoScale = false;

    autoRefresh();
}

const QwtScaleDiv *QwtRadialPlot::scaleDiv(int scaleId) const
{
    if ( !isValidScale(scaleId) )
        return NULL;

    return &d_data->scaleData[scaleId].scaleDiv;
}

QwtScaleDiv *QwtRadialPlot::scaleDiv(int scaleId)
{
    if ( !isValidScale(scaleId) )
        return NULL;

    return &d_data->scaleData[scaleId].scaleDiv;
}

QwtScaleMap QwtRadialPlot::canvasMap(int scaleId) const
{
    if ( !isValidScale(scaleId) )
        return QwtScaleMap();

    QwtScaleMap map;
    map.setTransformation(scaleEngine(scaleId)->transformation());

    const QwtScaleDiv *sd = scaleDiv(scaleId);
    map.setScaleInterval(sd->lBound(), sd->hBound());

    if ( isAngleScale(scaleId) )
        map.setPaintInterval(0, 5760); // 16 * 360, see QPainter
    else
        map.setPaintInterval(0, canvasRect().width() / 2);

    return map;
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
    if ( scaleDist > 0 )
        scaleDist++; // margin

    const QRect rect = boundingRect();
    return QRect(rect.x() + scaleDist, rect.y() + scaleDist,
        rect.width() - 2 * scaleDist, rect.height() - 2 * scaleDist);
}

QSize QwtRadialPlot::sizeHint() const
{
    const int extent = scaleExtent();
    if ( extent > 0 )
    {
        const int d = 6 * (extent + 1);
        return QSize( d, d );
    }

    return QSize(600, 600); // something
}

QSize QwtRadialPlot::minimumSizeHint() const
{
    const int extent = scaleExtent();
    if ( extent > 0 )
    {
        const int d = 3 * (extent + 1);
        return QSize( d, d );
    }

    return QSize(0, 0); // something
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

        for ( int scaleId = 0; scaleId < ScaleCount; scaleId++ )
        {
            if ( isAngleScaleVisible(scaleId) )
            {
                painter.save();
                drawAngleScale(&painter, scaleId, boundingRect());
                painter.restore();
            }
        }
    }
}

void QwtRadialPlot::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    if ( shape() == QRegion::Ellipse )
        updateMask();
}

void QwtRadialPlot::initPlot()
{
#if QT_VERSION < 0x040000
    setWFlags(Qt::WNoAutoErase);
#endif

    d_data = new PrivateData;

    d_data->canvasBrush = QBrush(Qt::white);

    d_data->autoReplot = false;
    d_data->shape = QRegion::Rectangle;
    
    for ( int scaleId = 0; scaleId < ScaleCount; scaleId++ )
    {
        ScaleData &scaleData = d_data->scaleData[scaleId];
        
        if ( isAngleScale(scaleId) )
        {
            scaleData.origin = 270.0;
            scaleData.scaleDraw = new QwtRoundScaleDraw();
            scaleData.isVisible = (scaleId == AngleScale1);

            scaleData.minValue = 0.0;
            scaleData.maxValue = 360.0;
            scaleData.stepSize = 15.0;
        }
        else
        {
            scaleData.origin = 0.0;
            scaleData.scaleDraw = NULL;
            scaleData.isVisible = false;

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

void QwtRadialPlot::updateMask()
{
    if ( shape() == QRegion::Ellipse )
        setMask( QRegion(boundingRect(), shape()) );
    else
        clearMask();
}

void QwtRadialPlot::drawCanvas(QPainter *painter, const QRect &rect) const
{
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(d_data->canvasBrush);

    setAntialiasing(painter, true);

    const QRect r(rect.x() - 1, rect.y() - 1, rect.width(), rect.height());
    painter->drawEllipse(r);
    painter->restore();

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
                map[item->distanceAxis()], map[item->angleAxis()],
                canvasRect);

            painter->restore();
        }
    }
}

void QwtRadialPlot::drawAngleScale(QPainter *painter, 
    int scaleId, const QRect &rect) const
{
    QwtRoundScaleDraw *scaleDraw = (QwtRoundScaleDraw *)angleScaleDraw(scaleId);
    if ( scaleDraw == NULL )
        return;

    const QPoint center = rect.center();
    int radius = rect.width() / 2;
    radius -= scaleDraw->extent(QPen(), font()) - 1;

    painter->setFont(font());
    scaleDraw->setRadius(radius);
    scaleDraw->moveCenter(center);

    const double o = origin(scaleId);
    scaleDraw->setAngleRange(o - 270.0, o + 90.0);

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

void QwtRadialPlot::updateScale(int scaleId)
{
    if ( !isValidScale(scaleId) )
        return;

    ScaleData &d = d_data->scaleData[scaleId];
    if ( !d.scaleDiv.isValid() )
    {
        d.scaleDiv = d.scaleEngine->divideScale(
            d.minValue, d.maxValue,
            d.maxMajor, d.maxMinor, d.stepSize);
    }

    if ( d.scaleDraw )
    {
        d.scaleDraw->setTransformation(d.scaleEngine->transformation());
        d.scaleDraw->setScaleDiv(d.scaleDiv);
    }
}

void QwtRadialPlot::polish()
{
    replot();

#if QT_VERSION < 0x040000
    QWidget::polish();
#endif
}

bool QwtRadialPlot::isValidScale(int scaleId)
{
    return ( scaleId >= 0 && scaleId < ScaleCount );
}

bool QwtRadialPlot::isAngleScale(int scaleId)
{
    return ( scaleId >= AngleScale1 && scaleId <= AngleScale4 );
}

bool QwtRadialPlot::isDistanceScale(int scaleId)
{
    return ( scaleId >= DistanceScale1 && scaleId <= DistanceScale4 );
}

int QwtRadialPlot::scaleExtent() const
{
    for ( int scaleId = 0; scaleId < ScaleCount; scaleId++ )
    {
        if ( isAngleScaleVisible(scaleId) )
        {
            const QwtRoundScaleDraw* scaleDraw = angleScaleDraw(scaleId);
            if ( scaleDraw )
                return scaleDraw->extent(QPen(), font());
        }
    }
    return 0;
}
