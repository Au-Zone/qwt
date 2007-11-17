/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qpainter.h>
#include "qwt_math.h"
#include "qwt_interval_symbol.h"
#include "qwt_painter.h"

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

void QwtIntervalSymbol::draw(QPainter *painter, Qt::Orientation orientation, 
    const QRect& rect) const
{
    switch(d_data->style)
    {
        case QwtIntervalSymbol::Bar:
        {
            const int pw = qwtMax(painter->pen().width(), 1);

            if ( orientation == Qt::Vertical )
            {
                const int x = rect.center().x();
                QwtPainter::drawLine(painter, x, rect.top(), 
                    x, rect.bottom() );

                if ( rect.width() > pw )
                {
                    QwtPainter::drawLine(painter, 
                        rect.bottomLeft(), rect.bottomRight());
                    QwtPainter::drawLine(painter, 
                        rect.topLeft(), rect.topRight());
                }
            }
            else
            {
                const int y = rect.center().y();
                QwtPainter::drawLine(painter, rect.left(), y, 
                    rect.right(), y);

                if ( rect.width() > pw )
                {
                    QwtPainter::drawLine(painter, 
                        rect.bottomLeft(), rect.topLeft());
                    QwtPainter::drawLine(painter, 
                        rect.bottomRight(), rect.topRight());
                }
            }

            break;
        }
        case QwtIntervalSymbol::Box:
        {
            QwtPainter::drawRect(painter, rect);
            break;
        }
        default:;
    }
}
