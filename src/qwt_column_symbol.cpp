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
#include "qwt_column_symbol.h"
#include "qwt_painter.h"

class QwtColumnSymbol::PrivateData
{
public:
    PrivateData():
        style(QwtColumnSymbol::NoSymbol)
    {
    }

    QwtColumnSymbol::Style style;

    QPen pen;
    QBrush brush;
};

QwtColumnSymbol::QwtColumnSymbol(Style style) 
{
    d_data = new PrivateData();
    d_data->style = style;
}

QwtColumnSymbol::~QwtColumnSymbol()
{
    delete d_data;
}

QwtColumnSymbol *QwtColumnSymbol::clone() const
{
    QwtColumnSymbol *other = new QwtColumnSymbol;
    *other->d_data = *d_data;

    return other;
}

//! == operator
bool QwtColumnSymbol::operator==(const QwtColumnSymbol &other) const
{
    return d_data->style == other.d_data->style &&
        d_data->pen == other.d_data->pen &&
        d_data->brush == other.d_data->brush;
}

//! != operator
bool QwtColumnSymbol::operator!=(const QwtColumnSymbol &other) const
{
    return !(*this == other);
}

void QwtColumnSymbol::setStyle(Style style)
{
    d_data->style = style;
}

QwtColumnSymbol::Style QwtColumnSymbol::style() const
{
    return d_data->style;
}

void QwtColumnSymbol::setBrush(const QBrush &brush)
{
    d_data->brush = brush;
}

const QBrush& QwtColumnSymbol::brush() const
{
    return d_data->brush;
}

void QwtColumnSymbol::setPen(const QPen &pen)
{
    d_data->pen = pen;
}

const QPen& QwtColumnSymbol::pen() const
{
    return d_data->pen;
}

void QwtColumnSymbol::draw(QPainter *painter, Qt::Orientation orientation, 
    const QRect& rect) const
{
    switch(d_data->style)
    {
        case QwtColumnSymbol::Box:
        {
            break;
        }
        default:;
    }
}
