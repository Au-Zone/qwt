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
#include <qmath.h>
#include <qdebug.h>

static inline void qwtDrawEllipseSymbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    painter->setBrush(symbol.brush());
    painter->setPen(symbol.pen());

    const QSize size = symbol.size();

    if ( QwtPainter::isAligning(painter) )
    {
        int pw = 0;
        if ( painter->pen().style() != Qt::NoPen )
            pw = qMax(painter->pen().width(), 1);

        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF &pos = points[i];

            const int x = (int)qCeil(pos.x());
            const int y = (int)qCeil(pos.y());

            const int x1 = x - size.width() / 2;
            const int y1 = y - size.height() / 2;
        
            QRectF r( x1 + 0.5 * pw, y1 + 0.5 * pw, 
                size.width() - pw, size.height() - pw);

            QwtPainter::drawEllipse(painter, r);
        }
    }
    else
    {
        double pw = 0.0;
        if ( painter->pen().style() != Qt::NoPen )
            pw = qMax(painter->pen().widthF(), 1.0);

        QRectF r(0.5 * pw, 0.5 * pw, 
            size.width() - pw, size.height() - pw);

        for ( int i = 0; i < numPoints; i++ )
        {
            r.moveCenter(points[i]);
            QwtPainter::drawEllipse(painter, r);
        }
    }
}

static inline void qwtDrawRectSymbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    const QSize size = symbol.size();

    QPen pen = symbol.pen();
    pen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(pen);
    painter->setBrush(symbol.brush());

    if ( QwtPainter::isAligning(painter) )
    {
        int pw = 0;
        if ( painter->pen().style() != Qt::NoPen )
            pw = qMax(painter->pen().width(), 1);

        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF &pos = points[i];

            const int x = (int)qCeil(pos.x());
            const int y = (int)qCeil(pos.y());

            const int x1 = x - size.width() / 2;
            const int y1 = y - size.height() / 2;

            QRectF r(x1 + pw / 2, y1 + pw / 2, 
                size.width() - pw, size.height() - pw);
        
            QwtPainter::drawRect(painter, r.toRect());
        }
    }
    else
    {
        double pw = 0.0;
        if ( painter->pen().style() != Qt::NoPen )
            pw = qMax(painter->pen().widthF(), 1.0);

        QRectF r(0.5 * pw, 0.5 * pw, 
            size.width() - pw, size.height() - pw);

        for ( int i = 0; i < numPoints; i++ )
        {
            r.moveCenter(points[i]);
            QwtPainter::drawRect(painter, r);
        }
    }
}

static inline void qwtDrawDiamondSymbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    const QSize size = symbol.size();

    QPen pen = symbol.pen();
    pen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(pen);
    painter->setBrush(symbol.brush());

    if ( QwtPainter::isAligning(painter) )
    {
        int pw = 0;
        if ( painter->pen().style() != Qt::NoPen )
            pw = qMax(painter->pen().width(), 1);

        const int off2 = int(0.5 * ::sqrt(2 * pw * pw));

        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF &pos = points[i];

            const int x = (int)qCeil(pos.x());
            const int y = (int)qCeil(pos.y());

            const int x1 = x - size.width() / 2 + off2;
            const int y1 = y - size.height() / 2 + off2;
            const int x2 = x1 + size.width() - 2 * off2;
            const int y2 = y1 + size.height() - 2 * off2;

            QPolygonF polygon;
            polygon += QPointF(x, y1);
            polygon += QPointF(x1, y);
            polygon += QPointF(x, y2);
            polygon += QPointF(x2, y);

            QwtPainter::drawPolygon(painter, polygon);
        }
    }
    else
    {
        double pw = 0.0;
        if ( painter->pen().style() != Qt::NoPen )
            pw = qMax(painter->pen().widthF(), 1.0);

        const double off = ::sqrt(2 * pw * pw);

        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF &pos = points[i];

            const double x1 = pos.x() - 0.5 * (size.width() - off);
            const double y1 = pos.y() - 0.5 * (size.height() - off);
            const double x2 = x1 + size.width() - off;
            const double y2 = y1 + size.height() - off;

            QPolygonF polygon;
            polygon += QPointF(pos.x(), y1);
            polygon += QPointF(x2, pos.y());
            polygon += QPointF(pos.x(), y2);
            polygon += QPointF(x1, pos.y());

#if 1
            painter->save();
            painter->setPen(Qt::gray);
            painter->setBrush(Qt::NoBrush);

            QRectF r(0, 0, size.width(), size.height());
            r.moveCenter(pos);
            painter->drawRect(r.adjusted(0, 0, -1, -1));
            painter->restore();
#endif

            QwtPainter::drawPolygon(painter, polygon);
        }
    }
}

static inline void qwtDrawDTriangleSymbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    const QSize size = symbol.size();

    painter->setPen(symbol.pen());

    const double dx = 0.5 * size.width();
    const double dy = 0.5 * size.height();

    for ( int i = 0; i < numPoints; i++ )
    {
        const QPointF &pos = points[i];

        QPolygonF polygon;
        polygon += QPointF(pos.x() - dx, pos.y() - dy);
        polygon += QPointF(pos.x() + dx, pos.y() - dy);
        polygon += QPointF(pos.x(), pos.y() + dy);

        QwtPainter::drawPolygon(painter, polygon);
    }
}

static inline void qwtDrawUTriangleSymbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    const QSize size = symbol.size();

    painter->setPen(symbol.pen());

    const double dx = 0.5 * size.width();
    const double dy = 0.5 * size.height();

    for ( int i = 0; i < numPoints; i++ )
    {
        const QPointF &pos = points[i];

        QPolygonF polygon;
        polygon += QPointF(pos.x(), pos.y() - dy);
        polygon += QPointF(pos.x() + dx, pos.y() + dy);
        polygon += QPointF(pos.x() - dx, pos.y() + dy);

        QwtPainter::drawPolygon(painter, polygon);
    }
}

static inline void qwtDrawLTriangleSymbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    const QSize size = symbol.size();

    painter->setPen(symbol.pen());

    const double dx = 0.5 * size.width();
    const double dy = 0.5 * size.height();

    for ( int i = 0; i < numPoints; i++ )
    {
        const QPointF &pos = points[i];

        QPolygonF polygon;
        polygon += QPointF(pos.x() + dx, pos.y() - dy);
        polygon += QPointF(pos.x() + dx, pos.y() + dy);
        polygon += QPointF(pos.x() - dx, pos.y());

        QwtPainter::drawPolygon(painter, polygon);
    }
}

static inline void qwtDrawRTriangleSymbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    const QSize size = symbol.size();

    painter->setPen(symbol.pen());

    const double dx = 0.5 * size.width();
    const double dy = 0.5 * size.height();

    for ( int i = 0; i < numPoints; i++ )
    {
        const QPointF &pos = points[i];

        QPolygonF polygon;
        polygon += QPointF(pos.x() - dx, pos.y() - dy);
        polygon += QPointF(pos.x() - dx, pos.y() + dy);
        polygon += QPointF(pos.x() + dx, pos.y());

        QwtPainter::drawPolygon(painter, polygon);
    }
}

static inline void qwtDrawCrossSymbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    const QSize size = symbol.size();

    QPen pen = symbol.pen();
    pen.setCapStyle(Qt::FlatCap);

    painter->setPen(pen);

    if ( QwtPainter::isAligning(painter) )
    {
        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF &pos = points[i];

            const int x = (int)qCeil(pos.x());
            const int y = (int)qCeil(pos.y());

            const int x1 = x - size.width() / 2;
            const int y1 = y - size.height() / 2;

            QwtPainter::drawLine(painter, x1, pos.y(), 
                x1 + size.width() - 1, pos.y());
            QwtPainter::drawLine(painter, pos.x(), y1, 
                pos.x(), y1 + size.height() - 1);
        }
    }
    else
    {
        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF &pos = points[i];

            const double x1 = pos.x() - 0.5 * size.width();
            const double y1 = pos.y() - 0.5 * size.height();

            QwtPainter::drawLine(painter, x1, pos.y(), 
                x1 + size.width(), pos.y());
            QwtPainter::drawLine(painter, pos.x(), y1, 
                pos.x(), y1 + size.height());
        }
    }
}

static inline void qwtDrawXCrossSymbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    const QSize size = symbol.size();

    QPen pen = symbol.pen();
    pen.setCapStyle(Qt::FlatCap);
    painter->setPen(pen);

    if ( QwtPainter::isAligning(painter) )
    {
        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF &pos = points[i];

            const int x = (int)qCeil(pos.x());
            const int y = (int)qCeil(pos.y());

            const int x1 = x - size.width() / 2;
            const int x2 = x1 + size.width();
            const int y1 = y - size.height() / 2;
            const int y2 = y1 + size.height();

            QwtPainter::drawLine(painter, x1, y1, x2, y2);
            QwtPainter::drawLine(painter, x2, y1, x1, y2);
        }
    }
    else
    {
        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF &pos = points[i];

            const double x1 = pos.x() - 0.5 * size.width();
            const double x2 = x1 + size.width();
            const double y1 = pos.y() - 0.5 * size.height();
            const double y2 = y1 + size.height();

            QwtPainter::drawLine(painter, x1, y1, x2, y2);
            QwtPainter::drawLine(painter, x1, y2, x2, y1);
        }
    }
}

static inline void qwtDrawHLineSymbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    const QSize size = symbol.size();

    painter->setPen(symbol.pen());

    QRectF r(0, 0, size.width(), size.height());

    for ( int i = 0; i < numPoints; i++ )
    {
        r.moveCenter(points[i]);
        const QPointF c = r.center();

        QwtPainter::drawLine(painter, 
            r.left(), c.y(), r.right(), c.y());
    }
}

static inline void qwtDrawVLineSymbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    const QSize size = symbol.size();

    painter->setPen(symbol.pen());

    QRectF r(0, 0, size.width(), size.height());

    for ( int i = 0; i < numPoints; i++ )
    {
        r.moveCenter(points[i]);
        const QPointF c = r.center();

        QwtPainter::drawLine(painter, 
            c.x(), r.top(), c.x(), r.bottom());
    }
}

static inline void qwtDrawStar1Symbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    const QSize size = symbol.size();

    painter->setPen(symbol.pen());
    QRectF r(0, 0, size.width(), size.height());

    for ( int i = 0; i < numPoints; i++ )
    {
        r.moveCenter(points[i]);

        const double sqrt1_2 = 0.70710678118654752440; /* 1/sqrt(2) */

        const QPointF c = r.center();
        const double d1  = r.width() / 2.0 * (1.0 - sqrt1_2);

        QwtPainter::drawLine(painter, 
            r.left() + d1, r.top() + d1,
            r.right() - d1, r.bottom() - d1);
        QwtPainter::drawLine(painter, 
            r.left() + d1, r.bottom() - d1,
            r.right() - d1, r.top() + d1);
        QwtPainter::drawLine(painter, 
            c.x(), r.top(),
            c.x(), r.bottom());
        QwtPainter::drawLine(painter, 
            r.left(), c.y(),
            r.right(), c.y());
    }
}

static inline void qwtDrawStar2Symbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    const QSize size = symbol.size();

    painter->setBrush(symbol.brush());
    painter->setPen(symbol.pen());

    QRectF r(0, 0, size.width(), size.height());

    for ( int i = 0; i < numPoints; i++ )
    {
        r.moveCenter(points[i]);
        const double cos30 = 0.866025; // cos(30°)

        const double w = r.width();
        const double side = w * (1.0 - cos30) / 2.0;  
        const double h4 = r.height() / 4.0;
        const double h2 = r.height() / 2.0;
        const double h34 = (r.height() * 3) / 4;

        QPolygonF polygon;
        polygon += QPointF(r.left() + (w / 2), r.top());
        polygon += QPointF(r.right() - (side + (w - 2 * side) / 3),
            r.top() + h4 );
        polygon += QPointF(r.right() - side, r.top() + h4);
        polygon += QPointF(r.right() - (side + (w / 2 - side) / 3),
            r.top() + h2 );
        polygon += QPointF(r.right() - side, r.top() + h34);
        polygon += QPointF(r.right() - (side + (w - 2 * side) / 3),
            r.top() + h34 );
        polygon += QPointF(r.left() + (w / 2), r.bottom());
        polygon += QPointF(r.left() + (side + (w - 2 * side) / 3),
            r.top() + h34 );
        polygon += QPointF(r.left() + side, r.top() + h34);
        polygon += QPointF(r.left() + (side + (w / 2 - side) / 3),
            r.top() + h2 );
        polygon += QPointF(r.left() + side, r.top() + h4);
        polygon += QPointF(r.left() + (side + (w - 2 * side) / 3),
            r.top() + h4 );

        QwtPainter::drawPolygon(painter, polygon);
    }
}

static inline void qwtDrawHexagonSymbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    const QSize size = symbol.size();

    painter->setBrush(symbol.brush());
    painter->setPen(symbol.pen());

    QRectF r(0, 0, size.width(), size.height());

    for ( int i = 0; i < numPoints; i++ )
    {
        r.moveCenter(points[i]);
        const double cos30 = 0.866025; // cos(30°)

        const QPointF c = r.center();
        const double side = r.width() * (1.0 - cos30) / 2.0;  
        const double h4 = r.height() / 4;
        const double h34 = (r.height() * 3) / 4;

        QPolygonF polygon;
        polygon += QPointF(c.x(), r.top());
        polygon += QPointF(r.right() - side, r.top() + h4);
        polygon += QPointF(r.right() - side, r.top() + h34);
        polygon += QPointF(c.x(), r.bottom());
        polygon += QPointF(r.left() + side, r.top() + h34);
        polygon += QPointF(r.left() + side, r.top() + h4);

        QwtPainter::drawPolygon(painter, polygon);
    }
}

class QwtSymbol::PrivateData
{
public:
    PrivateData(QwtSymbol::Style st, const QBrush &br,
            const QPen &pn, const QSize &sz):
        style(st),
        size(sz),
        brush(br),
        pen(pn)
    {
    }

    bool operator==(const PrivateData &other) const
    {
        return (style == other.style) 
            && (size == other.size) 
            && (brush == other.brush) 
            && (pen == other.pen);
    }


    Style style;
    QSize size;
    QBrush brush;
    QPen pen;
};

/*!
  Default Constructor
  \param style Symbol Style

  The symbol is constructed with gray interior,
  black outline with zero width, no size and style 'NoSymbol'.
*/
QwtSymbol::QwtSymbol(Style style) 
{
    d_data = new PrivateData(style, QBrush(Qt::gray), 
        QPen(Qt::black), QSize(0.0, 0.0));
}

/*!
  \brief Constructor
  \param style Symbol Style
  \param brush brush to fill the interior
  \param pen outline pen 
  \param size size
*/
QwtSymbol::QwtSymbol(QwtSymbol::Style style, const QBrush &brush, 
    const QPen &pen, const QSize &size) 
{
    d_data = new PrivateData(style, brush, pen, size);
}

//! Destructor
QwtSymbol::~QwtSymbol()
{
    delete d_data;
}

/*!
  \brief Specify the symbol's size

  If the 'h' parameter is left out or less than 0,
  and the 'w' parameter is greater than or equal to 0,
  the symbol size will be set to (w,w).
  \param width Width
  \param height Height (defaults to -1.0)
*/
void QwtSymbol::setSize(int width, int height)
{
    if ((width >= 0) && (height < 0)) 
        height = width;

    d_data->size = QSize(width, height);
}

/*! 
   Set the symbol's size
   \param size Size
*/
void QwtSymbol::setSize(const QSize &size)
{
    if (size.isValid()) 
        d_data->size = size;
}

//! Return Size
const QSize& QwtSymbol::size() const
{ 
    return d_data->size;
}   

/*!
  \brief Assign a brush

  The brush is used to draw the interior of the symbol.
  \param brush Brush
*/
void QwtSymbol::setBrush(const QBrush &brush)
{
    d_data->brush = brush;
}

//! Return Brush
const QBrush& QwtSymbol::brush() const
{
    return d_data->brush;
}

/*!
  Assign a pen

  The pen is used to draw the symbol's outline.

  The width of non cosmetic pens is scaled according to the resolution
  of the paint device.

  \param pen Pen
  \sa pen(), setBrush(), QwtPainter::scaledPen()
*/
void QwtSymbol::setPen(const QPen &pen)
{
    d_data->pen = pen;
}

//! Return Pen
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
void QwtSymbol::setColor(const QColor &color)
{
    switch(d_data->style)
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
            d_data->brush.setColor(color);
            break;
        }
        case QwtSymbol::Cross:
        case QwtSymbol::XCross:
        case QwtSymbol::HLine:
        case QwtSymbol::VLine:
        case QwtSymbol::Star1:
        {
            d_data->pen.setColor(color);
            break;
        }
        default:
        {
            d_data->brush.setColor(color);
            d_data->pen.setColor(color);
        }
    }
}

void QwtSymbol::drawSymbols(QPainter *painter, 
    const QPointF *points, int numPoints) const
{
    if ( numPoints <= 0 )
        return;

    painter->save();

    switch(d_data->style)
    {
        case QwtSymbol::Ellipse:
        {
            qwtDrawEllipseSymbols(painter, points, numPoints, *this);
            break;
        }
        case QwtSymbol::Rect:
        {
            qwtDrawRectSymbols(painter, points, numPoints, *this);
            break;
        }
        case QwtSymbol::Diamond:
        {
            qwtDrawDiamondSymbols(painter, points, numPoints, *this);
            break;
        }
        case QwtSymbol::Cross:
        {
            qwtDrawCrossSymbols(painter, points, numPoints, *this);
            break;
        }
        case QwtSymbol::XCross:
        {
            qwtDrawXCrossSymbols(painter, points, numPoints, *this);
            break;
        }
        case QwtSymbol::Triangle:
        case QwtSymbol::UTriangle:
        {
            qwtDrawUTriangleSymbols(painter, points, numPoints, *this);
            break;
        }
        case QwtSymbol::DTriangle:
        {
            qwtDrawDTriangleSymbols(painter, points, numPoints, *this);
            break;
        }
        case QwtSymbol::RTriangle:
        {
            qwtDrawRTriangleSymbols(painter, points, numPoints, *this);
            break;
        }
        case QwtSymbol::LTriangle:
        {
            qwtDrawLTriangleSymbols(painter, points, numPoints, *this);
            break;
        }
        case QwtSymbol::HLine:
        {
            qwtDrawHLineSymbols(painter, points, numPoints, *this);
            break;
        }
        case QwtSymbol::VLine:
        {
            qwtDrawVLineSymbols(painter, points, numPoints, *this);
            break;
        }
        case QwtSymbol::Star1:
        {
            qwtDrawStar1Symbols(painter, points, numPoints, *this);
            break;
        }
        case QwtSymbol::Star2:
        {
            qwtDrawStar2Symbols(painter, points, numPoints, *this);
            break;
        }
        case QwtSymbol::Hexagon:
        {
            qwtDrawHexagonSymbols(painter, points, numPoints, *this);
            break;
        }
        default:;
    }
#if 1
    painter->setPen(Qt::red);
    painter->setRenderHint(QPainter::Antialiasing, false);

    for ( int i = 0; i < numPoints; i++ )
    {
        const QPointF &pos = points[i];
        if ( QwtPainter::isAligning(painter) )
            painter->drawPoint( QPointF(qCeil(pos.x()), qCeil(pos.y())) );
        else
            painter->drawPoint( pos );
    }
#endif
    painter->restore();
}

/*!
  \brief Specify the symbol style

  The following styles are defined:<dl>
  <dt>NoSymbol<dd>No Style. The symbol cannot be drawn.
  <dt>Ellipse<dd>Ellipse or circle
  <dt>Rect<dd>Rectangle
  <dt>Diamond<dd>Diamond
  <dt>Triangle<dd>Triangle pointing upwards
  <dt>DTriangle<dd>Triangle pointing downwards
  <dt>UTriangle<dd>Triangle pointing upwards
  <dt>LTriangle<dd>Triangle pointing left
  <dt>RTriangle<dd>Triangle pointing right
  <dt>Cross<dd>Cross (+)
  <dt>XCross<dd>Diagonal cross (X)
  <dt>HLine<dd>Horizontal line
  <dt>VLine<dd>Vertical line
  <dt>Star1<dd>X combined with +
  <dt>Star2<dd>Six-pointed star
  <dt>Hexagon<dd>Hexagon</dl>

  \param style Style
*/
void QwtSymbol::setStyle(QwtSymbol::Style style)
{
    d_data->style = style;
}

//! Return Style
QwtSymbol::Style QwtSymbol::style() const
{
    return d_data->style;
}
//! == operator
bool QwtSymbol::operator==(const QwtSymbol &other) const
{
    return *d_data == *other.d_data;
}

//! != operator
bool QwtSymbol::operator!=(const QwtSymbol &other) const
{
    return !(*this == other);
}
