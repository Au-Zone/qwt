/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_rasteritem.h"
#include "qwt_legend.h"
#include "qwt_legend_item.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qpainter.h>
#include <qpaintengine.h>

static void transformMaps(const QTransform &tr,
	const QwtScaleMap &xMap, const QwtScaleMap &yMap,
	QwtScaleMap &xxMap, QwtScaleMap &yyMap)
{
	const QPointF p1 = tr.map( QPointF(xMap.p1(), yMap.p1()) );
	const QPointF p2 = tr.map( QPointF(xMap.p2(), yMap.p2()) );

    xxMap = xMap;
    xxMap.setPaintInterval(p1.x(), p2.x());

    yyMap = yMap;
    yyMap.setPaintInterval(p1.y(), p2.y());
}

class QwtPlotRasterItem::PrivateData
{
public:
    PrivateData():
        alpha(-1)
    {
        cache.policy = QwtPlotRasterItem::NoCache;
    }

    int alpha;

    struct ImageCache
    {
        QwtPlotRasterItem::CachePolicy policy;
        QRectF area;
        QSize size;
        QImage image;
    } cache;
};

static QImage toRgba(const QImage& image, int alpha)
{
    if ( alpha < 0 || alpha >= 255 )  
        return image;

    QImage alphaImage(image.size(), QImage::Format_ARGB32);

    const QRgb mask1 = qRgba(0, 0, 0, alpha);
    const QRgb mask2 = qRgba(255, 255, 255, 0);
    const QRgb mask3 = qRgba(0, 0, 0, 255);

    const int w = image.size().width();
    const int h = image.size().height();

    if ( image.depth() == 8 )
    {
        for ( int y = 0; y < h; y++ )
        {
            QRgb* alphaLine = (QRgb*)alphaImage.scanLine(y);
            const unsigned char *line = image.scanLine(y);

            for ( int x = 0; x < w; x++ )
                *alphaLine++ = (image.color(*line++) & mask2) | mask1;
        }
    }
    else if ( image.depth() == 32 )
    {
        for ( int y = 0; y < h; y++ )
        {
            QRgb* alphaLine = (QRgb*)alphaImage.scanLine(y);
            const QRgb* line = (const QRgb*) image.scanLine(y);

            for ( int x = 0; x < w; x++ )
            {
                const QRgb rgb = *line++;
                if ( rgb & mask3 ) // alpha != 0
                    *alphaLine++ = (rgb & mask2) | mask1;
                else
                    *alphaLine++ = rgb;
            }
        }
    }

    return alphaImage;
}

//! Constructor
QwtPlotRasterItem::QwtPlotRasterItem(const QString& title):
    QwtPlotItem(QwtText(title))
{
    init();
}

//! Constructor
QwtPlotRasterItem::QwtPlotRasterItem(const QwtText& title):
    QwtPlotItem(title)
{
    init();
}

//! Destructor
QwtPlotRasterItem::~QwtPlotRasterItem()
{
    delete d_data;
}

void QwtPlotRasterItem::init()
{
    d_data = new PrivateData();

    setItemAttribute(QwtPlotItem::AutoScale, true);
    setItemAttribute(QwtPlotItem::Legend, false);

    setZ(8.0);
}

/*!
   \brief Set an alpha value for the raster data

   Often a plot has several types of raster data organized in layers.
   ( f.e a geographical map, with weather statistics ).
   Using setAlpha() raster items can be stacked easily.

   The alpha value is a value [0, 255] to
   control the transparency of the image. 0 represents a fully 
   transparent color, while 255 represents a fully opaque color.
   
   \param alpha Alpha value

   - alpha >= 0\n
     All alpha values of the pixels returned by renderImage() will be set to 
     alpha, beside those with an alpha value of 0 (invalid pixels). 
   - alpha < 0
     The alpha values returned by renderImage() are not changed.

   The default alpha value is -1.

   \sa alpha()
*/
void QwtPlotRasterItem::setAlpha(int alpha)
{
    if ( alpha < 0 )
        alpha = -1;

    if ( alpha > 255 )
        alpha = 255;

    if ( alpha != d_data->alpha )
    {
        d_data->alpha = alpha;

        itemChanged();
    }
}

/*!
  \return Alpha value of the raster item
  \sa setAlpha()
*/
int QwtPlotRasterItem::alpha() const
{
    return d_data->alpha;
}

/*!
  Change the cache policy

  The default policy is NoCache

  \param policy Cache policy
  \sa CachePolicy, cachePolicy()
*/
void QwtPlotRasterItem::setCachePolicy(
    QwtPlotRasterItem::CachePolicy policy)
{
    if ( d_data->cache.policy != policy )
    {
        d_data->cache.policy = policy;

        invalidateCache();
        itemChanged();
    }
}

/*!
  \return Cache policy
  \sa CachePolicy, setCachePolicy()
*/
QwtPlotRasterItem::CachePolicy QwtPlotRasterItem::cachePolicy() const
{
    return d_data->cache.policy;
}

/*!
   Invalidate the paint cache
   \sa setCachePolicy()
*/
void QwtPlotRasterItem::invalidateCache()
{
    d_data->cache.image = QImage();
    d_data->cache.area = QRect();
    d_data->cache.size = QSize();
}

/*!
   \brief Returns the recommended raster for a given rect.

   F.e the raster hint can be used to limit the resolution of
   the image that is rendered.

   The default implementation returns an invalid size (QSize()),
   what means: no hint.
*/
QSize QwtPlotRasterItem::rasterHint(const QRectF &) const
{
    return QSize();
}

/*!
  \brief Draw the raster data
  \param painter Painter
  \param xMap X-Scale Map
  \param yMap Y-Scale Map
  \param canvasRect Contents rect of the plot canvas
*/
void QwtPlotRasterItem::draw(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect) const
{
    if ( canvasRect.isEmpty() || d_data->alpha == 0 )
        return;

    /*
        Scaling a rastered image always results in a loss of
        precision/quality. So we always render the image in
        paint device resolution.
     */

	QTransform tr = painter->transform();

    QwtScaleMap xxMap = xMap;
	QwtScaleMap yyMap = yMap;
	QRectF cRect = canvasRect;

#if 0
	if ( !tr.isRotating() )
#endif
	{
		transformMaps(tr, xMap, yMap, xxMap, yyMap);
    	cRect = tr.mapRect(canvasRect);
		tr = QTransform();
	}

    QRectF area = QwtScaleMap::invTransform(xxMap, yyMap, cRect);

    const QRectF br = boundingRect();
    if ( br.isValid() )
        area &= br;

    QRectF paintRect = QwtScaleMap::transform( xxMap, yyMap, area);
    if ( !paintRect.isValid() )
        return;

    // align the image to the raster of the paint device
    const QRect imageRect = paintRect.toAlignedRect();

    QRectF imageArea = QwtScaleMap::invTransform(
        xxMap, yyMap, imageRect);
    if ( br.isValid() )
    {
        // RectF -> Rect -> RectF has some rounding errors
        // we can't avoid. But at least we avoid, that
        // these run around the bounding rect
        imageArea &= br;
    }

    QImage image;

    QwtPlotRasterItem::CachePolicy policy = d_data->cache.policy;
    if ( policy != QwtPlotRasterItem::NoCache )
    {
        // Caching doesn't make sense, when the item is
        // not painted to screen

        switch(painter->paintEngine()->type())
        {
            case QPaintEngine::SVG:
            case QPaintEngine::Pdf:
            case QPaintEngine::PostScript:
            case QPaintEngine::MacPrinter:
            case QPaintEngine::Picture:
                policy = QwtPlotRasterItem::NoCache;
                break;
            default:;
        }
    }

    if ( policy == QwtPlotRasterItem::PaintCache )
    {
        if ( d_data->cache.image.isNull() || d_data->cache.area != imageArea
            || d_data->cache.size != imageRect.size() )
        {
            d_data->cache.image = renderImage(xxMap, yyMap, imageArea);
            d_data->cache.area = imageArea;
            d_data->cache.size = imageRect.size();
        }

        image = d_data->cache.image;
    }
	else
	{
        image = renderImage(xxMap, yyMap, imageArea);
	}

    if ( d_data->alpha >= 0 && d_data->alpha < 255 )
        image = toRgba(image, d_data->alpha);

    painter->save();
    painter->setWorldTransform(tr);

    QwtPainter::drawImage(painter, paintRect, image);

    painter->restore();
}
