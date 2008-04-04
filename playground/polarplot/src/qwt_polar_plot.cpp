/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GPL License, Version 2.0
 *****************************************************************************/

#include <qglobal.h>
#if QT_VERSION < 0x040000
#include <qguardedptr.h>
#else
#include <qpointer.h>
#endif
#include <qpainter.h>
#include <qevent.h>
#include <qlayout.h>
#include <qpaintengine.h>
#include "qwt_painter.h"
#include "qwt_math.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_div.h"
#include "qwt_text_label.h"
#include "qwt_round_scale_draw.h"
#include "qwt_polar_rect.h"
#include "qwt_polar_canvas.h"
#include "qwt_legend.h"
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
    QwtPolarRect zoomRect;

    ScaleData scaleData[QwtPolar::ScaleCount];
#if QT_VERSION < 0x040000
    QGuardedPtr<QwtTextLabel> titleLabel;
    QGuardedPtr<QwtPolarCanvas> canvas;
    QGuardedPtr<QwtLegend> legend;
#else
    QPointer<QwtTextLabel> titleLabel;
    QPointer<QwtPolarCanvas> canvas;
    QPointer<QwtLegend> legend;
#endif
};

QwtPolarPlot::QwtPolarPlot( QWidget *parent):
    QFrame(parent)
{
    initPlot(QwtText());
}

QwtPolarPlot::QwtPolarPlot(const QwtText &title, QWidget *parent):
    QFrame(parent)
{
    initPlot(title);
}


QwtPolarPlot::~QwtPolarPlot()
{
    delete d_data;
}

void QwtPolarPlot::setTitle(const QString &title)
{
    if ( title != d_data->titleLabel->text().text() )
    {
        d_data->titleLabel->setText(title);
        d_data->titleLabel->setVisible(!title.isEmpty());
    }
}

void QwtPolarPlot::setTitle(const QwtText &title)
{
    if ( title != d_data->titleLabel->text() )
    {
        d_data->titleLabel->setText(title);
        d_data->titleLabel->setVisible(!title.isEmpty());
    }
}

QwtText QwtPolarPlot::title() const
{
    return d_data->titleLabel->text();
}

QwtTextLabel *QwtPolarPlot::titleLabel()
{
    return d_data->titleLabel;
}

const QwtTextLabel *QwtPolarPlot::titleLabel() const
{
    return d_data->titleLabel;
}

void QwtPolarPlot::setCanvasBackground(const QBrush &brush)
{
    if ( brush != d_data->canvasBrush )
    {
        d_data->canvasBrush = brush;
        autoRefresh();
    }
}

const QBrush &QwtPolarPlot::canvasBackground() const
{
    return d_data->canvasBrush;
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
    setZoomRect(QwtPolarRect());
}

void QwtPolarPlot::setZoomRect(const QwtPolarRect &rect)
{
    QwtPolarRect zoomRect = rect.normalized();
    if ( zoomRect != d_data->zoomRect )
    {
        d_data->zoomRect = zoomRect;
        autoRefresh();
    }
}

QwtPolarRect QwtPolarPlot::zoomRect() const
{
    return d_data->zoomRect;
}

QwtPolarRect QwtPolarPlot::scaleRect() const
{
    const double d = 2 * qwtAbs(scaleDiv(QwtPolar::Radius)->range());
    return QwtPolarRect(0.0, 0.0, d, d);
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
        const double w = plotRect().width(); 
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

void QwtPolarPlot::initPlot(const QwtText &title)
{
#if QT_VERSION < 0x040000
    setWFlags(Qt::WNoAutoErase);
#endif

    d_data = new PrivateData;

    QwtText text(title);
    int flags = Qt::AlignCenter;
#if QT_VERSION < 0x040000
    flags |= Qt::WordBreak | Qt::ExpandTabs;
#else
    flags |= Qt::TextWordWrap;
#endif
    text.setRenderFlags(flags);

    d_data->titleLabel = new QwtTextLabel(text, this);
    d_data->titleLabel->setFont(QFont(fontInfo().family(), 14, QFont::Bold));
    d_data->titleLabel->setVisible(!text.isEmpty());

    d_data->canvas = new QwtPolarCanvas(this);
#if 1
    d_data->canvas->setFrameStyle(QFrame::Panel | QFrame::Raised);
    d_data->canvas->setLineWidth(2);
#endif

    d_data->autoReplot = false;
    d_data->canvasBrush = QBrush(Qt::white);
    
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

void QwtPolarPlot::updateLayout()
{
    delete layout();

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setSpacing(0);
    l->setMargin(0);
    l->addWidget(d_data->titleLabel);
    l->addWidget(d_data->canvas, 10);
    l->activate();
}

void QwtPolarPlot::replot()
{
    bool doAutoReplot = autoReplot();
    setAutoReplot(false);

    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
        updateScale(scaleId);

    d_data->canvas->invalidatePaintCache();
    d_data->canvas->repaint();

    setAutoReplot(doAutoReplot);
}

QwtPolarCanvas *QwtPolarPlot::canvas()
{
    return d_data->canvas;
}

const QwtPolarCanvas *QwtPolarPlot::canvas() const
{
    return d_data->canvas;
}

void QwtPolarPlot::drawCanvas(QPainter *painter, 
    const QwtDoubleRect &canvasRect) const
{
    const QwtDoubleRect pr = plotRect();

    if ( d_data->canvasBrush.style() != Qt::NoBrush )
    {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(d_data->canvasBrush);
        painter->drawEllipse(pr);
        painter->restore();
    }

    drawItems(painter, 
        scaleMap(QwtPolar::Azimuth), scaleMap(QwtPolar::Radius),
        pr.center(), pr.width() / 2.0, canvasRect);
}

void QwtPolarPlot::drawItems(QPainter *painter,
        const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
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

            item->draw(painter, azimuthMap, radialMap, 
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
            *scaleDiv(QwtPolar::Azimuth), *scaleDiv(QwtPolar::Radius));
    }
}

void QwtPolarPlot::polish()
{
    updateLayout();
    replot();

#if QT_VERSION < 0x040000
    QWidget::polish();
#endif
}

int QwtPolarPlot::canvasMarginHint() const
{
    int margin = 0;
    const QwtPolarItemList& itmList = itemList();
    for ( QwtPolarItemIterator it = itmList.begin();
        it != itmList.end(); ++it ) 
    {
        QwtPolarItem *item = *it;
        if ( item && item->isVisible() )
        {
            const int hint = item->canvasMarginHint();
            if ( hint > margin )
                margin = hint;
        }
    }
    return margin;
}

QwtDoubleRect QwtPolarPlot::plotRect() const
{
    QwtDoubleRect zr = d_data->zoomRect.toRect();
    if ( zr.isEmpty() )
        zr = scaleRect().toRect();
    zr = zr.normalized();

    const int margin = canvasMarginHint();
    const QRect cr = canvas()->contentsRect();
    const int radius = qwtMin(cr.width(), cr.height()) / 2 - margin;

    const double ratio = 2 * radius / qwtMin(zr.width(), zr.height());

    const double px = cr.center().x() - radius - zr.x() * ratio;
    const double py = cr.top() + margin + 2 * radius + zr.y() * ratio;
    const double d = 2 * qwtAbs(scaleDiv(QwtPolar::Radius)->range()) * ratio;

    QwtDoubleRect rect(0.0, 0.0, d, d);
    rect.moveCenter(QwtDoublePoint(px, py));
    return rect;
}
