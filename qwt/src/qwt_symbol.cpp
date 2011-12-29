/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_symbol.h"
#include "qwt_painter.h"
#include <qapplication.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qpixmap.h>
#include <qpaintengine.h>
#include <qmath.h>

namespace QwtTriangle
{
    enum Type
    {
        Left,
        Right,
        Up,
        Down
    };
}

static inline QRectF qwtPathBoundingRect( 
    const QPainterPath &path, const QPen &pen )
{
    QPainterPathStroker stroker;
    stroker.setDashPattern( pen.style() );
    stroker.setWidth( pen.widthF() );
    stroker.setJoinStyle( pen.joinStyle() );
    stroker.setCapStyle( pen.capStyle() );
    stroker.setMiterLimit( pen.miterLimit() );

    const QPainterPath strokedPath = stroker.createStroke( path );
    return strokedPath.boundingRect();
}

static inline void qwtDrawPathSymbols( QPainter *painter, 
    const QSizeF &pathSize, const QPointF *points, int numPoints, 
    const QwtSymbol &symbol )
{
    if ( pathSize.isEmpty() )
        return;

    painter->setBrush( symbol.brush() );
    painter->setPen( symbol.pen() );

    double sx = 1.0;
    double sy = 1.0;

    const QSize sz = symbol.size();
    if ( sz.isValid() )
    {
        sx = sz.width() / pathSize.width();
        sy = sz.height() / pathSize.height();
    }

    for ( int i = 0; i < numPoints; i++ )
    {
        // to avoid scaling of non cosmetic pens we
        // map the path instead of setting the transformation
        // for the painter

        QTransform transform;
        transform.translate( points[i].x(), points[i].y() );
        transform.scale( sx, sy );

        const QPainterPath path = transform.map( symbol.path() );
        painter->drawPath( path );
    }
}

static inline void qwtDrawEllipseSymbols( QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol )
{
    painter->setBrush( symbol.brush() );
    painter->setPen( symbol.pen() );

    const QSize size = symbol.size();

    if ( QwtPainter::roundingAlignment( painter ) )
    {
        const int sw = size.width();
        const int sh = size.height();
        const int sw2 = size.width() / 2;
        const int sh2 = size.height() / 2;

        for ( int i = 0; i < numPoints; i++ )
        {
            const int x = qRound( points[i].x() );
            const int y = qRound( points[i].y() );

            const QRectF r( x - sw2, y - sh2, sw, sh );
            QwtPainter::drawEllipse( painter, r );
        }
    }
    else
    {
        const double sw = size.width();
        const double sh = size.height();
        const double sw2 = 0.5 * size.width();
        const double sh2 = 0.5 * size.height();

        for ( int i = 0; i < numPoints; i++ )
        {
            const double x = points[i].x();
            const double y = points[i].y();

            const QRectF r( x - sw2, y - sh2, sw, sh );
            QwtPainter::drawEllipse( painter, r );
        }
    }
}

static inline void qwtDrawRectSymbols( QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol )
{
    const QSize size = symbol.size();

    QPen pen = symbol.pen();
    pen.setJoinStyle( Qt::MiterJoin );
    painter->setPen( pen );
    painter->setBrush( symbol.brush() );
    painter->setRenderHint( QPainter::Antialiasing, false );

    if ( QwtPainter::roundingAlignment( painter ) )
    {
        const int sw = size.width();
        const int sh = size.height();
        const int sw2 = size.width() / 2;
        const int sh2 = size.height() / 2;

        for ( int i = 0; i < numPoints; i++ )
        {
            const int x = qRound( points[i].x() );
            const int y = qRound( points[i].y() );

            const QRect r( x - sw2, y - sh2, sw, sh );
            QwtPainter::drawRect( painter, r );
        }
    }
    else
    {
        const double sw = size.width();
        const double sh = size.height();
        const double sw2 = 0.5 * size.width();
        const double sh2 = 0.5 * size.height();

        for ( int i = 0; i < numPoints; i++ )
        {
            const double x = points[i].x();
            const double y = points[i].y();

            const QRectF r( x - sw2, y - sh2, sw, sh );
            QwtPainter::drawRect( painter, r );
        }
    }
}

static inline void qwtDrawDiamondSymbols( QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol )
{
    const QSize size = symbol.size();

    QPen pen = symbol.pen();
    pen.setJoinStyle( Qt::MiterJoin );
    painter->setPen( pen );
    painter->setBrush( symbol.brush() );

    if ( QwtPainter::roundingAlignment( painter ) )
    {
        for ( int i = 0; i < numPoints; i++ )
        {
            const int x = qRound( points[i].x() );
            const int y = qRound( points[i].y() );

            const int x1 = x - size.width() / 2;
            const int y1 = y - size.height() / 2;
            const int x2 = x1 + size.width();
            const int y2 = y1 + size.height();

            QPolygonF polygon;
            polygon += QPointF( x, y1 );
            polygon += QPointF( x1, y );
            polygon += QPointF( x, y2 );
            polygon += QPointF( x2, y );

            QwtPainter::drawPolygon( painter, polygon );
        }
    }
    else
    {
        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF &pos = points[i];

            const double x1 = pos.x() - 0.5 * size.width();
            const double y1 = pos.y() - 0.5 * size.height();
            const double x2 = x1 + size.width();
            const double y2 = y1 + size.height();

            QPolygonF polygon;
            polygon += QPointF( pos.x(), y1 );
            polygon += QPointF( x2, pos.y() );
            polygon += QPointF( pos.x(), y2 );
            polygon += QPointF( x1, pos.y() );

            QwtPainter::drawPolygon( painter, polygon );
        }
    }
}

static inline void qwtDrawTriangleSymbols(
    QPainter *painter, QwtTriangle::Type type,
    const QPointF *points, int numPoints,
    const QwtSymbol &symbol )
{
    const QSize size = symbol.size();

    QPen pen = symbol.pen();
    pen.setJoinStyle( Qt::MiterJoin );
    painter->setPen( pen );

    painter->setBrush( symbol.brush() );

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    double sw2 = 0.5 * size.width();
    double sh2 = 0.5 * size.height();

    if ( doAlign )
    {
        sw2 = qFloor( sw2 );
        sh2 = qFloor( sh2 );
    }

    QPolygonF triangle( 3 );
    QPointF *trianglePoints = triangle.data();

    for ( int i = 0; i < numPoints; i++ )
    {
        const QPointF &pos = points[i];

        double x = pos.x();
        double y = pos.y();

        if ( doAlign )
        {
            x = qRound( x );
            y = qRound( y );
        }

        const double x1 = x - sw2;
        const double x2 = x1 + size.width();
        const double y1 = y - sh2;
        const double y2 = y1 + size.height();

        switch ( type )
        {
            case QwtTriangle::Left:
            {
                trianglePoints[0].rx() = x2;
                trianglePoints[0].ry() = y1;

                trianglePoints[1].rx() = x1;
                trianglePoints[1].ry() = y;

                trianglePoints[2].rx() = x2;
                trianglePoints[2].ry() = y2;

                break;
            }
            case QwtTriangle::Right:
            {
                trianglePoints[0].rx() = x1;
                trianglePoints[0].ry() = y1;

                trianglePoints[1].rx() = x2;
                trianglePoints[1].ry() = y;

                trianglePoints[2].rx() = x1;
                trianglePoints[2].ry() = y2;

                break;
            }
            case QwtTriangle::Up:
            {
                trianglePoints[0].rx() = x1;
                trianglePoints[0].ry() = y2;

                trianglePoints[1].rx() = x;
                trianglePoints[1].ry() = y1;

                trianglePoints[2].rx() = x2;
                trianglePoints[2].ry() = y2;

                break;
            }
            case QwtTriangle::Down:
            {
                trianglePoints[0].rx() = x1;
                trianglePoints[0].ry() = y1;

                trianglePoints[1].rx() = x;
                trianglePoints[1].ry() = y2;

                trianglePoints[2].rx() = x2;
                trianglePoints[2].ry() = y1;

                break;
            }
        }
        QwtPainter::drawPolygon( painter, triangle );
    }
}

static inline void qwtDrawLineSymbols(
    QPainter *painter, int orientations,
    const QPointF *points, int numPoints, const QwtSymbol &symbol )
{
    const QSize size = symbol.size();

    int off = 0;

    QPen pen = symbol.pen();
    if ( pen.width() > 1 )
    {
        pen.setCapStyle( Qt::FlatCap );
        off = 1;
    }

    painter->setPen( pen );
    painter->setRenderHint( QPainter::Antialiasing, false );

    if ( QwtPainter::roundingAlignment( painter ) )
    {
        const int sw = qFloor( size.width() );
        const int sh = qFloor( size.height() );
        const int sw2 = size.width() / 2;
        const int sh2 = size.height() / 2;

        for ( int i = 0; i < numPoints; i++ )
        {
            if ( orientations & Qt::Horizontal )
            {
                const int x = qRound( points[i].x() ) - sw2;
                const int y = qRound( points[i].y() );

                QwtPainter::drawLine( painter, x, y, x + sw + off, y );
            }
            if ( orientations & Qt::Vertical )
            {
                const int x = qRound( points[i].x() );
                const int y = qRound( points[i].y() ) - sh2;

                QwtPainter::drawLine( painter, x, y, x, y + sh + off );
            }
        }
    }
    else
    {
        const double sw = size.width();
        const double sh = size.height();
        const double sw2 = 0.5 * size.width();
        const double sh2 = 0.5 * size.height();

        for ( int i = 0; i < numPoints; i++ )
        {
            if ( orientations & Qt::Horizontal )
            {
                const double x = points[i].x() - sw2;
                const double y = points[i].y();

                QwtPainter::drawLine( painter, x, y, x + sw, y );
            }
            if ( orientations & Qt::Vertical )
            {
                const double y = points[i].y() - sh2;
                const double x = points[i].x();

                QwtPainter::drawLine( painter, x, y, x, y + sh );
            }
        }
    }
}

static inline void qwtDrawXCrossSymbols( QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol )
{
    const QSize size = symbol.size();
    int off = 0;

    QPen pen = symbol.pen();
    if ( pen.width() > 1 )
    {
        pen.setCapStyle( Qt::FlatCap );
        off = 1;
    }
    painter->setPen( pen );


    if ( QwtPainter::roundingAlignment( painter ) )
    {
        const int sw = size.width();
        const int sh = size.height();
        const int sw2 = size.width() / 2;
        const int sh2 = size.height() / 2;

        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF &pos = points[i];

            const int x = qRound( pos.x() );
            const int y = qRound( pos.y() );

            const int x1 = x - sw2;
            const int x2 = x1 + sw + off;
            const int y1 = y - sh2;
            const int y2 = y1 + sh + off;

            QwtPainter::drawLine( painter, x1, y1, x2, y2 );
            QwtPainter::drawLine( painter, x2, y1, x1, y2 );
        }
    }
    else
    {
        const double sw = size.width();
        const double sh = size.height();
        const double sw2 = 0.5 * size.width();
        const double sh2 = 0.5 * size.height();

        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF &pos = points[i];

            const double x1 = pos.x() - sw2;
            const double x2 = x1 + sw;
            const double y1 = pos.y() - sh2;
            const double y2 = y1 + sh;

            QwtPainter::drawLine( painter, x1, y1, x2, y2 );
            QwtPainter::drawLine( painter, x1, y2, x2, y1 );
        }
    }
}

static inline void qwtDrawStar1Symbols( QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol )
{
    const QSize size = symbol.size();
    painter->setPen( symbol.pen() );

    if ( QwtPainter::roundingAlignment( painter ) )
    {
        QRect r( 0, 0, size.width(), size.height() );

        for ( int i = 0; i < numPoints; i++ )
        {
            r.moveCenter( points[i].toPoint() );

            const double sqrt1_2 = 0.70710678118654752440; /* 1/sqrt(2) */

            const double d1 = r.width() / 2.0 * ( 1.0 - sqrt1_2 );

            QwtPainter::drawLine( painter,
                qRound( r.left() + d1 ), qRound( r.top() + d1 ),
                qRound( r.right() - d1 ), qRound( r.bottom() - d1 ) );
            QwtPainter::drawLine( painter,
                qRound( r.left() + d1 ), qRound( r.bottom() - d1 ),
                qRound( r .right() - d1), qRound( r.top() + d1 ) );

            const QPoint c = r.center();

            QwtPainter::drawLine( painter,
                c.x(), r.top(), c.x(), r.bottom() );
            QwtPainter::drawLine( painter,
                r.left(), c.y(), r.right(), c.y() );
        }
    }
    else
    {
        QRectF r( 0, 0, size.width(), size.height() );

        for ( int i = 0; i < numPoints; i++ )
        {
            r.moveCenter( points[i] );

            const double sqrt1_2 = 0.70710678118654752440; /* 1/sqrt(2) */

            const QPointF c = r.center();
            const double d1  = r.width() / 2.0 * ( 1.0 - sqrt1_2 );

            QwtPainter::drawLine( painter,
                r.left() + d1, r.top() + d1,
                r.right() - d1, r.bottom() - d1 );
            QwtPainter::drawLine( painter,
                r.left() + d1, r.bottom() - d1,
                r.right() - d1, r.top() + d1 );
            QwtPainter::drawLine( painter,
                c.x(), r.top(),
                c.x(), r.bottom() );
            QwtPainter::drawLine( painter,
                r.left(), c.y(),
                r.right(), c.y() );
        }
    }
}

static inline void qwtDrawStar2Symbols( QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol )
{
    QPen pen = symbol.pen();
    if ( pen.width() > 1 )
        pen.setCapStyle( Qt::FlatCap );
    pen.setJoinStyle( Qt::MiterJoin );
    painter->setPen( pen );

    painter->setBrush( symbol.brush() );

    const double cos30 = 0.866025; // cos(30°)

    const double dy = 0.25 * symbol.size().height();
    const double dx = 0.5 * symbol.size().width() * cos30 / 3.0;

    QPolygonF star( 12 );
    QPointF *starPoints = star.data();

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    for ( int i = 0; i < numPoints; i++ )
    {
        double x = points[i].x();
        double y = points[i].y();
        if ( doAlign )
        {
            x = qRound( x );
            y = qRound( y );
        }

        double x1 = x - 3 * dx;
        double y1 = y - 2 * dy;
        if ( doAlign )
        {
            x1 = qRound( x - 3 * dx );
            y1 = qRound( y - 2 * dy );
        }

        const double x2 = x1 + 1 * dx;
        const double x3 = x1 + 2 * dx;
        const double x4 = x1 + 3 * dx;
        const double x5 = x1 + 4 * dx;
        const double x6 = x1 + 5 * dx;
        const double x7 = x1 + 6 * dx;

        const double y2 = y1 + 1 * dy;
        const double y3 = y1 + 2 * dy;
        const double y4 = y1 + 3 * dy;
        const double y5 = y1 + 4 * dy;

        starPoints[0].rx() = x4;
        starPoints[0].ry() = y1;

        starPoints[1].rx() = x5;
        starPoints[1].ry() = y2;

        starPoints[2].rx() = x7;
        starPoints[2].ry() = y2;

        starPoints[3].rx() = x6;
        starPoints[3].ry() = y3;

        starPoints[4].rx() = x7;
        starPoints[4].ry() = y4;

        starPoints[5].rx() = x5;
        starPoints[5].ry() = y4;

        starPoints[6].rx() = x4;
        starPoints[6].ry() = y5;

        starPoints[7].rx() = x3;
        starPoints[7].ry() = y4;

        starPoints[8].rx() = x1;
        starPoints[8].ry() = y4;

        starPoints[9].rx() = x2;
        starPoints[9].ry() = y3;

        starPoints[10].rx() = x1;
        starPoints[10].ry() = y2;

        starPoints[11].rx() = x3;
        starPoints[11].ry() = y2;

        QwtPainter::drawPolygon( painter, star );
    }
}

static inline void qwtDrawHexagonSymbols( QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol )
{
    painter->setBrush( symbol.brush() );
    painter->setPen( symbol.pen() );

    const double cos30 = 0.866025; // cos(30°)
    const double dx = 0.5 * ( symbol.size().width() - cos30 );

    const double dy = 0.25 * symbol.size().height();

    QPolygonF hexaPolygon( 6 );
    QPointF *hexaPoints = hexaPolygon.data();

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    for ( int i = 0; i < numPoints; i++ )
    {
        double x = points[i].x();
        double y = points[i].y();
        if ( doAlign )
        {
            x = qRound( x );
            y = qRound( y );
        }

        double x1 = x - dx;
        double y1 = y - 2 * dy;
        if ( doAlign )
        {
            x1 = qCeil( x1 );
            y1 = qCeil( y1 );
        }

        const double x2 = x1 + 1 * dx;
        const double x3 = x1 + 2 * dx;

        const double y2 = y1 + 1 * dy;
        const double y3 = y1 + 3 * dy;
        const double y4 = y1 + 4 * dy;

        hexaPoints[0].rx() = x2;
        hexaPoints[0].ry() = y1;

        hexaPoints[1].rx() = x3;
        hexaPoints[1].ry() = y2;

        hexaPoints[2].rx() = x3;
        hexaPoints[2].ry() = y3;

        hexaPoints[3].rx() = x2;
        hexaPoints[3].ry() = y4;

        hexaPoints[4].rx() = x1;
        hexaPoints[4].ry() = y3;

        hexaPoints[5].rx() = x1;
        hexaPoints[5].ry() = y2;

        QwtPainter::drawPolygon( painter, hexaPolygon );
    }
}

class QwtSymbol::PrivateData
{
public:
    PrivateData( QwtSymbol::Style st, const QBrush &br,
            const QPen &pn, const QSize &sz ):
        style( st ),
        size( sz ),
        brush( br ),
        pen( pn )
    {
        cache.policy = QwtSymbol::AutoCache;
    }

    bool operator==( const PrivateData &other ) const
    {
        return ( style == other.style )
            && ( size == other.size )
            && ( brush == other.brush )
            && ( pen == other.pen );
    }

    Style style;
    QSize size;
    QBrush brush;
    QPen pen;

    struct Path
    {
        QRectF pathRect;
        QRectF outerRect;
        QPainterPath path;
    } path;

    struct PaintCache
    {
        QwtSymbol::CachePolicy policy;
        QPixmap pixmap;

    } cache;
};

/*!
  Default Constructor
  \param style Symbol Style

  The symbol is constructed with gray interior,
  black outline with zero width, no size and style 'NoSymbol'.
*/
QwtSymbol::QwtSymbol( Style style )
{
    d_data = new PrivateData( style, QBrush( Qt::gray ),
        QPen( Qt::black ), QSize() );
}

/*!
  \brief Constructor
  \param style Symbol Style
  \param brush brush to fill the interior
  \param pen outline pen
  \param size size

  \sa setStyle(), setBrush(), setPen(), setSize()
*/
QwtSymbol::QwtSymbol( QwtSymbol::Style style, const QBrush &brush,
    const QPen &pen, const QSize &size )
{
    d_data = new PrivateData( style, brush, pen, size );
}

/*!
  \brief Constructor

  The symbol gets initialized by a painter path. The style is
  set to QwtSymbol::Path, the size is set to empty ( the path
  is displayed unscaled ).

  \param path painter path
  \param brush brush to fill the interior
  \param pen outline pen

  \sa setPath(), setBrush(), setPen(), setSize()
*/

QwtSymbol::QwtSymbol( const QPainterPath &path, const QBrush &brush,
    const QPen &pen )
{
    d_data = new PrivateData( QwtSymbol::Path, brush, pen, QSize() );
    setPath( path );
}

/*!
  \brief Copy constructor

  \param other Symbol
*/
QwtSymbol::QwtSymbol( const QwtSymbol &other )
{
    d_data = new PrivateData( other.style(), other.brush(),
        other.pen(), other.size() );
};

//! Destructor
QwtSymbol::~QwtSymbol()
{
    delete d_data;
}

//! \brief Assignment operator
QwtSymbol &QwtSymbol::operator=( const QwtSymbol &other )
{
    *d_data = *other.d_data;
    return *this;
}

//! \brief Compare two symbols
bool QwtSymbol::operator==( const QwtSymbol &other ) const
{
    return *d_data == *other.d_data;
}

//! \brief Compare two symbols
bool QwtSymbol::operator!=( const QwtSymbol &other ) const
{
    return !( *d_data == *other.d_data );
}

/*!
  Change the cache policy

  The default policy is AutoCache

  \param policy Cache policy
  \sa CachePolicy, cachePolicy()
*/
void QwtSymbol::setCachePolicy(
    QwtSymbol::CachePolicy policy )
{
    if ( d_data->cache.policy != policy )
    {
        d_data->cache.policy = policy;
        invalidateCache();
    }
}

/*!
  \return Cache policy
  \sa CachePolicy, setCachePolicy()
*/
QwtSymbol::CachePolicy QwtSymbol::cachePolicy() const
{
    return d_data->cache.policy;
}

/*!
  \brief Set a painter path as symbol

  The symbol is represented by a painter path, where the 
  origin ( 0, 0 ) of the path coordinate system is mapped to
  the position of the symbol.

  When the symbol has valid size the painter path gets scaled
  to fit into the size. Otherwise the symbol size depends on
  the bounding rectangle of the path.

  \param path Painter path

  \note The style is implicitely set to QwtSymbol::Path.
  \sa path(), setSize()
 */
void QwtSymbol::setPath( const QPainterPath &path )
{
    d_data->style = QwtSymbol::Path;
    d_data->path.path = path;
    d_data->path.pathRect = path.boundingRect();
    d_data->path.outerRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
}

/*!
   \return Painter path for displaying the symbol
   \sa setPath()
*/
const QPainterPath &QwtSymbol::path() const
{
    return d_data->path.path;
}

/*!
  \brief Specify the symbol's size

  If the 'h' parameter is left out or less than 0,
  and the 'w' parameter is greater than or equal to 0,
  the symbol size will be set to (w,w).

  \param width Width
  \param height Height (defaults to -1)

  \sa size()
*/
void QwtSymbol::setSize( int width, int height )
{
    if ( ( width >= 0 ) && ( height < 0 ) )
        height = width;

    setSize( QSize( width, height ) );
}

/*!
   Set the symbol's size
   \param size Size

   \sa size()
*/
void QwtSymbol::setSize( const QSize &size )
{
    if ( size.isValid() && size != d_data->size )
    {
        d_data->size = size;
        invalidateCache();
    }
}

/*!
   \return Size
   \sa setSize()
*/
const QSize& QwtSymbol::size() const
{
    return d_data->size;
}

/*!
  \brief Assign a brush

  The brush is used to draw the interior of the symbol.
  \param brush Brush

  \sa brush()
*/
void QwtSymbol::setBrush( const QBrush &brush )
{
    if ( brush != d_data->brush )
    {
        d_data->brush = brush;
        invalidateCache();
    }
}

/*!
  \return Brush
  \sa setBrush()
*/
const QBrush& QwtSymbol::brush() const
{
    return d_data->brush;
}

/*!
  Assign a pen

  The pen is used to draw the symbol's outline.

  \param pen Pen
  \sa pen(), setBrush()
*/
void QwtSymbol::setPen( const QPen &pen )
{
    if ( pen != d_data->pen )
    {
        d_data->pen = pen;
        invalidateCache();
        d_data->path.outerRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
    }
}

/*!
  \return Pen
  \sa setPen(), brush()
*/
const QPen& QwtSymbol::pen() const
{
    return d_data->pen;
}

/*!
  \brief Set the color of the symbol

  Change the color of the brush for symbol types with a filled area.
  For all other symbol types the color will be assigned to the pen.

  \param color Color

  \sa setBrush(), setPen(), brush(), pen()
*/
void QwtSymbol::setColor( const QColor &color )
{
    switch ( d_data->style )
    {
        case QwtSymbol::Ellipse:
        case QwtSymbol::Rect:
        case QwtSymbol::Diamond:
        case QwtSymbol::Triangle:
        case QwtSymbol::UTriangle:
        case QwtSymbol::DTriangle:
        case QwtSymbol::RTriangle:
        case QwtSymbol::LTriangle:
        case QwtSymbol::Star2:
        case QwtSymbol::Hexagon:
        {
            if ( d_data->brush.color() != color )
            {
                d_data->brush.setColor( color );
                invalidateCache();
            }
            break;
        }
        case QwtSymbol::Cross:
        case QwtSymbol::XCross:
        case QwtSymbol::HLine:
        case QwtSymbol::VLine:
        case QwtSymbol::Star1:
        {
            if ( d_data->pen.color() != color )
            {
                d_data->pen.setColor( color );
                invalidateCache();
            }
            break;
        }
        default:
        {
            if ( d_data->brush.color() != color ||
                d_data->pen.color() != color )
            {
                invalidateCache();
            }

            d_data->brush.setColor( color );
            d_data->pen.setColor( color );
        }
    }
}

/*!
  Render an array of symbols

  Painting several symbols is more effective than drawing symbols
  one by one, as a couple of layout calculations and setting of pen/brush
  can be done once for the complete array.

  \param painter Painter
  \param points Array of points
  \param numPoints Number of points
*/
void QwtSymbol::drawSymbols( QPainter *painter,
    const QPointF *points, int numPoints ) const
{
    if ( numPoints <= 0 )
        return;

    bool useCache = false;

    // Don't use the pixmap, when the paint device
    // could generate scalable vectors

    if ( QwtPainter::roundingAlignment( painter ) )
    {
        if ( d_data->cache.policy == QwtSymbol::Cache )
        {
            useCache = true;
        }
        else if ( d_data->cache.policy == QwtSymbol::AutoCache )
        {
            if ( painter->paintEngine()->type() == QPaintEngine::Raster )
            {
                useCache = true;
            }
            else
            {
                switch( d_data->style )
                {
                    case QwtSymbol::XCross:
                    case QwtSymbol::HLine:
                    case QwtSymbol::VLine:
                    case QwtSymbol::Cross:
                        break;

                    default:
                        useCache = true;
                }
            }
        }
    }
        
    if ( useCache )
    {
        QRect br = boundingRect();
        br.setSize( ( 2 * br.size() + QSize( 1, 1 ) / 2 ) );

        const QPointF center = br.center();
        
        if ( d_data->cache.pixmap.isNull() )
        {
            d_data->cache.pixmap = QPixmap( br.size() );
            d_data->cache.pixmap.fill( Qt::transparent );

            QPainter p( &d_data->cache.pixmap );
            p.setRenderHints( painter->renderHints() );
            renderSymbols( &p, &center, 1 );
            p.end();
        }

        for ( int i = 0; i < numPoints; i++ )
        {
            const int left = qRound( points[i].x() ) - center.x();
            const int top = qRound( points[i].y() ) - center.y();

            painter->drawPixmap( left, top, d_data->cache.pixmap );
        }
    }
    else
    {
        painter->save();
        renderSymbols( painter, points, numPoints );
        painter->restore();
    }
}

void QwtSymbol::renderSymbols( QPainter *painter,
    const QPointF *points, int numPoints ) const
{
    switch ( d_data->style )
    {
        case QwtSymbol::Ellipse:
        {
            qwtDrawEllipseSymbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Rect:
        {
            qwtDrawRectSymbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Diamond:
        {
            qwtDrawDiamondSymbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Cross:
        {
            qwtDrawLineSymbols( painter, Qt::Horizontal | Qt::Vertical,
                points, numPoints, *this );
            break;
        }
        case QwtSymbol::XCross:
        {
            qwtDrawXCrossSymbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Triangle:
        case QwtSymbol::UTriangle:
        {
            qwtDrawTriangleSymbols( painter, QwtTriangle::Up,
                points, numPoints, *this );
            break;
        }
        case QwtSymbol::DTriangle:
        {
            qwtDrawTriangleSymbols( painter, QwtTriangle::Down,
                points, numPoints, *this );
            break;
        }
        case QwtSymbol::RTriangle:
        {
            qwtDrawTriangleSymbols( painter, QwtTriangle::Right,
                points, numPoints, *this );
            break;
        }
        case QwtSymbol::LTriangle:
        {
            qwtDrawTriangleSymbols( painter, QwtTriangle::Left,
                points, numPoints, *this );
            break;
        }
        case QwtSymbol::HLine:
        {
            qwtDrawLineSymbols( painter, Qt::Horizontal,
                points, numPoints, *this );
            break;
        }
        case QwtSymbol::VLine:
        {
            qwtDrawLineSymbols( painter, Qt::Vertical,
                points, numPoints, *this );
            break;
        }
        case QwtSymbol::Star1:
        {
            qwtDrawStar1Symbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Star2:
        {
            qwtDrawStar2Symbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Hexagon:
        {
            qwtDrawHexagonSymbols( painter, points, numPoints, *this );
            break;
        }
        case QwtSymbol::Path:
        {
            qwtDrawPathSymbols( painter, d_data->path.pathRect.size(), 
                points, numPoints, *this );
            break;
        }
        default:;
    }
}

QRect QwtSymbol::boundingRect() const
{
    QRectF rect;

    switch ( d_data->style )
    {
        case QwtSymbol::Ellipse:
        case QwtSymbol::Rect:
        case QwtSymbol::Hexagon:
        {
            qreal pw = 0.0;
            if ( d_data->pen.style() != Qt::NoPen )
                pw = qMax( d_data->pen.widthF(), qreal( 1.0 ) );

            rect.setSize( d_data->size + QSizeF( pw, pw ) );
            rect.moveCenter( QPointF( 0.0, 0.0 ) );

            break;
        }
        case QwtSymbol::XCross:
        case QwtSymbol::Diamond:
        case QwtSymbol::Triangle:
        case QwtSymbol::UTriangle:
        case QwtSymbol::DTriangle:
        case QwtSymbol::RTriangle:
        case QwtSymbol::LTriangle:
        case QwtSymbol::Star1:
        case QwtSymbol::Star2:
        {
            qreal pw = 0.0;
            if ( d_data->pen.style() != Qt::NoPen )
                pw = qMax( d_data->pen.widthF(), qreal( 1.0 ) );

            rect.setSize( d_data->size + QSizeF( 2 * pw, 2 * pw ) );
            rect.moveCenter( QPointF( 0.0, 0.0 ) );
            break;
        }
        case QwtSymbol::Path:
        {
            qreal pw = 0.0;
            if ( d_data->pen.style() != Qt::NoPen )
                pw = qMax( d_data->pen.widthF(), qreal( 1.0 ) );

            if ( d_data->pen.style() != Qt::NoPen &&
                d_data->pen.widthF() > 0.0 )
            {
                if ( d_data->path.outerRect.width() < 0 )
                {
                    d_data->path.outerRect = 
                        qwtPathBoundingRect( d_data->path.path, d_data->pen );
                }

                rect = d_data->path.outerRect;
            }
            else
            {
                const double m = 2 * pw;
                rect = d_data->path.pathRect.adjusted( -m, -m, m, m );
            }

            if ( d_data->size.isValid() && !rect.isEmpty() )
            {
                const QSizeF sz = rect.size() - 
                    0.5 * ( rect.size() - d_data->path.pathRect.size() );

                const double sx = d_data->size.width() / sz.width();
                const double sy = d_data->size.height() / sz.height();

                QTransform transform;
                transform.scale( sx, sy );

                rect = transform.mapRect( rect );
            }

            break;
        }
        default:
        {
            rect.setSize( d_data->size );
            rect.moveCenter( QPointF( 0.0, 0.0 ) );
        }
    }


    QRect r;
    r.setLeft( qFloor( rect.left() ) );
    r.setTop( qFloor( rect.top() ) );
    r.setRight( qCeil( rect.right() ) );
    r.setBottom( qCeil( rect.bottom() ) );

    r.adjust( -1, -1, 1, 1 ); // for antialiasing

    return r;
}

/*!
  Invalidate the cached symbol pixmap

  The symbol invalidates its cache, whenever an attribute is changed
  that has an effect ob how to display a symbol. In case of derived
  classes with indidividual styles ( >= QwtSymbol::UserStyle ) it
  might be necessary to call invalidateCache() for attributes
  that are relevant for this style.

  \sa CachePolicy, setCachePolicy(), drawSymbols()
 */
void QwtSymbol::invalidateCache()
{
    if ( !d_data->cache.pixmap.isNull() )
        d_data->cache.pixmap = QPixmap();
}

/*!
  Specify the symbol style

  \param style Style
  \sa style()
*/
void QwtSymbol::setStyle( QwtSymbol::Style style )
{
    if ( d_data->style != style )
    {
        d_data->style = style;
        invalidateCache();
    }
}

/*!
  \return Current symbol style
  \sa setStyle()
*/
QwtSymbol::Style QwtSymbol::style() const
{
    return d_data->style;
}
