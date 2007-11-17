/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qpainter.h>
#include <qpalette.h>
#include "qwt_math.h"
#include "qwt_text.h"
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

    QPalette palette;
    QwtText label;
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
        d_data->palette == other.d_data->palette &&
        d_data->label == other.d_data->label;
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

void QwtColumnSymbol::setPalette(const QPalette &palette)
{
    d_data->palette = palette;
}

const QPalette& QwtColumnSymbol::palette() const
{
    return d_data->palette;
}

void QwtColumnSymbol::setLabel(const QwtText &label)
{
    d_data->label = label;
}

const QwtText& QwtColumnSymbol::label() const
{
    return d_data->label;
}

void QwtColumnSymbol::draw(QPainter *painter, 
    Qt::Orientation orientation, const QRect &rect) const
{
#if QT_VERSION >= 0x040000
    const QRect r = rect.normalized();
#else
    const QRect r = rect.normalize();
#endif
    painter->save();

    switch(d_data->style)
    {
        case QwtColumnSymbol::Box:
        {
            drawBox(painter, orientation, r);
            break;
        }
        case QwtColumnSymbol::RaisedBox:
        {
            drawRaisedBox(painter, orientation, r);
            break;
        }
        default:;
    }

    painter->restore();
}

void QwtColumnSymbol::drawBox(QPainter *, 
    Qt::Orientation, const QRect &) const
{
}

void QwtColumnSymbol::drawRaisedBox(QPainter *painter, 
    Qt::Orientation, const QRect &rect) const
{
    const QColor color(painter->pen().color());

    const int factor = 125;
    const QColor light(color.light(factor));
    const QColor dark(color.dark(factor));

    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    QwtPainter::drawRect(painter, rect.x() + 1, rect.y() + 1,
        rect.width() - 2, rect.height() - 2);
    painter->setBrush(Qt::NoBrush);

    painter->setPen(QPen(light, 2));
#if QT_VERSION >= 0x040000
    QwtPainter::drawLine(painter,
        rect.left() + 1, rect.top() + 2, rect.right() + 1, rect.top() + 2);
#else
    QwtPainter::drawLine(painter,
        rect.left(), rect.top() + 2, rect.right() + 1, rect.top() + 2);
#endif

    painter->setPen(QPen(dark, 2));
#if QT_VERSION >= 0x040000 
    QwtPainter::drawLine(painter,
        rect.left() + 1, rect.bottom(), rect.right() + 1, rect.bottom());
#else
    QwtPainter::drawLine(painter,
        rect.left(), rect.bottom(), rect.right() + 1, rect.bottom());
#endif
    painter->setPen(QPen(light, 1));

#if QT_VERSION >= 0x040000
    QwtPainter::drawLine(painter,
        rect.left(), rect.top() + 1, rect.left(), rect.bottom());
    QwtPainter::drawLine(painter,
        rect.left() + 1, rect.top() + 2, rect.left() + 1, rect.bottom() - 1);
#else
    QwtPainter::drawLine(painter,
        rect.left(), rect.top() + 1, rect.left(), rect.bottom() + 1);
    QwtPainter::drawLine(painter,
        rect.left() + 1, rect.top() + 2, rect.left() + 1, rect.bottom());
#endif

    painter->setPen(QPen(dark, 1));

#if QT_VERSION >= 0x040000
    QwtPainter::drawLine(painter,
        rect.right() + 1, rect.top() + 1, rect.right() + 1, rect.bottom());
    QwtPainter::drawLine(painter,
        rect.right(), rect.top() + 2, rect.right(), rect.bottom() - 1);
#else
    QwtPainter::drawLine(painter,
        rect.right() + 1, rect.top() + 1, rect.right() + 1, rect.bottom() + 1);
    QwtPainter::drawLine(painter,
        rect.right(), rect.top() + 2, rect.right(), rect.bottom());
#endif
}
