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

class QwtIntervalSymbol::PrivateData
{
public:
    PrivateData():
        style(QwtIntervalSymbol::NoSymbol),
        width(5)
    {
    }

    QwtIntervalSymbol::Style style;
    int width;

    QPen pen;
    QBrush brush;
};

QwtIntervalSymbol::QwtIntervalSymbol(Style style) 
{
    d_data = new PrivateData();
    d_data->style = style;
}

QwtIntervalSymbol::~QwtIntervalSymbol()
{
    delete d_data;
}

QwtIntervalSymbol *QwtIntervalSymbol::clone() const
{
    QwtIntervalSymbol *other = new QwtIntervalSymbol;
    *other->d_data = *d_data;

    return other;
}

//! == operator
bool QwtIntervalSymbol::operator==(const QwtIntervalSymbol &other) const
{
    return d_data->style == other.d_data->style &&
        d_data->width == other.d_data->width &&
        d_data->pen == other.d_data->pen &&
        d_data->brush == other.d_data->brush;
}

//! != operator
bool QwtIntervalSymbol::operator!=(const QwtIntervalSymbol &other) const
{
    return !(*this == other);
}

void QwtIntervalSymbol::setStyle(Style style)
{
    d_data->style = style;
}

QwtIntervalSymbol::Style QwtIntervalSymbol::style() const
{
    return d_data->style;
}

void QwtIntervalSymbol::setWidth(int width)
{
    d_data->width = width;
}

int QwtIntervalSymbol::width() const
{
    return d_data->width;
}

void QwtIntervalSymbol::setBrush(const QBrush &brush)
{
    d_data->brush = brush;
}

const QBrush& QwtIntervalSymbol::brush() const
{
    return d_data->brush;
}

void QwtIntervalSymbol::setPen(const QPen &pen)
{
    d_data->pen = pen;
}

const QPen& QwtIntervalSymbol::pen() const
{
    return d_data->pen;
}

void QwtIntervalSymbol::draw(QPainter *painter, 
        const QPointF &from, const QPointF &to) const
{
    const int pw = qMax(painter->pen().width(), 1);

    switch(d_data->style)
    {
        case QwtIntervalSymbol::Bar:
        {
            QwtPainter::drawLine(painter, from, to);
            if ( d_data->width > pw )
            {
                if ( from.y() == to.y() )
                {
                    const int sw = d_data->width;

                    const int y = from.y() - sw / 2;
                    QwtPainter::drawLine(painter,
                        from.x(), y, from.x(), y + sw);
                    QwtPainter::drawLine(painter,
                        to.x(), y, to.x(), y + sw);
                }
                else if ( from.x() == to.x() )
                {
                    const int sw = d_data->width;

                    const int x = from.x() - sw / 2;
                    QwtPainter::drawLine(painter,
                        x, from.y(), x + sw, from.y());
                    QwtPainter::drawLine(painter,
                        x, to.y(), x + sw, to.y());
                }
                else    
                {
                    const int sw = d_data->width;

                    const double dx = to.x() - from.x();
                    const double dy = to.y() - from.y();
                    const double angle = ::atan2(dy, dx) + M_PI_2;
                    double dw2 = sw / 2.0;

                    const int cx = qRound(::cos(angle) * dw2);
                    const int sy = qRound(::sin(angle) * dw2);

                    QwtPainter::drawLine(painter, 
                        from.x() - cx, from.y() - sy,
                        from.x() + cx, from.y() + sy);
                    QwtPainter::drawLine(painter, 
                        to.x() - cx, to.y() - sy,
                        to.x() + cx, to.y() + sy);
                }
            }
            break;
        }
        case QwtIntervalSymbol::Box:
        {
            if ( d_data->width <= pw )
            {
                QwtPainter::drawLine(painter, from, to);
            }
            else
            {
                if ( from.y() == to.y() )
                {
                    const int sw = d_data->width;

                    const int y = from.y() - d_data->width / 2;
                    QwtPainter::drawRect(painter,
                        from.x(), y, to.x() - from.x(),  sw);
                }
                else if ( from.x() == to.x() )
                {
                    const int sw = d_data->width;

                    const int x = from.x() - d_data->width / 2;
                    QwtPainter::drawRect(painter,
                        x, from.y(), sw, to.y() - from.y() );
                }
                else
                {
                    const int sw = d_data->width;

                    const double dx = to.x() - from.x();
                    const double dy = to.y() - from.y();
                    const double angle = ::atan2(dy, dx) + M_PI_2;
                    double dw2 = sw / 2.0;

                    const int cx = qRound(::cos(angle) * dw2);
                    const int sy = qRound(::sin(angle) * dw2);

                    QPolygon polygon(4);
                    polygon.setPoint(0, from.x() - cx, from.y() - sy);
                    polygon.setPoint(1, from.x() + cx, from.y() + sy);
                    polygon.setPoint(2, to.x() + cx, to.y() + sy);
                    polygon.setPoint(3, to.x() - cx, to.y() - sy);

                    QwtPainter::drawPolygon(painter, polygon);
                }
            }
            break;
        }
        default:;
    }
}
