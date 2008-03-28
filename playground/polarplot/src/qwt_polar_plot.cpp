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
#include "qwt_polar_plot.h"

class QwtPolarPlot::ScaleData
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
};

class QwtPolarPlot::PrivateData
{
public:
    QBrush canvasBrush;

    bool autoReplot;
    QwtDoubleRect zoomRect;

    ScaleData scaleData[QwtPolar::ScaleCount];
};

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

void QwtPolarPlot::setScaleMaxMinor(int scaleId, int maxMinor)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
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

int QwtPolarPlot::scaleMaxMinor(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return 0;

    return d_data->scaleData[scaleId].maxMinor;
}

void QwtPolarPlot::setScaleMaxMajor(int scaleId, int maxMajor)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
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

int QwtPolarPlot::scaleMaxMajor(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return 0;

    return d_data->scaleData[scaleId].maxMajor;
}

QwtScaleEngine *QwtPolarPlot::scaleEngine(int scaleId)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return NULL;

    return d_data->scaleData[scaleId].scaleEngine;
}

const QwtScaleEngine *QwtPolarPlot::scaleEngine(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return NULL;

    return d_data->scaleData[scaleId].scaleEngine;
}

void QwtPolarPlot::setScaleEngine(int scaleId, QwtScaleEngine *scaleEngine)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    ScaleData &scaleData = d_data->scaleData[scaleId];
    if (scaleEngine == NULL || scaleEngine == scaleData.scaleEngine )
        return;

    delete scaleData.scaleEngine;
    scaleData.scaleEngine = scaleEngine;

    scaleData.scaleDiv.invalidate();

    autoRefresh();
}

void QwtPolarPlot::setScale(int scaleId, 
    double min, double max, double stepSize)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    ScaleData &scaleData = d_data->scaleData[scaleId];

    scaleData.scaleDiv.invalidate();

    scaleData.minValue = min;
    scaleData.maxValue = max;
    scaleData.stepSize = stepSize;
    scaleData.doAutoScale = false;

    autoRefresh();
}

void QwtPolarPlot::setScaleDiv(int scaleId, const QwtScaleDiv &scaleDiv)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    ScaleData &scaleData = d_data->scaleData[scaleId];

    scaleData.scaleDiv = scaleDiv;
    scaleData.doAutoScale = false;

    autoRefresh();
}

const QwtScaleDiv *QwtPolarPlot::scaleDiv(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return NULL;

    return &d_data->scaleData[scaleId].scaleDiv;
}

QwtScaleDiv *QwtPolarPlot::scaleDiv(int scaleId)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return NULL;

    return &d_data->scaleData[scaleId].scaleDiv;
}

void QwtPolarPlot::unzoom()
{
    setZoomRect(QwtDoubleRect());
}

void QwtPolarPlot::setZoomRect(const QwtDoubleRect &rect)
{
    const QwtDoubleRect zoomRect = rect.normalized();
    if ( zoomRect != d_data->zoomRect )
    {
        d_data->zoomRect = zoomRect;
        autoRefresh();
    }
}

QwtDoubleRect QwtPolarPlot::zoomRect() const
{
    return d_data->zoomRect;
}

QwtScaleMap QwtPolarPlot::scaleMap(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return QwtScaleMap();

    QwtScaleMap map;
    map.setTransformation(scaleEngine(scaleId)->transformation());

    const QwtScaleDiv *sd = scaleDiv(scaleId);
    map.setScaleInterval(sd->lBound(), sd->hBound());

    if ( scaleId == QwtPolar::Azimuth)
    {
        map.setPaintXInterval(0.0, M_2PI); 
    }
    else
    {
        const double w = polarRect().width(); 
        map.setPaintXInterval(0.0, w / 2.0);
    }

    return map;
}

QSize QwtPolarPlot::sizeHint() const
{
    return QWidget::sizeHint();
}

QSize QwtPolarPlot::minimumSizeHint() const
{
    return QWidget::minimumSizeHint();
}

bool QwtPolarPlot::event(QEvent *e)
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

void QwtPolarPlot::paintEvent(QPaintEvent *e)
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
        drawCanvas(&painter);
        painter.restore();
    }
}

void QwtPolarPlot::initPlot()
{
#if QT_VERSION < 0x040000
    setWFlags(Qt::WNoAutoErase);
#endif

    d_data = new PrivateData;

    d_data->autoReplot = false;
    
    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
    {
        ScaleData &scaleData = d_data->scaleData[scaleId];
        
        if ( scaleId == QwtPolar::Azimuth )
        {
            scaleData.minValue = 0.0;
            scaleData.maxValue = 360.0;
            scaleData.stepSize = 30.0;
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

void QwtPolarPlot::autoRefresh()
{
    if (d_data->autoReplot)
        replot();
}

void QwtPolarPlot::replot()
{
    bool doAutoReplot = autoReplot();
    setAutoReplot(false);

    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
        updateScale(scaleId);

    setAutoReplot(doAutoReplot);

    repaint();
}

void QwtPolarPlot::drawCanvas(QPainter *painter) const
{
    const QwtDoubleRect pr = polarRect();

    drawItems(painter, 
        scaleMap(QwtPolar::Radius), scaleMap(QwtPolar::Azimuth),
        pr.center(), pr.width() / 2.0, 
        QwtDoubleRect(contentsRect()));
}

void QwtPolarPlot::drawItems(QPainter *painter,
        const QwtScaleMap &radialMap, const QwtScaleMap &azimuthMap,
        const QwtDoublePoint &pole, double radius,
        const QwtDoubleRect &canvasRect) const
{
    const QwtPolarItemList& itmList = itemList();
    for ( QwtPolarItemIterator it = itmList.begin();
        it != itmList.end(); ++it )
    {
        QwtPolarItem *item = *it;
        if ( item && item->isVisible() )
        {
            painter->save();

#if QT_VERSION >= 0x040000
            painter->setRenderHint(QPainter::Antialiasing,
                item->testRenderHint(QwtPolarItem::RenderAntialiased) );
#endif

            item->draw(painter, radialMap, azimuthMap, 
                pole, radius, canvasRect);

            painter->restore();
        }
    }
}

void QwtPolarPlot::updateScale(int scaleId)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    ScaleData &d = d_data->scaleData[scaleId];
    if ( !d.scaleDiv.isValid() )
    {
        d.scaleDiv = d.scaleEngine->divideScale(
            d.minValue, d.maxValue,
            d.maxMajor, d.maxMinor, d.stepSize);
    }

    const QwtPolarItemList& itmList = itemList();

    QwtPolarItemIterator it;

    for ( it = itmList.begin(); it != itmList.end(); ++it )
    {
        QwtPolarItem *item = *it;
        item->updateScaleDiv( 
            *scaleDiv(QwtPolar::Radius), *scaleDiv(QwtPolar::Azimuth));
    }
}

void QwtPolarPlot::polish()
{
    replot();

#if QT_VERSION < 0x040000
    QWidget::polish();
#endif
}

QwtDoubleRect QwtPolarPlot::polarRect() const
{
    const int radius = qwtMin(width(), height()) / 2;

    QRect r(0, 0, 2 * radius, 2 * radius);
    r.moveCenter(rect().center());

    const QwtPolarItemList& itmList = itemList();
    for ( QwtPolarItemIterator it = itmList.begin();
        it != itmList.end(); ++it )
    {
        QwtPolarItem *item = *it;
        if ( item && item->isVisible() )
        {
            const QRect hint = item->canvasLayoutHint(r);
            if ( hint.isValid() )
                r &= hint;
        }
    }
    return QwtDoubleRect(r);
}
