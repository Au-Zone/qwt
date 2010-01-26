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

    QwtScaleMap xxMap = xMap;
    QwtScaleMap yyMap = yMap;
    QRectF scaledCanvasRect = canvasRect;

    const QTransform tr = painter->transform();
    if ( tr.isScaling() )
    {
        /*
            Scaling a rastered image always results in a loss of
            precision/quality. So we always render the image in
            paint device resolution.
         */

        scaledCanvasRect = tr.mapRect(scaledCanvasRect);
        xxMap.setPaintInterval(tr.m11() * xxMap.p1(), tr.m11() * xxMap.p2());
        yyMap.setPaintInterval(tr.m22() * yyMap.p1(), tr.m22() * yyMap.p2());
    }

    QRectF area = QwtScaleMap::invTransform(
        xxMap, yyMap, scaledCanvasRect);
    if ( boundingRect().isValid() )
        area &= boundingRect();

    QRectF paintRect = QwtScaleMap::xTransform(
        xxMap, yyMap, area);
    if ( !paintRect.isValid() )
        return;

    // after scaling the rectangle we also need to do the
    // translation of the painter, when we want to use a
    // painter without transformation later.

    paintRect.translate(tr.m31(), tr.m32());

    // align the image to the raster of the paint device
    const QRect imageRect = paintRect.toAlignedRect();
    const QRectF imageArea = QwtScaleMap::invTransform(
        xxMap, yyMap, imageRect);

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

    if ( policy == QwtPlotRasterItem::NoCache )
    {
        image = renderImage(xxMap, yyMap, imageArea);
    }
    else if ( policy == QwtPlotRasterItem::PaintCache )
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
    else if ( policy == QwtPlotRasterItem::ScreenCache )
    {
        const QSize screenSize =
            QApplication::desktop()->screenGeometry().size();

        if ( imageArea.width() > screenSize.width() ||
            imageArea.height() > screenSize.height() )
        {
            image = renderImage(xxMap, yyMap, imageArea);
        }
        else
        {
            if ( d_data->cache.image.isNull() || 
                d_data->cache.area != imageArea )
            {
                QwtScaleMap cacheXMap = xxMap;
                cacheXMap.setPaintInterval( 0, screenSize.width());

                QwtScaleMap cacheYMap = yyMap;
                cacheYMap.setPaintInterval(screenSize.height(), 0);

                d_data->cache.image = renderImage(
                    cacheXMap, cacheYMap, imageArea);
                d_data->cache.area = imageArea;
                d_data->cache.size = imageRect.size();
            }

            image = d_data->cache.image;
        }
    }

    if ( d_data->alpha >= 0 && d_data->alpha < 255 )
        image = toRgba(image, d_data->alpha);

    painter->save();
    painter->resetTransform();

    QwtPainter::drawImage(painter, paintRect, image);

    painter->restore();
}
