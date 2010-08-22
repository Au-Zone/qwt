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

static QRectF expandToPixels(const QRectF &rect, const QRectF &pixelRect)
{
    const double pw = pixelRect.width();
    const double ph = pixelRect.height();

    const double dx1 = pixelRect.left() - rect.left();
    const double dx2 = pixelRect.right() - rect.right();
    const double dy1 = pixelRect.top() - rect.top();
    const double dy2 = pixelRect.bottom() - rect.bottom();

    QRectF r;
    r.setLeft( pixelRect.left() - qCeil( dx1 / pw ) * pw );
    r.setTop( pixelRect.top() - qCeil( dy1 / ph ) * ph );
    r.setRight( pixelRect.right() - qFloor( dx2 / pw ) * pw );
    r.setBottom( pixelRect.bottom() - qFloor( dy2 / ph ) * ph );

    return r;
}

static void transformMaps( const QTransform &tr,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    QwtScaleMap &xxMap, QwtScaleMap &yyMap )
{
    const QPointF p1 = tr.map( QPointF( xMap.p1(), yMap.p1() ) );
    const QPointF p2 = tr.map( QPointF( xMap.p2(), yMap.p2() ) );

    xxMap = xMap;
    xxMap.setPaintInterval( p1.x(), p2.x() );

    yyMap = yMap;
    yyMap.setPaintInterval( p1.y(), p2.y() );
}

static bool useCache( QwtPlotRasterItem::CachePolicy policy,
    const QPainter *painter )
{
    bool doCache = false;

    if ( policy == QwtPlotRasterItem::PaintCache )
    {
        // Caching doesn't make sense, when the item is
        // not painted to screen

        switch ( painter->paintEngine()->type() )
        {
            case QPaintEngine::SVG:
            case QPaintEngine::Pdf:
            case QPaintEngine::PostScript:
            case QPaintEngine::MacPrinter:
            case QPaintEngine::Picture:
                break;
            default:;
                doCache = true;
        }
    }

    return doCache;
}

class QwtPlotRasterItem::PrivateData
{
public:
    PrivateData():
        alpha( -1 )
    {
        cache.policy = QwtPlotRasterItem::NoCache;
    }

    int alpha;

    struct ImageCache
    {
        QwtPlotRasterItem::CachePolicy policy;
        QRectF area;
        QSizeF size;
        QImage image;
    } cache;
};

static QImage toRgba( const QImage& image, int alpha )
{
    if ( alpha < 0 || alpha >= 255 )
        return image;

    QImage alphaImage( image.size(), QImage::Format_ARGB32 );

    const QRgb mask1 = qRgba( 0, 0, 0, alpha );
    const QRgb mask2 = qRgba( 255, 255, 255, 0 );
    const QRgb mask3 = qRgba( 0, 0, 0, 255 );

    const int w = image.size().width();
    const int h = image.size().height();

    if ( image.depth() == 8 )
    {
        for ( int y = 0; y < h; y++ )
        {
            QRgb* alphaLine = ( QRgb* )alphaImage.scanLine( y );
            const unsigned char *line = image.scanLine( y );

            for ( int x = 0; x < w; x++ )
                *alphaLine++ = ( image.color( *line++ ) & mask2 ) | mask1;
        }
    }
    else if ( image.depth() == 32 )
    {
        for ( int y = 0; y < h; y++ )
        {
            QRgb* alphaLine = ( QRgb* )alphaImage.scanLine( y );
            const QRgb* line = ( const QRgb* ) image.scanLine( y );

            for ( int x = 0; x < w; x++ )
            {
                const QRgb rgb = *line++;
                if ( rgb & mask3 ) // alpha != 0
                    *alphaLine++ = ( rgb & mask2 ) | mask1;
                else
                    *alphaLine++ = rgb;
            }
        }
    }

    return alphaImage;
}

//! Constructor
QwtPlotRasterItem::QwtPlotRasterItem( const QString& title ):
    QwtPlotItem( QwtText( title ) )
{
    init();
}

//! Constructor
QwtPlotRasterItem::QwtPlotRasterItem( const QwtText& title ):
    QwtPlotItem( title )
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

    setItemAttribute( QwtPlotItem::AutoScale, true );
    setItemAttribute( QwtPlotItem::Legend, false );

    setZ( 8.0 );
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
void QwtPlotRasterItem::setAlpha( int alpha )
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
    QwtPlotRasterItem::CachePolicy policy )
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
   \brief Pixel hint

   The geometry of a pixel is used to calculated the resolution and
   alignment of the rendered image. 

   Width and height of the hint need to be the horizontal  
   and vertical distances between 2 neighboured points. 
   The center of the hint has to be the position of any point 
   ( it doesn't matter which one ).

   Limiting the resolution of the image might significantly improve
   the performance and heavily reduce the amount of memory when rendering
   a QImage from the raster data. 

   The default implementation returns an empty rectangle (QRectF()),
   meaning, that the image will be rendered in target device ( f.e screen )
   resolution.

   \param area In most implementations the resolution of the data doesn't
               depend on the requested area.

   \return Bounding rectangle of a pixel

   \sa render(), renderImage()
*/
QRectF QwtPlotRasterItem::pixelHint( const QRectF & ) const
{
    return QRectF();
}

/*!
  \brief Draw the raster data
  \param painter Painter
  \param xMap X-Scale Map
  \param yMap Y-Scale Map
  \param canvasRect Contents rect of the plot canvas
*/
void QwtPlotRasterItem::draw( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect ) const
{
    if ( canvasRect.isEmpty() || d_data->alpha == 0 )
        return;

    /*
        Scaling a rastered image always results in a loss of
        precision/quality. So we always render the image in
        paint device resolution.
    */

    QwtScaleMap xxMap, yyMap;
    transformMaps( painter->transform(), xMap, yMap, xxMap, yyMap );

    QRectF paintRect = painter->transform().mapRect( canvasRect );
    QRectF area = QwtScaleMap::invTransform( xxMap, yyMap, paintRect );

    const QRectF br = boundingRect();
    if ( br.isValid() && !br.contains( area ) )
    {
        area &= br;
        if ( !area.isValid() )
            return;

        paintRect = QwtScaleMap::transform( xxMap, yyMap, area );
    }

    QRectF imageRect = paintRect;
    QSize imageSize = QSize( qCeil( imageRect.width() ),
            qCeil( imageRect.height() ) );
            

    const QRectF pixelRect = pixelHint(area);
    if ( !pixelRect.isEmpty() )
    {
        // align the area to the data pixels
        area = expandToPixels(area, pixelRect);
        imageRect = QwtScaleMap::transform( xxMap, yyMap, area );

        imageSize.setWidth( qRound( area.width() / pixelRect.width() ) );
        imageSize.setHeight( qRound( area.height() / pixelRect.height() ) );
    }

    QImage image;

    if ( useCache( d_data->cache.policy, painter ) )
    {
        if ( d_data->cache.image.isNull()
            || d_data->cache.area != area
            || d_data->cache.size != imageRect.size() )
        {
            d_data->cache.area = area;
            d_data->cache.size = imageRect.size();
            d_data->cache.image = renderImage( 
                xxMap, yyMap, area, imageSize );
        }

        image = d_data->cache.image;
    }
    else
    {
        image = renderImage( xxMap, yyMap, area, imageSize );
    }

    if ( d_data->alpha >= 0 && d_data->alpha < 255 )
        image = toRgba( image, d_data->alpha );

    painter->save();
    painter->setWorldTransform( QTransform() );

    if ( QwtPainter::isAligning( painter ) )
    {
        imageRect.setLeft( qFloor( imageRect.left() ) );
        imageRect.setRight( qFloor( imageRect.right() ) );
        imageRect.setTop( qFloor( imageRect.top() ) );
        imageRect.setBottom( qFloor( imageRect.bottom() ) );

        paintRect.setLeft( qFloor( paintRect.left() ) );
        paintRect.setRight( qFloor( paintRect.right() ) );
        paintRect.setTop( qFloor( paintRect.top() ) );
        paintRect.setBottom( qFloor( paintRect.bottom() ) );
    }

    painter->setClipRect(paintRect, Qt::IntersectClip);
    QwtPainter::drawImage( painter, imageRect, image );

    painter->restore();
}

/*!
  Renders an image for an area

  The format of the image must be QImage::Format_Indexed8,
  QImage::Format_RGB32 or QImage::Format_ARGB32

  \param xMap Maps x-values into pixel coordinates.
  \param yMap Maps y-values into pixel coordinates.
  \param area Requested area for the image in scale coordinates

  \sa render()
*/
QImage QwtPlotRasterItem::renderImage(
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &area, const QSize &imageSize ) const
{
    if ( area.isEmpty() )
        return QImage();

    const QSizeF pixelSize( area.width() / imageSize.width(), 
        area.height() / imageSize.height() );

    double px1 = 0.0;
    double px2 = imageSize.width();
    if ( xMap.p1() > xMap.p2() )
        qSwap( px1, px2 );

    double sx1 = area.left();
    double sx2 = area.right();
    if ( xMap.s1() > xMap.s2() )
        qSwap( sx1, sx2 );

    double py1 = 0.0;
    double py2 = imageSize.height();
    if ( yMap.p1() > yMap.p2() )
        qSwap( py1, py2 );

    double sy1 = area.top();
    double sy2 = area.bottom();
    if ( yMap.s1() > yMap.s2() )
        qSwap( sy1, sy2 );

    QwtScaleMap xxMap = xMap;
    xxMap.setPaintInterval( px1, px2 );
    xxMap.setScaleInterval( sx1, sx2 );

    QwtScaleMap yyMap = yMap;
    yyMap.setPaintInterval( py1, py2 );
    yyMap.setScaleInterval( sy1, sy2 );

    QImage image = render( xxMap, yyMap, area, imageSize );

    // Mirror the image in case of inverted maps

    const bool hInvert = xxMap.p1() > xxMap.p2();
    const bool vInvert = yyMap.p1() < yyMap.p2();
    if ( hInvert || vInvert )
    {
        // Better invert the image composition !
        image = image.mirrored( hInvert, vInvert );
    }

    return image;
}


/*!
  Returns a QRect based on the values of this rectangle. The coordinates
  in the returned rectangle are rounded to the nearest integer, that
  is inside rect.

  \param rect Rectangle to be rounded
  \return Rectangle rounded to integers
*/
QRect QwtPlotRasterItem::innerRect( const QRectF &rect ) const
{
    const QRectF r = rect.normalized();

    const int left = qCeil( r.left() );
    const int top = qCeil( r.top() );
    const int right = qMax( qFloor( r.right() ), left );
    const int bottom = qMax( qFloor( r.bottom() ), top );

    return QRect( left, top, right - left, bottom - top );
}
