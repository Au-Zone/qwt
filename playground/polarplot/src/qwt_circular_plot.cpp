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
#include "qwt_circular_plot.h"

class QwtCircularPlot::ScaleData
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

    bool isEnabled;

    double minValue;
    double maxValue;
    double stepSize;

    int maxMajor;
    int maxMinor;

    QwtScaleDiv scaleDiv;
    QwtScaleEngine *scaleEngine;
    QwtRoundScaleDraw *scaleDraw;
};

class QwtCircularPlot::PrivateData
{
public:
    bool visibleBackground;
    double origin;

    bool autoReplot;
	ScaleData scaleData;
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

QwtCircularPlot::QwtCircularPlot( QWidget *parent):
    QWidget(parent)
{
    init();
}

#if QT_VERSION < 0x040000
QwtCircularPlot::QwtCircularPlot( QWidget *parent, const char *name):
    QWidget(parent, name)
{
    init();
}
#endif

QwtCircularPlot::~QwtCircularPlot()
{
    delete d_data;
}

void QwtCircularPlot::setCanvasBackground(const QColor &c)
{
    QPalette p = palette();

    for ( int i = 0; i < QPalette::NColorGroups; i++ )
    {
#if QT_VERSION < 0x040000
        p.setColor((QPalette::ColorGroup)i, QColorGroup::Foreground, c);
#else
        p.setColor((QPalette::ColorGroup)i, QPalette::Foreground, c);
#endif
    }

    setPalette(p);
}

const QColor& QwtCircularPlot::canvasBackground() const
{
#if QT_VERSION < 0x040000
    return palette().color(QPalette::Normal, QColorGroup::Foreground);
#else
    return palette().color(QPalette::Normal, QPalette::Foreground);
#endif
}

void QwtCircularPlot::setAutoReplot(bool enable)
{
    d_data->autoReplot = enable;
}

bool QwtCircularPlot::autoReplot() const
{
    return d_data->autoReplot;
}

bool QwtCircularPlot::hasVisibleBackground() const
{
    return d_data->visibleBackground;
}

void QwtCircularPlot::showBackground(bool enable)
{
    if ( enable != d_data->visibleBackground )
    {
        d_data->visibleBackground = enable;
        updateMask();
    }
}

void QwtCircularPlot::enableScale(bool enable)
{
    if ( d_data->scaleData.isEnabled != enable )
    {
        d_data->scaleData.isEnabled = enable;
        autoRefresh();
    }
}

bool QwtCircularPlot::isScaleEnabled() const
{
    return d_data->scaleData.isEnabled;
}

void QwtCircularPlot::setScaleMaxMinor(int maxMinor)
{
    if ( maxMinor < 0 )
        maxMinor = 0;
    if ( maxMinor > 100 )
        maxMinor = 100;

    ScaleData &d = d_data->scaleData;

    if ( maxMinor != d.maxMinor )
    {
        d.maxMinor = maxMinor;
        d.scaleDiv.invalidate();
        autoRefresh();
    }
}

int QwtCircularPlot::scaleMaxMinor() const
{
    return d_data->scaleData.maxMinor;
}

int QwtCircularPlot::scaleMaxMajor() const
{
    return d_data->scaleData.maxMajor;
}

void QwtCircularPlot::setScaleMaxMajor(int maxMajor)
{
    if ( maxMajor < 1 )
        maxMajor = 1;
    if ( maxMajor > 1000 )
        maxMajor = 10000;

    ScaleData &d = d_data->scaleData;
    if ( maxMajor != d.maxMinor )
    {
        d.maxMajor = maxMajor;
        d.scaleDiv.invalidate();
        autoRefresh();
    }

}

QwtScaleEngine *QwtCircularPlot::scaleEngine()
{
    return d_data->scaleData.scaleEngine;
}

const QwtScaleEngine *QwtCircularPlot::scaleEngine() const
{
    return d_data->scaleData.scaleEngine;
}

void QwtCircularPlot::setScaleEngine(QwtScaleEngine *scaleEngine)
{
    ScaleData &d = d_data->scaleData;

    if (scaleEngine == NULL || scaleEngine == d.scaleEngine )
        return;

    delete d.scaleEngine;
    d.scaleEngine = scaleEngine;

    d.scaleDiv.invalidate();

    autoRefresh();
}

void QwtCircularPlot::setScale(double min, double max, double stepSize)
{
    ScaleData &d = d_data->scaleData;

    d.scaleDiv.invalidate();

    d.minValue = min;
    d.maxValue = max;
    d.stepSize = stepSize;

    autoRefresh();
}

void QwtCircularPlot::setScaleDiv(const QwtScaleDiv &scaleDiv)
{
    d_data->scaleData.scaleDiv = scaleDiv;
    autoRefresh();
}


void QwtCircularPlot::setScaleDraw(QwtRoundScaleDraw *scaleDraw)
{
	if ( scaleDraw == NULL )
		return;

    ScaleData &d = d_data->scaleData;
    if ( scaleDraw != d.scaleDraw )
    {
        delete d.scaleDraw;
        d.scaleDraw = scaleDraw;
        autoRefresh();
    }
}

const QwtScaleDiv *QwtCircularPlot::scaleDiv() const
{
    return &d_data->scaleData.scaleDiv;
}

QwtScaleDiv *QwtCircularPlot::scaleDiv()
{
    return &d_data->scaleData.scaleDiv;
}

const QwtRoundScaleDraw *QwtCircularPlot::scaleDraw() const
{
    return d_data->scaleData.scaleDraw;
}

QwtRoundScaleDraw *QwtCircularPlot::scaleDraw()
{
    return d_data->scaleData.scaleDraw;
}

QwtScaleMap QwtCircularPlot::scaleMap() const
{
	QwtScaleMap map;
    map.setTransformation(scaleEngine()->transformation());

    const QwtScaleDiv *sd = scaleDiv();
    map.setScaleInterval(sd->lBound(), sd->hBound());

    map.setPaintInterval(0, 5760); // 16 * 360, see QPainter
	return map;
}

void QwtCircularPlot::setOrigin(double origin)
{
    if ( d_data->origin != origin )
    {
        d_data->origin = origin;
        autoRefresh();
    }
}

double QwtCircularPlot::origin() const
{
    return d_data->origin;
}

QRect QwtCircularPlot::boundingRect() const
{
    const int radius = qwtMin(width(), height()) / 2;

    QRect r(0, 0, 2 * radius, 2 * radius);
    r.moveCenter(rect().center());
    return r;
}

QRect QwtCircularPlot::canvasRect() const
{
#if QT_VERSION < 0x040000
    const QPen scalePen(colorGroup().text(), 0, Qt::NoPen);
#else
    const QPen scalePen(palette().text(), 0, Qt::NoPen);
#endif

    int scaleDist = 0;
    if ( d_data->scaleData.scaleDraw )
    {
        scaleDist = d_data->scaleData.scaleDraw->extent(scalePen, font());
        scaleDist++; // margin
    }

    const QRect rect = boundingRect();
    return QRect(rect.x() + scaleDist, rect.y() + scaleDist,
        rect.width() - 2 * scaleDist, rect.height() - 2 * scaleDist);
}

QSize QwtCircularPlot::sizeHint() const
{
    int sh = 0;
    if ( d_data->scaleData.scaleDraw )
        sh = d_data->scaleData.scaleDraw->extent( QPen(), font() );

    const int d = 6 * sh;
    return QSize( d, d );
}

QSize QwtCircularPlot::minimumSizeHint() const
{
    int sh = 0;
    if ( d_data->scaleData.scaleDraw )
        sh = d_data->scaleData.scaleDraw->extent( QPen(), font() );

    const int d = 3 * sh;
    return QSize( d, d );
}

void QwtCircularPlot::paintEvent(QPaintEvent *e)
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

       	painter.save();
		drawScale(&painter, boundingRect());
       	painter.restore();
	}
}

void QwtCircularPlot::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    if ( !hasVisibleBackground() )
        updateMask();
}

void QwtCircularPlot::init()
{
#if QT_VERSION < 0x040000
    setWFlags(Qt::WNoAutoErase);
#endif

    d_data = new PrivateData;

    d_data->autoReplot = false;
    d_data->visibleBackground = true;
    d_data->origin = 270.0;
    
    ScaleData &d = d_data->scaleData;
    
    d.minValue = 0.0;
    d.maxValue = 360.0;
    d.stepSize = 15.0;
    
    d.maxMinor = 5;
    d.maxMajor = 8;
    
    d.scaleEngine = new QwtLinearScaleEngine;

    d.scaleDiv.invalidate();
    d.scaleDraw = new QwtRoundScaleDraw();

	updateScale();

    setSizePolicy(QSizePolicy::MinimumExpanding,
        QSizePolicy::MinimumExpanding);
}

void QwtCircularPlot::autoRefresh()
{
    if (d_data->autoReplot)
        replot();
}

void QwtCircularPlot::replot()
{
	updateScale();
}

void QwtCircularPlot::updateMask()
{
    if ( d_data->visibleBackground )
        clearMask();
    else
        setMask(QRegion(boundingRect(), QRegion::Ellipse));
}

void QwtCircularPlot::drawCanvas(QPainter *painter, const QRect &rect) const
{
	painter->save();
	painter->setPen(Qt::NoPen);

#if QT_VERSION < 0x040000
	painter->setBrush(colorGroup().brush(QColorGroup::Foreground));
#else
	painter->setBrush(palette().brush(QPalette::Foreground));
#endif

	setAntialiasing(painter, true);
	painter->drawEllipse(rect);
	painter->restore();
}

void QwtCircularPlot::drawScale(QPainter *painter, const QRect &rect) const
{
	QwtRoundScaleDraw *scaleDraw = d_data->scaleData.scaleDraw;

    const QPoint center = rect.center();
    int radius = rect.width() / 2;
	radius -= scaleDraw->extent(QPen(), font()) - 1;

    painter->setFont(font());
    scaleDraw->setRadius(radius);
    scaleDraw->moveCenter(center);
    scaleDraw->setAngleRange(d_data->origin - 270.0, d_data->origin + 90.0);

#if QT_VERSION < 0x040000
    QColorGroup cg = colorGroup();

    const QColor textColor = cg.color(QColorGroup::Text);
    cg.setColor(QColorGroup::Foreground, textColor);
    painter->setPen(QPen(textColor, d_data->scaleDraw->penWidth()));

    scaleDraw->draw(painter, cg);
#else
    QPalette pal = palette();

    const QColor textColor = pal.color(QPalette::Text);
    pal.setColor(QPalette::Foreground, textColor); //ticks, backbone

    painter->setPen(QPen(textColor));
	setAntialiasing(painter, true);
    scaleDraw->draw(painter, pal);
#endif
}

void QwtCircularPlot::updateScale()
{
	ScaleData &d = d_data->scaleData;
    if ( !d.scaleDiv.isValid() )
    {
        d.scaleDiv = d.scaleEngine->divideScale(
            d.minValue, d.maxValue,
            d.maxMajor, d.maxMinor, d.stepSize);
    }

    d.scaleDraw->setTransformation(d.scaleEngine->transformation());
    d.scaleDraw->setScaleDiv(d.scaleDiv);
}
