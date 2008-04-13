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
#include <qpaintengine.h>
#endif
#include <qpainter.h>
#include <qevent.h>
#include <qlayout.h>
#include "qwt_painter.h"
#include "qwt_math.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_div.h"
#include "qwt_text_label.h"
#include "qwt_round_scale_draw.h"
#include "qwt_polar_canvas.h"
#include "qwt_legend.h"
#include "qwt_dyngrid_layout.h"
#include "qwt_polar_plot.h"

static inline double distance(
    const QwtDoublePoint &p1, const QwtDoublePoint &p2)
{
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    return ::sqrt(dx * dx + dy * dy);
}

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

    QwtPolarPoint zoomPos;
    double zoomFactor;

    ScaleData scaleData[QwtPolar::ScaleCount];
#if QT_VERSION < 0x040000
    QGuardedPtr<QwtTextLabel> titleLabel;
    QGuardedPtr<QwtPolarCanvas> canvas;
    QGuardedPtr<QwtLegend> legend;
    QGuardedPtr<QWidget> spacer;
#else
    QPointer<QwtTextLabel> titleLabel;
    QPointer<QwtPolarCanvas> canvas;
    QPointer<QwtLegend> legend;
    QPointer<QWidget> spacer;
#endif
    QwtPolarPlot::LegendPosition legendPosition;
};

/*!
  Constructor
  \param parent Parent widget
 */
QwtPolarPlot::QwtPolarPlot(QWidget *parent):
    QFrame(parent)
{
    initPlot(QwtText());
}

/*!
  Constructor 
  \param title Title text
  \param parent Parent widget
 */
QwtPolarPlot::QwtPolarPlot(const QwtText &title, QWidget *parent):
    QFrame(parent)
{
    initPlot(title);
}

//! Destructor
QwtPolarPlot::~QwtPolarPlot()
{
    delete d_data;
}

/*!
  Change the plot's title
  \param title New title
*/
void QwtPolarPlot::setTitle(const QString &title)
{
    if ( title != d_data->titleLabel->text().text() )
    {
        d_data->titleLabel->setText(title);
        if ( !title.isEmpty() )
            d_data->titleLabel->show();
        else
            d_data->titleLabel->hide();
    }
}

/*!
  Change the plot's title
  \param title New title
*/
void QwtPolarPlot::setTitle(const QwtText &title)
{
    if ( title != d_data->titleLabel->text() )
    {
        d_data->titleLabel->setText(title);
        if ( !title.isEmpty() )
            d_data->titleLabel->show();
        else
            d_data->titleLabel->hide();
    }
}

//! \return the plot's title
QwtText QwtPolarPlot::title() const
{
    return d_data->titleLabel->text();
}

//! \return the plot's title
QwtTextLabel *QwtPolarPlot::titleLabel()
{
    return d_data->titleLabel;
}

//! \return the plot's titel label.
const QwtTextLabel *QwtPolarPlot::titleLabel() const
{
    return d_data->titleLabel;
}

/*!
  \brief Insert a legend

  If the position legend is \c QwtPlot::LeftLegend or \c QwtPlot::RightLegend
  the legend will be organized in one column from top to down.
  Otherwise the legend items will be placed in a table
  with a best fit number of columns from left to right.

  If pos != QwtPolarPlot::ExternalLegend the plot widget will become
  parent of the legend. It will be deleted when the plot is deleted,
  or another legend is set with insertLegend().

  \param legend Legend
  \param pos The legend's position. For top/left position the number
             of colums will be limited to 1, otherwise it will be set to
             unlimited.

  \sa legend()
*/
void QwtPolarPlot::insertLegend(QwtLegend *legend,
    QwtPolarPlot::LegendPosition pos)
{
    d_data->legendPosition = pos;
    if ( legend != d_data->legend )
    {
        if ( d_data->legend && d_data->legend->parent() == this )
            delete d_data->legend;

        d_data->legend = legend;

        if ( d_data->legend )
        {
            if ( pos != ExternalLegend )
            {
                if ( d_data->legend->parent() != this )
                {
#if QT_VERSION < 0x040000
                    d_data->legend->reparent(this, QPoint(0, 0));
#else
                    d_data->legend->setParent(this);
#endif
                }
            }

            const QwtPolarItemList& itmList = itemList();
            for ( QwtPolarItemIterator it = itmList.begin();
                it != itmList.end(); ++it )
            {
                (*it)->updateLegend(d_data->legend);
            }

            QLayout *l = d_data->legend->contentsWidget()->layout();
            if ( l && l->inherits("QwtDynGridLayout") )
            {
                QwtDynGridLayout *tl = (QwtDynGridLayout *)l;
                switch(d_data->legendPosition)
                {
                    case LeftLegend:
                    case RightLegend:
                        tl->setMaxCols(1); // 1 column: align vertical
                        break;
                    case TopLegend:
                    case BottomLegend:
                        tl->setMaxCols(0); // unlimited
                        break;
                    case ExternalLegend:
                        break;
                }
            }

        }
    }
    updateLayout();
}

/*!
  \return the plot's legend
  \sa insertLegend()
*/
QwtLegend *QwtPolarPlot::legend()
{
    return d_data->legend;
}

/*!
  \return the plot's legend
  \sa insertLegend()
*/
const QwtLegend *QwtPolarPlot::legend() const
{
    return d_data->legend;
}

/*!
  Called internally when the legend has been clicked on.
  Emits a legendClicked() signal.
*/
void QwtPolarPlot::legendItemClicked()
{
    if ( d_data->legend && sender()->isWidgetType() )
    {
        QwtPolarItem *plotItem = (QwtPolarItem*)d_data->legend->find((QWidget *)sender());
        if ( plotItem )
            emit legendClicked(plotItem);
    }
}

/*!
  Called internally when the legend has been checked
  Emits a legendClicked() signal.
*/
void QwtPolarPlot::legendItemChecked(bool on)
{
    if ( d_data->legend && sender()->isWidgetType() )
    {
        QwtPolarItem *plotItem = (QwtPolarItem*)d_data->legend->find((QWidget *)sender());
        if ( plotItem )
            emit legendChecked(plotItem, on);
    }
}

/*!
   \brief Set the background of the plot area

   The plot area is the circle around the pole. It's radius
   is defined by the radial scale. 
   
   \param brush Background Brush
   \sa plotBackground(), plotArea()
*/
void QwtPolarPlot::setPlotBackground(const QBrush &brush)
{
    if ( brush != d_data->canvasBrush )
    {
        d_data->canvasBrush = brush;
        autoRefresh();
    }
}

/*!
   \return plot background brush
   \sa plotBackground(), plotArea()
*/
const QBrush &QwtPolarPlot::plotBackground() const
{
    return d_data->canvasBrush;
}

/*!
  \brief Set or reset the autoReplot option

  If the autoReplot option is set, the plot will be
  updated implicitly by manipulating member functions.
  Since this may be time-consuming, it is recommended
  to leave this option switched off and call replot()
  explicitly if necessary.

  The autoReplot option is set to false by default, which
  means that the user has to call replot() in order to make
  changes visible.
  \param tf \c true or \c false. Defaults to \c true.
  \sa replot()
*/
void QwtPolarPlot::setAutoReplot(bool enable)
{
    d_data->autoReplot = enable;
}

//! \return true if the autoReplot option is set.
bool QwtPolarPlot::autoReplot() const
{
    return d_data->autoReplot;
}

/*!
  Set the maximum number of major scale intervals for a specified scale

  \param scaleId Scale index
  \param maxMajor maximum number of major steps
  \sa scaleMaxMajor()
*/
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

/*!
  \return the maximum number of minor ticks for a specified axis
  \param scaleId Scale index
  \sa setScaleMaxMinor()
*/  
int QwtPolarPlot::scaleMaxMinor(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return 0;

    return d_data->scaleData[scaleId].maxMinor;
}

/*!
  Set the maximum number of major scale intervals for a specified scale

  \param scaleId Scale index
  \param maxMajor maximum number of major steps
  \sa scaleMaxMajor()
*/
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

/*!
  \return the maximum number of major ticks for a specified axis
  \param scaleId Scale index

  \sa setScaleMaxMajor()
*/
int QwtPolarPlot::scaleMaxMajor(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return 0;

    return d_data->scaleData[scaleId].maxMajor;
}

/*!
   Change the scale engine for an axis
    
  \param scaleId Svale index
  \param scaleEngine Scale engine

  \sa axisScaleEngine()
*/
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

//! \return Scale engine for a specific scale
QwtScaleEngine *QwtPolarPlot::scaleEngine(int scaleId)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return NULL;

    return d_data->scaleData[scaleId].scaleEngine;
}

//! \return Scale engine for a specific scale
const QwtScaleEngine *QwtPolarPlot::scaleEngine(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return NULL;

    return d_data->scaleData[scaleId].scaleEngine;
}

/*!
  \brief Disable autoscaling and specify a fixed scale for a selected scale.
  \param scaleId Scale index
  \param min
  \param max minimum and maximum of the scale
  \param stepSize Major step size. If <code>step == 0</code>, the step size is
            calculated automatically using the maxMajor setting.
  \sa setScaleMaxMajor(), setAutoScale()
*/
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

/*!
  \brief Disable autoscaling and specify a fixed scale for a selected scale.
  \param scaleId Scale index
  \param scaleDiv Scale division
  \sa setScale(), setAutoScale()
*/
void QwtPolarPlot::setScaleDiv(int scaleId, const QwtScaleDiv &scaleDiv)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    ScaleData &scaleData = d_data->scaleData[scaleId];

    scaleData.scaleDiv = scaleDiv;
    scaleData.doAutoScale = false;

    autoRefresh();
}

/*!
  \brief Return the scale division of a specified scale
  
  scaleDiv(scaleId)->lBound(), scaleDiv(scaleId)->hBound()
  are the current limits of the scale.
  
  \param scaleId Scale index
  \return Scale division 

  \sa QwtScaleDiv, setScaleDiv(), setScale()
*/
const QwtScaleDiv *QwtPolarPlot::scaleDiv(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return NULL;

    return &d_data->scaleData[scaleId].scaleDiv;
}

/*!
  \brief Return the scale division of a specified scale
  
  scaleDiv(scaleId)->lBound(), scaleDiv(scaleId)->hBound()
  are the current limits of the scale.
  
  \param scaleId Scale index
  \return Scale division 

  \sa QwtScaleDiv, setScaleDiv(), setScale()
*/
QwtScaleDiv *QwtPolarPlot::scaleDiv(int scaleId)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return NULL;

    return &d_data->scaleData[scaleId].scaleDiv;
}

/*!
   \brief Translate and in/decrease the zoom factor

   In zoom mode the zoom position is in the center of the 
   canvas. The radius of the circle depends on the size of the plot canvas,
   that is devided by the zoom factor. Thus a factor < 1.0 zoom in.

   Setting an invalid zoom position disables zooming.

   \param zoomPos Center of the translation
   \param zoomFactor Zoom factor

   \sa unzoom(), zoomPos(), zoomFactor()
*/
void QwtPolarPlot::zoom(const QwtPolarPoint &zoomPos, double zoomFactor)
{
    zoomFactor = qwtAbs(zoomFactor);
    if ( zoomPos != d_data->zoomPos || 
        zoomFactor != d_data->zoomFactor )
    {
        d_data->zoomPos = zoomPos;
        d_data->zoomFactor = zoomFactor;
        autoRefresh();
    }
}

/*!
   Unzoom the plot
   \sa zoom()
*/
void QwtPolarPlot::unzoom()
{
    if ( d_data->zoomFactor != 1.0 || d_data->zoomPos.isValid() )
    {
        d_data->zoomFactor = 1.0;
        d_data->zoomPos = QwtPolarPoint();
        autoRefresh();
    }
}

/*!
   \return Zoom position
   \sa zoom(), zoomFactor()
*/
QwtPolarPoint QwtPolarPlot::zoomPos() const
{
    return d_data->zoomPos;
}

/*!
   \return Zoom factor
   \sa zoom(), zoomPos()
*/
double QwtPolarPlot::zoomFactor() const
{
    return d_data->zoomFactor;
}

/*!
  Build a scale map

  The azimuth map translates between the scale values and angles from
  [0.0, 2 * PI[. The radial map translates scale values into the distance
  from the pole.

  \param scaleId Scale index
  \return Map for the scale on the canvas. With this map pixel coordinates can
          translated to plot coordinates and vice versa.

  \sa QwtScaleMap, transform(), invTransform()

*/
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

//!  Adds handling of polish requests
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

    d_data->spacer = new QWidget(this),

    d_data->titleLabel = new QwtTextLabel(text, this);
    d_data->titleLabel->setFont(QFont(fontInfo().family(), 14, QFont::Bold));
    if ( !text.isEmpty() )
        d_data->titleLabel->show();
    else
        d_data->titleLabel->hide();

    d_data->canvas = new QwtPolarCanvas(this);

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
    }
    d_data->zoomFactor = 1.0;
    d_data->legendPosition = QwtPolarPlot::RightLegend;

    setSizePolicy(QSizePolicy::MinimumExpanding,
        QSizePolicy::MinimumExpanding);

    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
        updateScale(scaleId);
}

//! Replots the plot if QwtPlot::autoReplot() is \c true.
void QwtPolarPlot::autoRefresh()
{
    if (d_data->autoReplot)
        replot();
}

//! Rebuild the layout
void QwtPolarPlot::updateLayout()
{
    delete layout();

    QwtPolarPlot::LegendPosition pos = d_data->legendPosition;
    if ( !d_data->legend )
        pos = QwtPolarPlot::ExternalLegend;
    
    switch(pos)
    {
        case QwtPolarPlot::LeftLegend:
        {
            QGridLayout *l = new QGridLayout(this);
            l->setSpacing(0);
            l->setMargin(0);
            l->setRowStretch(1, 10);
#if QT_VERSION < 0x040000
            l->setColStretch(1, 10);
#else
            l->setColumnStretch(1, 10);
#endif
            l->addWidget(d_data->spacer, 0, 0);
            l->addWidget(d_data->legend, 0, 1);
            l->addWidget(d_data->titleLabel, 0, 1);
            l->addWidget(d_data->canvas, 1, 1);
            d_data->spacer->show();
            break;
        }
        case QwtPolarPlot::RightLegend:
        {
            QGridLayout *l = new QGridLayout(this);
            l->setSpacing(0);
            l->setMargin(0);
            l->setRowStretch(1, 10);
#if QT_VERSION < 0x040000
            l->setColStretch(0, 10);
#else
            l->setColumnStretch(0, 10);
#endif
            l->addWidget(d_data->titleLabel, 0, 0);
            l->addWidget(d_data->canvas, 1, 0);
            l->addWidget(d_data->spacer, 0, 1);
            l->addWidget(d_data->legend, 1, 1);
            d_data->spacer->show();
            break;
        }
        case QwtPolarPlot::BottomLegend:
        {
            QVBoxLayout *l = new QVBoxLayout(this);
            l->setSpacing(0);
            l->setMargin(0);
            l->addWidget(d_data->titleLabel);
            l->addWidget(d_data->canvas, 10);
            l->addWidget(d_data->legend);
            d_data->spacer->hide();
            break;
        }
        case QwtPolarPlot::TopLegend:
        {
            QVBoxLayout *l = new QVBoxLayout(this);
            l->setSpacing(0);
            l->setMargin(0);
            l->addWidget(d_data->legend);
            l->addWidget(d_data->titleLabel);
            l->addWidget(d_data->canvas, 10);
            d_data->spacer->hide();
            break;
        }
        case QwtPolarPlot::ExternalLegend:
        {
            QVBoxLayout *l = new QVBoxLayout(this);
            l->setSpacing(0);
            l->setMargin(0);
            l->addWidget(d_data->titleLabel);
            l->addWidget(d_data->canvas, 10);
            d_data->spacer->hide();
            break;
        }
    }

    if ( d_data->legend )
    {
        if ( d_data->legend->itemCount() > 0 )
            d_data->legend->show();
        else
            d_data->legend->hide();
    }

    layout()->activate();
}

/*!
  \brief Redraw the plot
  
  If the autoReplot option is not set (which is the default)
  or if any curves are attached to raw data, the plot has to
  be refreshed explicitly in order to make changes visible.

  \sa setAutoReplot()
  \warning Calls canvas()->repaint, take care of infinite recursions
*/  
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

//!  \return the plot's canvas
QwtPolarCanvas *QwtPolarPlot::canvas()
{
    return d_data->canvas;
}

//!  \return the plot's canvas
const QwtPolarCanvas *QwtPolarPlot::canvas() const
{
    return d_data->canvas;
}

/*!
  Redraw the canvas.
  \param painter Painter used for drawing
  \param canvasRect Contents rect of the canvas
*/
void QwtPolarPlot::drawCanvas(QPainter *painter, 
    const QwtDoubleRect &canvasRect) const
{
    const QwtDoubleRect pr = plotRect();
    if ( d_data->canvasBrush.style() != Qt::NoBrush )
    {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(d_data->canvasBrush);
#if QT_VERSION < 0x040000
        painter->drawEllipse(pr.toRect());
#else
        painter->drawEllipse(pr);
#endif
        painter->restore();
    }

    drawItems(painter, 
        scaleMap(QwtPolar::Azimuth), scaleMap(QwtPolar::Radius),
        pr.center(), pr.width() / 2.0, canvasRect);
}

/*!
  Redraw the canvas items.

  \param painter Painter used for drawing
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param radius Radius of the complete plot area in painter coordinates
  \param canvasRect Contents rect of the canvas in painter coordinates
*/
void QwtPolarPlot::drawItems(QPainter *painter,
        const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
        const QwtDoublePoint &pole, double radius,
        const QwtDoubleRect &canvasRect) const
{
    const QwtDoubleRect pr = plotRect();

    const QwtPolarItemList& itmList = itemList();
    for ( QwtPolarItemIterator it = itmList.begin();
        it != itmList.end(); ++it )
    {
        QwtPolarItem *item = *it;
        if ( item && item->isVisible() )
        {
            painter->save();
            
            const int margin = item->marginHint();
            const QwtDoubleRect clipRect(pr.x() - margin, pr.y() - margin,
                pr.width() + 2 * margin, pr.height() + 2 * margin);

            if ( !clipRect.contains(canvasRect) )
            {
                QRegion clipRegion(clipRect.toRect(), QRegion::Ellipse);
                painter->setClipRegion(clipRegion);
            }

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

//! Rebuild the scale 
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

    const QwtDoubleInterval interval = visibleInterval();

    const QwtPolarItemList& itmList = itemList();
    for ( QwtPolarItemIterator it = itmList.begin(); 
        it != itmList.end(); ++it )
    {
        QwtPolarItem *item = *it;
        item->updateScaleDiv( *scaleDiv(QwtPolar::Azimuth), 
            *scaleDiv(QwtPolar::Radius), interval);
    }
}

//! Polish
void QwtPolarPlot::polish()
{
    updateLayout();
    replot();

#if QT_VERSION < 0x040000
    QWidget::polish();
#endif
}

/*! 
   \return Maximum of all item margin hints.
   \sa QwtPolarItem::marhinHint()
*/
int QwtPolarPlot::plotMarginHint() const
{
    int margin = 0;
    const QwtPolarItemList& itmList = itemList();
    for ( QwtPolarItemIterator it = itmList.begin();
        it != itmList.end(); ++it ) 
    {
        QwtPolarItem *item = *it;
        if ( item && item->isVisible() )
        {
            const int hint = item->marginHint();
            if ( hint > margin )
                margin = hint;
        }
    }
    return margin;
}

/*!
   Calculate the bounding rect of the plot area
   The plot area depends on the size of the canvas
   and the zoom parameters.
*/
QwtDoubleRect QwtPolarPlot::plotRect() const
{
    const QwtScaleDiv *sd = scaleDiv(QwtPolar::Radius);
    const QwtScaleEngine *se = scaleEngine(QwtPolar::Radius);

    const int margin = plotMarginHint();
    const QRect cr = canvas()->contentsRect();
    const int radius = qwtMin(cr.width(), cr.height()) / 2 - margin;

    QwtScaleMap map;
    map.setTransformation(se->transformation());
    map.setPaintXInterval(0.0, radius / d_data->zoomFactor);
    map.setScaleInterval(sd->lBound(), sd->hBound());

    double v = map.s1();
    if ( map.s1() <= map.s2() )
        v += d_data->zoomPos.radius();
    else
        v -= d_data->zoomPos.radius();
    v = map.xTransform(v);

    const QwtDoublePoint off =
        QwtPolarPoint(v, d_data->zoomPos.azimuth()).toPoint();

    QwtDoublePoint center(cr.center().x(), cr.top() + margin + radius);
    center -= QwtDoublePoint(off.x(), -off.y());

    QwtDoubleRect rect(0, 0, 2 * map.p2(), 2 * map.p2());
    rect.moveCenter(center);

    return rect;
}

/*!
   Calculate the bounding interval of the radial scale that is
   visible on the canvas. 
*/
QwtDoubleInterval QwtPolarPlot::visibleInterval() const
{
    const QwtScaleDiv *sd = scaleDiv(QwtPolar::Radius);

    const QwtDoubleRect pRect = plotRect();
    const QwtDoubleRect cRect = canvas()->contentsRect();
    if ( cRect.contains(pRect.toRect()) || !cRect.intersects(pRect) )
    {
        return QwtDoubleInterval(sd->lBound(), sd->hBound());
    }

    const QwtDoublePoint pole = pRect.center();
    const QwtDoubleRect scaleRect = pRect & cRect;
    
    const QwtScaleMap map = scaleMap(QwtPolar::Radius);

    double dmin = 0.0; 
    double dmax = 0.0;
    if ( scaleRect.contains(pole) )
    {
        dmin = 0.0;

        QwtDoublePoint corners[4];
        corners[0] = scaleRect.bottomRight();
        corners[1] = scaleRect.topRight();
        corners[2] = scaleRect.topLeft();
        corners[3] = scaleRect.bottomLeft();

        dmax = 0.0;
        for ( int i = 0; i < 4; i++ )
        {
            const double dist = distance(pole, corners[i]);
            if ( dist > dmax )
                dmax = dist;
        }
    }
    else
    {
        if ( pole.x() < scaleRect.left() )
        {
            if ( pole.y() < scaleRect.top() )
            {
                dmin = distance(pole, scaleRect.topLeft());
                dmax = distance(pole, scaleRect.bottomRight());
            }
            else if ( pole.y() > scaleRect.bottom() )
            {
                dmin = distance(pole, scaleRect.bottomLeft());
                dmax = distance(pole, scaleRect.topRight());
            }
            else
            {
                dmin = scaleRect.left() - pole.x();
                dmax = qwtMax(distance(pole, scaleRect.bottomRight()),
                    distance(pole, scaleRect.topRight()) );
            }
        } 
        else if ( pole.x() > scaleRect.right() )
        {
            if ( pole.y() < scaleRect.top() )
            {
                dmin = distance(pole, scaleRect.topRight());
                dmax = distance(pole, scaleRect.bottomLeft());
            }
            else if ( pole.y() > scaleRect.bottom() )
            {
                dmin = distance(pole, scaleRect.bottomRight());
                dmax = distance(pole, scaleRect.topLeft());
            }
            else
            {
                dmin = pole.x() - scaleRect.right();
                dmax = qwtMax(distance(pole, scaleRect.bottomLeft()),
                    distance(pole, scaleRect.topLeft()) );
            }
        }
        else if ( pole.y() < scaleRect.top() )
        {
            dmin = scaleRect.top() - pole.y();
            dmax = qwtMax(distance(pole, scaleRect.bottomLeft()),
                distance(pole, scaleRect.bottomRight()));
        }
        else if ( pole.y() > scaleRect.bottom() )
        {
            dmin = pole.y() - scaleRect.bottom();
            dmax = qwtMax(distance(pole, scaleRect.topLeft()),
                distance(pole, scaleRect.topRight()));
        }
    }

    const double radius = pRect.width() / 2.0;
    if ( dmax > radius )
        dmax = radius;

    QwtDoubleInterval interval;
    interval.setMinValue(map.invTransform(dmin));
    interval.setMaxValue(map.invTransform(dmax));
    
    return interval;
}
