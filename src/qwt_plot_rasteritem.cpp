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
#include <float.h>

static QRectF stripRect(const QRectF &rect, const QRectF &area,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QwtInterval &xInterval, const QwtInterval &yInterval)
{
    QRectF r = rect;
    if ( xInterval.borderFlags() & QwtInterval::ExcludeMinimum )
    {
        if ( area.left() <= xInterval.minValue() )
        {
            if ( xMap.isInverting() )
                r.adjust(0, 0, -1, 0);
            else
                r.adjust(1, 0, 0, 0);
        }
    }

    if ( xInterval.borderFlags() & QwtInterval::ExcludeMaximum )
    {
        if ( area.right() >= xInterval.maxValue() )
        {
            if ( xMap.isInverting() )
                r.adjust(1, 0, 0, 0);
            else
                r.adjust(0, 0, -1, 0);
        }
    }

    if ( yInterval.borderFlags() & QwtInterval::ExcludeMinimum )
    {
        if ( area.top() <= yInterval.minValue() )
        {
            if ( yMap.isInverting() )
                r.adjust(0, 0, 0, -1);
            else
                r.adjust(0, 1, 0, 0);
        }
    }

    if ( yInterval.borderFlags() & QwtInterval::ExcludeMaximum )
    {
        if ( area.bottom() >= yInterval.maxValue() )
        {
            if ( yMap.isInverting() )
                r.adjust(0, 1, 0, 0);
            else
                r.adjust(0, 0, 0, -1);
        }
    }

    return r;
}

static QImage expandImage(const QImage &image,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &area, const QRectF &area2, const QRectF &paintRect,
    const QwtInterval &xInterval, const QwtInterval &yInterval )
{
    const QRectF strippedRect = stripRect(paintRect, area2,
        xMap, yMap, xInterval, yInterval);
    const QSize sz = strippedRect.toRect().size();

    const int w = image.width();
    const int h = image.height();

    const QRectF r = QwtScaleMap::transform(xMap, yMap, area).normalized();
    const double pw = ( r.width() - 1) / w;
    const double ph = ( r.height() - 1) / h;

    double px0, py0;
    if ( !xMap.isInverting() )
    {
        px0 = xMap.transform( area2.left() );
        px0 = qRound( px0 );
        px0 = px0 - xMap.transform( area.left() );
    }
    else
    {
        px0 = xMap.transform( area2.right() );
        px0 = qRound( px0 );
        px0 -= xMap.transform( area.right() );

        px0 -= 1.0;
    }
    px0 += strippedRect.left() - paintRect.left();

    if ( !yMap.isInverting() )
    {
        py0 = yMap.transform( area2.top() );
        py0 = qRound( py0 );
        py0 -= yMap.transform( area.top() );
    }
    else
    {
        py0 = yMap.transform( area2.bottom() );
        py0 = qRound( py0 );
        py0 -= yMap.transform( area.bottom() );

        py0 -= 1.0;
    }
    py0 += strippedRect.top() - paintRect.top();

    QImage expanded(sz, image.format());
#if 0
    expanded.fill(QColor(Qt::white).rgb());
    //return expanded;
#endif

    switch( image.depth() )
    {
        case 32:
        {
            for ( int y1 = 0; y1 < h; y1++ )
            {
                int yy1 = qRound( y1 * ph - py0 );
                int yy2 = qRound( ( y1 + 1 ) * ph - py0 );

                if ( yy1 < 0 )
                    yy1 = 0;

                if ( yy2 > sz.height() )
                    yy2 = sz.height();

                const quint32 *line1 = (const quint32 *) image.scanLine( y1 );

                for ( int x1 = 0; x1 < w; x1++ )
                {
                    const quint32 rgb( line1[x1] );

                    int xx1 = qRound( x1 * pw - px0 );
                    int xx2 = qRound( ( x1 + 1 ) * pw - px0 );

                    if ( xx1 < 0 )
                        xx1 = 0;

                    if ( xx2 > sz.width() )
                        xx2 = sz.width();

                    for ( int y2 = yy1; y2 < yy2; y2++ )
                    {
                        quint32 *line2 = ( quint32 *) expanded.scanLine( y2 );
                        for ( int x2 = xx1; x2 < xx2; x2++ ) 
                            line2[x2] = rgb;
                    }       
                }   
            }   
            break;
        }
        case 8:
        {
            for ( int y1 = 0; y1 < h; y1++ )
            {
                int yy1 = qRound( y1 * ph - py0 );
                int yy2 = qRound( ( y1 + 1 ) * ph - py0 );

                if ( yy1 < 0 )
                    yy1 = 0;

                if ( yy2 > sz.height() )
                    yy2 = sz.height();

                const uchar *line1 = image.scanLine( y1 );

                for ( int x1 = 0; x1 < w; x1++ )
                {
                    int xx1 = qRound( x1 * pw - px0 );
                    int xx2 = qRound( ( x1 + 1 ) * pw - px0 );

                    if ( xx1 < 0 )
                        xx1 = 0;

                    if ( xx2 > sz.width() )
                        xx2 = sz.width();

                    for ( int y2 = yy1; y2 < yy2; y2++ )
                    {
                        uchar *line2 = expanded.scanLine( y2 );
                        memset( line2 + xx1, line1[x1], xx2 - xx1 );
                    }       
                }   
            }
            break;
        }
        default:
#if 1
            expanded = image;
#endif
    }
    
    return expanded;
}   

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

    const QwtInterval xInterval = interval( Qt::XAxis );
    const QwtInterval yInterval = interval( Qt::YAxis );

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

    if ( QwtPainter::isAligning( painter ) )
    {
        paintRect.setLeft( qRound( paintRect.left() ) );
        paintRect.setRight( qRound( paintRect.right() ) );
        paintRect.setTop( qRound( paintRect.top() ) );
        paintRect.setBottom( qRound( paintRect.bottom() ) );
    }


    QSize imageSize;
    QRectF imageArea;

    const QRectF pixelRect = pixelHint(area);
    if ( pixelRect.isEmpty() )
    {
        if ( QwtPainter::isAligning( painter ) )
            imageArea = QwtScaleMap::invTransform( xxMap, yyMap, paintRect );
        else
            imageArea = area;

        imageSize.setWidth( qRound( paintRect.width() ) );
        imageSize.setHeight( qRound( paintRect.height() ) );
    }
    else
    {
        // align the area to the data pixels
        imageArea = expandToPixels(area, pixelRect);

        if ( imageArea.right() == xInterval.maxValue() &&
            !( xInterval.borderFlags() & QwtInterval::ExcludeMaximum ) )
        {
            imageArea.adjust(0, 0, pixelRect.width(), 0);
        }
        if ( imageArea.bottom() == yInterval.maxValue() &&
            !( yInterval.borderFlags() & QwtInterval::ExcludeMaximum ) )
        {
            imageArea.adjust(0, 0, 0, pixelRect.height() );
        }

        imageSize.setWidth( qRound( imageArea.width() / pixelRect.width() ) );
        imageSize.setHeight( qRound( imageArea.height() / pixelRect.height() ) );
    }

    QImage image;

    if ( useCache( d_data->cache.policy, painter ) )
    {
        if ( d_data->cache.image.isNull()
            || d_data->cache.area != imageArea
            || d_data->cache.size != paintRect.size() )
        {
            d_data->cache.area = imageArea;
            d_data->cache.size = paintRect.size();
            d_data->cache.image = renderImage( 
                xxMap, yyMap, imageArea, paintRect, imageSize );
        }

        image = d_data->cache.image;
    }
    else
    {
        image = renderImage( xxMap, yyMap, imageArea, paintRect, imageSize );
    }

    if ( d_data->alpha >= 0 && d_data->alpha < 255 )
        image = toRgba( image, d_data->alpha );

    const QRectF imageRect = stripRect(paintRect, area, 
        xxMap, yyMap, xInterval, yInterval);

    if ( pixelRect.isEmpty() )
    {
        if ( imageRect != paintRect )
        {
            const QRect r( 
                qRound( imageRect.x() - paintRect.x()),
                qRound( imageRect.y() - paintRect.y() ),
                qRound( imageRect.width() ),
                qRound( imageRect.height() ) );
                
            image = image.copy(r);
        }   
    }
    else
    {
        if ( image.width() > 1 || image.height() > 1 )
        {
            image = expandImage(image, xxMap, yyMap, 
                imageArea, area, paintRect, xInterval, yInterval );
        }
    }

    painter->save();
    painter->setWorldTransform( QTransform() );
    
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
    const QRectF &area, const QRectF &paintRect,
    const QSize &imageSize ) const
{
    if ( area.isEmpty() )
        return QImage();

    double px1 = 0.0;
    double px2 = imageSize.width();
    double sx1 = area.left();
    double sx2 = area.right();
    if ( xMap.isInverting() && ( sx1 < sx2 ) )
        qSwap( sx1, sx2 );

    if ( paintRect.toRect().width() > imageSize.width() )
    {
        double xOff = 0.5 * area.width() / imageSize.width();
        if ( xMap.isInverting() )
            xOff = -xOff;

        sx1 += xOff;
        sx2 += xOff;
    }
    else
    {
        px2--;
    }

    QwtScaleMap xxMap = xMap;
    xxMap.setPaintInterval( px1, px2 );
    xxMap.setScaleInterval( sx1, sx2 );

    double py1 = 0.0;
    double py2 = imageSize.height();
    double sy1 = area.top();
    double sy2 = area.bottom();

    if ( paintRect.toRect().height() > imageSize.height() )
    {
        double yOff = 0.5 * area.height() / imageSize.height();
        if ( yMap.isInverting() )
            yOff = -yOff;

        sy1 += yOff;
        sy2 += yOff;
    }
    else
    {
        py2--;
    }

    if ( yMap.isInverting() && ( sy1 < sy2 ) )
        qSwap( sy1, sy2 );

    QwtScaleMap yyMap = yMap;
    yyMap.setPaintInterval( py1, py2 );
    yyMap.setScaleInterval( sy1, sy2 );

    QImage image = render( xxMap, yyMap, area, imageSize );
#if 0
    for ( int i = -1; i <= 3; i++ )
    {
        double x1 = xxMap.transform(i);
        double x2 = xMap.transform(i);

        qDebug() << i << ": " << x1 << paintRect.x() + x1 << x2;
        qDebug() << xxMap.invTransform(0) << " -> " << 
            xxMap.invTransform(imageSize.width() - 1);
    }
    qDebug() << area;
#endif

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

QwtInterval QwtPlotRasterItem::interval(Qt::Axis) const
{
    return QwtInterval();
}

/*!
   \return Bounding rect of the data
   \sa QwtPlotRasterItem::interval()
*/
QRectF QwtPlotRasterItem::boundingRect() const
{
    const QwtInterval intervalX = interval( Qt::XAxis );
    const QwtInterval intervalY = interval( Qt::YAxis );

    if ( !intervalX.isValid() && !intervalY.isValid() )
        return QRectF(); // no bounding rect

    QRectF r;

    if ( intervalX.isValid() )
    {
        r.setLeft( intervalX.minValue() );
        r.setRight( intervalX.maxValue() );
    }
    else
    {
        r.setLeft(-0.5 * DBL_MAX);
        r.setWidth(DBL_MAX);
    }

    if ( intervalY.isValid() )
    {
        r.setTop( intervalY.minValue() );
        r.setBottom( intervalY.maxValue() );
    }
    else
    {
        r.setTop(-0.5 * DBL_MAX);
        r.setHeight(DBL_MAX);
    }

    return r.normalized();
}
