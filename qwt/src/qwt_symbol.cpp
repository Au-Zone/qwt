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

/*!
  Default Constructor

  The symbol is constructed with gray interior,
  black outline with zero width, no size and style 'NoSymbol'.
*/
QwtSymbol::QwtSymbol(): 
    d_brush(Qt::gray), 
    d_pen(Qt::black), 
    d_size(0.0, 0.0),
    d_style(QwtSymbol::NoSymbol)
{
}

/*!
  \brief Constructor
  \param style Symbol Style
  \param brush brush to fill the interior
  \param pen outline pen 
  \param size size
*/
QwtSymbol::QwtSymbol(QwtSymbol::Style style, const QBrush &brush, 
        const QPen &pen, const QSizeF &size): 
    d_brush(brush), 
    d_pen(pen), 
    d_size(size),
    d_style(style)
{
}

//! Destructor
QwtSymbol::~QwtSymbol()
{
}

/*!
  Allocate and return a symbol with the same attributes
  \return Cloned symbol
*/
QwtSymbol *QwtSymbol::clone() const
{
    QwtSymbol *other = new QwtSymbol;
    *other = *this;

    return other;
}

/*!
  \brief Specify the symbol's size

  If the 'h' parameter is left out or less than 0,
  and the 'w' parameter is greater than or equal to 0,
  the symbol size will be set to (w,w).
  \param width Width
  \param height Height (defaults to -1.0)
*/
void QwtSymbol::setSize(double width, double height)
{
    if ((width >= 0.0) && (height < 0.0)) 
        height = width;

    d_size = QSizeF(width, height);
}

/*! 
   Set the symbol's size
   \param size Size
*/
void QwtSymbol::setSize(const QSizeF &size)
{
    if (size.isValid()) 
        d_size = size;
}

/*!
  \brief Assign a brush

  The brush is used to draw the interior of the symbol.
  \param brush Brush
*/
void QwtSymbol::setBrush(const QBrush &brush)
{
    d_brush = brush;
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
    d_pen = pen;
}

/*!
  \brief Draw the symbol at a point (x,y).
*/
void QwtSymbol::draw(QPainter *painter, double x, double y) const
{
    draw(painter, QPointF(x, y));
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
    switch(d_style)
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
            d_brush.setColor(color);
            break;
        }
        case QwtSymbol::Cross:
        case QwtSymbol::XCross:
        case QwtSymbol::HLine:
        case QwtSymbol::VLine:
        case QwtSymbol::Star1:
        {
            d_pen.setColor(color);
            break;
        }
        default:
        {
            d_brush.setColor(color);
            d_pen.setColor(color);
        }
    }
}


/*!
  \brief Draw the symbol into a bounding rectangle.

  This function assumes that the painter has been initialized with
  brush and pen before. This allows a much more performant implementation
  when painting many symbols with the same brush and pen like in curves.

  \param painter Painter
  \param r Bounding rectangle
*/
void QwtSymbol::draw(QPainter *painter, const QRectF &r) const
{
    switch(d_style)
    {
        case QwtSymbol::Ellipse:
        {
            QwtPainter::drawEllipse(painter, r);
            break;
        }
        case QwtSymbol::Rect:
        {
            QwtPainter::drawRect(painter, r);
            break;
        }
        case QwtSymbol::Diamond:
        {
            const QPointF c = r.center();

            QPolygonF polygon;
            polygon += QPointF(c.x(), r.top());
            polygon += QPointF(r.right(), c.y());
            polygon += QPointF(c.x(), r.bottom());
            polygon += QPointF(r.left(), c.y());

            QwtPainter::drawPolygon(painter, polygon);
            break;
        }
        case QwtSymbol::Cross:
        {
            const QPointF c = r.center();

            QwtPainter::drawLine(painter, c.x(), r.top(), 
                c.x(), r.bottom());
            QwtPainter::drawLine(painter, r.left(), c.y(), 
                r.right(), c.y());
            break;
        }
        case QwtSymbol::XCross:
        {
            QwtPainter::drawLine(painter, r.left(), r.top(), 
                r.right(), r.bottom());
            QwtPainter::drawLine(painter, r.left(), r.bottom(), 
                r.right(), r.top());
            break;
        }
        case QwtSymbol::Triangle:
        case QwtSymbol::UTriangle:
        {
            const QPointF c = r.center();

            QPolygonF polygon;
            polygon += QPointF(c.x(), r.top());
            polygon += QPointF(r.right(), r.bottom());
            polygon += QPointF(r.left(), r.bottom());

            QwtPainter::drawPolygon(painter, polygon);

            break;
        }
        case QwtSymbol::DTriangle:
        {
            const QPointF c = r.center();

            QPolygonF polygon;
            polygon += QPointF(r.left(), r.y());
            polygon += QPointF(r.right(), r.top());
            polygon += QPointF(c.x(), r.bottom());

            QwtPainter::drawPolygon(painter, polygon);

            break;
        }
        case QwtSymbol::RTriangle:
        {
            const QPointF c = r.center();

            QPolygonF polygon;
            polygon += QPointF(r.left(), r.top());
            polygon += QPointF(r.right(), c.y());
            polygon += QPointF(r.left(), r.bottom());

            QwtPainter::drawPolygon(painter, polygon);

            break;
        }
        case QwtSymbol::LTriangle:
        {
            const QPointF c = r.center();

            QPolygonF polygon;
            polygon += QPointF(r.right(), r.top());
            polygon += QPointF(r.left(), c.y());
            polygon += QPointF(r.right(), r.bottom());

            QwtPainter::drawPolygon(painter, polygon);

            break;
        }
        case QwtSymbol::HLine:
        {
            const QPointF c = r.center();

            QwtPainter::drawLine(painter, 
                r.left(), c.y(), r.right(), c.y());

            break;
        }
        case QwtSymbol::VLine:
        {
            const QPointF c = r.center();

            QwtPainter::drawLine(painter, 
                c.x(), r.top(), c.x(), r.bottom());

            break;
        }
        case QwtSymbol::Star1:
        {
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

            break;
        }
        case QwtSymbol::Star2:
        {
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

            break;
        }
        case QwtSymbol::Hexagon:
        {
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

            break;
        }
        default:;
    }
}

/*!
  \brief Draw the symbol at a specified point

  \param painter Painter
  \param pos Center of the symbol
*/
void QwtSymbol::draw(QPainter *painter, const QPointF &pos) const
{
    QRectF rect;
    rect.setSize(d_size);
    rect.moveCenter(pos);

    painter->setBrush(d_brush);
    painter->setPen(d_pen);
    
    draw(painter, rect);
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

  \param s style
*/
void QwtSymbol::setStyle(QwtSymbol::Style s)
{
    d_style = s;
}

//! == operator
bool QwtSymbol::operator==(const QwtSymbol &other) const
{
    return brush() == other.brush() && pen() == other.pen()
            && style() == other.style() && size() == other.size();
}

//! != operator
bool QwtSymbol::operator!=(const QwtSymbol &other) const
{
    return !(*this == other);
}
