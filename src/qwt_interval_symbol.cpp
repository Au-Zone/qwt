/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_interval_symbol.h"
#include "qwt_painter.h"
#include "qwt_math.h"
#include <qpainter.h>

#if QT_VERSION < 0x040601
#define qAtan2(y, x) ::atan2(y, x)
#endif

class QwtIntervalSymbol::PrivateData
{
public:
    PrivateData():
        style( QwtIntervalSymbol::NoSymbol ),
        width( 6 )
    {
    }

    QwtIntervalSymbol::Style style;
    int width;

    QPen pen;
    QBrush brush;
};

/*!
  Constructor

  \param style Style of the symbol
  \sa setStyle(), style(), Style
*/
QwtIntervalSymbol::QwtIntervalSymbol( Style style )
{
    d_data = new PrivateData();
    d_data->style = style;
}

//! Destructor
QwtIntervalSymbol::~QwtIntervalSymbol()
{
    delete d_data;
}

/*!
  Specify the symbol style

  \param style Style
  \sa style(), Style
*/
void QwtIntervalSymbol::setStyle( Style style )
{
    d_data->style = style;
}

/*!
  \return Current symbol style
  \sa setStyle()
*/
QwtIntervalSymbol::Style QwtIntervalSymbol::style() const
{
    return d_data->style;
}

/*!
  Specify the width of the symbol
  It is used depending on the style.

  \param width Width
  \sa width(), setStyle()
*/
void QwtIntervalSymbol::setWidth( int width )
{
    d_data->width = width;
}

/*!
  \return Width of the symbol.
  \sa setWidth(), setStyle()
*/
int QwtIntervalSymbol::width() const
{
    return d_data->width;
}

/*!
  \brief Assign a brush

  The brush is used for the Box style.

  \param brush Brush
  \sa brush()
*/
void QwtIntervalSymbol::setBrush( const QBrush &brush )
{
    d_data->brush = brush;
}

/*!
  \return Brush
  \sa setBrush()
*/
const QBrush& QwtIntervalSymbol::brush() const
{
    return d_data->brush;
}

/*!
  Assign a pen

  \param pen Pen
  \sa pen(), setBrush()
*/
void QwtIntervalSymbol::setPen( const QPen &pen )
{
    d_data->pen = pen;
}

/*!
  \return Pen
  \sa setPen(), brush()
*/
const QPen& QwtIntervalSymbol::pen() const
{
    return d_data->pen;
}

/*!
  Draw a symbol depending on its style

  \param painter Painter
  \param from Start point of the interval in target device coordinates
  \param to End point of the interval in target device coordinates

  \sa setStyle()
*/
void QwtIntervalSymbol::draw( QPainter *painter,
        const QPointF &from, const QPointF &to ) const
{
    const double pw = qMax( painter->pen().widthF(), 1.0 );

    switch ( d_data->style )
    {
        case QwtIntervalSymbol::Bar:
        {
            QwtPainter::drawLine( painter, from, to );
            if ( d_data->width > pw )
            {
                if ( from.y() == to.y() )
                {
                    const double sw = d_data->width;

                    const double y = from.y() - sw / 2;
                    QwtPainter::drawLine( painter,
                        from.x(), y, from.x(), y + sw );
                    QwtPainter::drawLine( painter,
                        to.x(), y, to.x(), y + sw );
                }
                else if ( from.x() == to.x() )
                {
                    const double sw = d_data->width;

                    const double x = from.x() - sw / 2;
                    QwtPainter::drawLine( painter,
                        x, from.y(), x + sw, from.y() );
                    QwtPainter::drawLine( painter,
                        x, to.y(), x + sw, to.y() );
                }
                else
                {
                    const double sw = d_data->width;

                    const double dx = to.x() - from.x();
                    const double dy = to.y() - from.y();
                    const double angle = qAtan2( dy, dx ) + M_PI_2;
                    double dw2 = sw / 2.0;

                    const double cx = qCos( angle ) * dw2;
                    const double sy = qSin( angle ) * dw2;

                    QwtPainter::drawLine( painter,
                        from.x() - cx, from.y() - sy,
                        from.x() + cx, from.y() + sy );
                    QwtPainter::drawLine( painter,
                        to.x() - cx, to.y() - sy,
                        to.x() + cx, to.y() + sy );
                }
            }
            break;
        }
        case QwtIntervalSymbol::Box:
        {
            if ( d_data->width <= pw )
            {
                QwtPainter::drawLine( painter, from, to );
            }
            else
            {
                if ( from.y() == to.y() )
                {
                    const double sw = d_data->width;

                    const double y = from.y() - d_data->width / 2;
                    QwtPainter::drawRect( painter,
                        from.x(), y, to.x() - from.x(),  sw );
                }
                else if ( from.x() == to.x() )
                {
                    const double sw = d_data->width;

                    const double x = from.x() - d_data->width / 2;
                    QwtPainter::drawRect( painter,
                        x, from.y(), sw, to.y() - from.y() );
                }
                else
                {
                    const double sw = d_data->width;

                    const double dx = to.x() - from.x();
                    const double dy = to.y() - from.y();
                    const double angle = qAtan2( dy, dx ) + M_PI_2;
                    double dw2 = sw / 2.0;

                    const int cx = qCos( angle ) * dw2;
                    const int sy = qSin( angle ) * dw2;

                    QPolygonF polygon;
                    polygon += QPointF( from.x() - cx, from.y() - sy );
                    polygon += QPointF( from.x() + cx, from.y() + sy );
                    polygon += QPointF( to.x() + cx, to.y() + sy );
                    polygon += QPointF( to.x() - cx, to.y() - sy );

                    QwtPainter::drawPolygon( painter, polygon );
                }
            }
            break;
        }
        default:;
    }
}
