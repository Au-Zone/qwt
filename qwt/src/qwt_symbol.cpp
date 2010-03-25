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

            QwtPainter::drawPolygon(painter, polygon);
        }
    }
}

static inline void qwtDrawTriangleSymbols(
    QPainter *painter, QwtTriangle::Type type, 
    const QPointF *points, int numPoints, 
    const QwtSymbol &symbol)
{
    const QSize size = symbol.size();

    QPen pen = symbol.pen();
    pen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(pen);

    painter->setBrush(symbol.brush());

    const bool doAlign = QwtPainter::isAligning(painter);

    double sw2 = 0.5 * size.width();
    double sh2 = 0.5 * size.height();

    if ( doAlign )
    {
        sw2 = qFloor(sw2);
        sh2 = qFloor(sh2);
    }

    for ( int i = 0; i < numPoints; i++ )
    {
        const QPointF &pos = points[i];

        double x = pos.x();
        double y = pos.y();

        if ( doAlign )
        {
            x = qCeil(x);
            y = qCeil(y);
        }

        const double x1 = x - sw2;
        const double x2 = x1 + size.width();
        const double y1 = y - sh2;
        const double y2 = y1 + size.height();

        QPolygonF polygon;
        switch(type)
        {
            case QwtTriangle::Left:
            {
                polygon += QPointF(x2, y1);
                polygon += QPointF(x1, y);
                polygon += QPointF(x2, y2);
                break;
            }
            case QwtTriangle::Right:
            {
                polygon += QPointF(x1, y1);
                polygon += QPointF(x2, y);
                polygon += QPointF(x1, y2);
                break;
            }
            case QwtTriangle::Up:
            {
                polygon += QPointF(x1, y2);
                polygon += QPointF(x, y1);
                polygon += QPointF(x2, y2);
                break;
            }
            case QwtTriangle::Down:
            {
                polygon += QPointF(x1, y1);
                polygon += QPointF(x, y2);
                polygon += QPointF(x2, y1);
                break;
            }
        }
        QwtPainter::drawPolygon(painter, polygon);
    }
}

static inline void qwtDrawLineSymbols(
    QPainter *painter, int orientations,
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

            if ( orientations & Qt::Horizontal )
            {
                const int x = (int)qCeil(pos.x());
                const int x1 = x - size.width() / 2;

                QwtPainter::drawLine(painter, x1, pos.y(), 
                    x1 + size.width() - 1, pos.y());
            }
            if ( orientations & Qt::Vertical )
            {
                const int y = (int)qCeil(pos.y());
                const int y1 = y - size.height() / 2;

                QwtPainter::drawLine(painter, pos.x(), y1, 
                    pos.x(), y1 + size.height() - 1);
            }
        }
    }
    else
    {
        for ( int i = 0; i < numPoints; i++ )
        {
            const QPointF &pos = points[i];

            if ( orientations & Qt::Horizontal )
            {
                const double x1 = pos.x() - 0.5 * size.width();
                QwtPainter::drawLine(painter, x1, pos.y(), 
                    x1 + size.width(), pos.y());
            }
            if ( orientations & Qt::Vertical )
            {
                const double y1 = pos.y() - 0.5 * size.height();
                QwtPainter::drawLine(painter, pos.x(), y1, 
                    pos.x(), y1 + size.height());
            }
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

    QPen pen = symbol.pen();
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(symbol.pen());

    painter->setBrush(symbol.brush());

    const double cos30 = 0.866025; // cos(30°)

    const double dy = 0.25 * size.height();
    const double dx = 0.5 * size.width() * cos30 / 3.0;

    QPolygonF star(12);
    QPointF *starPoints = star.data();

    for ( int i = 0; i < numPoints; i++ )
    {
        const QPointF pos = points[i];

        const double x1 = pos.x() - 3 * dx;
        const double x2 = x1 + dx;
        const double x3 = x1 + 2 * dx;
        const double x4 = x1 + 3 * dx;
        const double x5 = x1 + 4 * dx;
        const double x6 = x1 + 5 * dx;
        const double x7 = x1 + 6 * dx;

        const double y1 = pos.y() - 2 * dy;
        const double y2 = y1 + dy;
        const double y3 = y1 + 2 * dy;
        const double y4 = y1 + 3 * dy;
        const double y5 = y1 + 4 * dy;

        starPoints[0] = QPointF(x4, y1);
        starPoints[1] = QPointF(x5, y2 );
        starPoints[2] = QPointF(x7, y2);
        starPoints[3] = QPointF(x6, y3 );
        starPoints[4] = QPointF(x7, y4);
        starPoints[5] = QPointF(x5, y4 );
        starPoints[6] = QPointF(x4, y5);
        starPoints[7] = QPointF(x3, y4 );
        starPoints[8] = QPointF(x1, y4);
        starPoints[9] = QPointF(x2, y3 );
        starPoints[10] = QPointF(x1, y2);
        starPoints[11] = QPointF(x3, y2 );

        QwtPainter::drawPolygon(painter, star);
    }
}

static inline void qwtDrawHexagonSymbols(QPainter *painter,
    const QPointF *points, int numPoints, const QwtSymbol &symbol)
{
    painter->setBrush(symbol.brush());
    painter->setPen(symbol.pen());

    const double cos30 = 0.866025; // cos(30°)
    const double dx = 0.5 * ( symbol.size().width() - cos30 );

    const double dy = 0.25 * symbol.size().height();

    QPolygonF hexaPolygon(6);
    QPointF *hexaPoints = hexaPolygon.data();

    for ( int i = 0; i < numPoints; i++ )
    {
        const QPointF pos = points[i];

        const double x1 = pos.x() - dx;
        const double x2 = x1 + dx;
        const double x3 = x1 + 2 * dx;

        const double y1 = pos.y() - 2 * dy;
        const double y2 = y1 + dy;
        const double y3 = y1 + 3 * dy;
        const double y4 = y1 + 4 * dy;

        hexaPoints[0] = QPointF(x2, y1);
        hexaPoints[1] = QPointF(x3, y2);
        hexaPoints[2] = QPointF(x3, y3);
        hexaPoints[3] = QPointF(x2, y4);
        hexaPoints[4] = QPointF(x1, y3);
        hexaPoints[5] = QPointF(x1, y2);

        QwtPainter::drawPolygon(painter, hexaPolygon);
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
            qwtDrawLineSymbols(painter, Qt::Horizontal | Qt::Vertical,
                points, numPoints, *this);
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
            qwtDrawTriangleSymbols(painter, QwtTriangle::Up,
                points, numPoints, *this);
            break;
        }
        case QwtSymbol::DTriangle:
        {
            qwtDrawTriangleSymbols(painter, QwtTriangle::Down,
                points, numPoints, *this);
            break;
        }
        case QwtSymbol::RTriangle:
        {
            qwtDrawTriangleSymbols(painter, QwtTriangle::Right,
                points, numPoints, *this);
            break;
        }
        case QwtSymbol::LTriangle:
        {
            qwtDrawTriangleSymbols(painter, QwtTriangle::Left,
                points, numPoints, *this);
            break;
        }
        case QwtSymbol::HLine:
        {
            qwtDrawLineSymbols(painter, Qt::Horizontal,
                points, numPoints, *this);
            break;
        }
        case QwtSymbol::VLine:
        {
            qwtDrawLineSymbols(painter, Qt::Vertical,
                points, numPoints, *this);
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
#if 0
    painter->setBrush(Qt::NoBrush);
    painter->setRenderHint(QPainter::Antialiasing, false);

    for ( int i = 0; i < numPoints; i++ )
    {
        QPointF pos = points[i];
        if ( QwtPainter::isAligning(painter) )
            pos = QPointF(qCeil(pos.x()), qCeil(pos.y()));

        painter->setPen(Qt::red);
        painter->drawPoint( pos );

#if 0
        painter->setPen(Qt::blue);
        QRectF r(0, 0, d_data->size.width(), d_data->size.height());
        r.moveCenter(pos);

        painter->drawRect(r.adjusted(0, 0, -1, -1));
#endif
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
