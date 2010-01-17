/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PAINTER_H
#define QWT_PAINTER_H

#include "qwt_global.h"

#include <qpoint.h>
#include <qrect.h>
#include <qpen.h>

class QPainter;
class QBrush;
class QColor;
class QWidget;
class QwtScaleMap;
class QwtColorMap;
class QwtDoubleInterval;

class QPalette;
class QTextDocument;

/*!
  \brief A collection of QPainter workarounds
*/

class QWT_EXPORT QwtPainter
{
public:
    static void setPolylineSplitting(bool);
    static bool polylineSplitting();

    static void setClipRect(QPainter *, const QRectF &);

    static void drawText(QPainter *, double x, double y, 
        const QString &);
    static void drawText(QPainter *, const QPointF &, 
        const QString &);
    static void drawText(QPainter *, double x, double y, double w, double h, 
        int flags, const QString &);
    static void drawText(QPainter *, const QRectF &, 
        int flags, const QString &);

#ifndef QT_NO_RICHTEXT
    static void drawSimpleRichText(QPainter *, const QRectF &,
        int flags, QTextDocument &);
#endif

    static void drawRect(QPainter *, double x, double y, double w, double h);
    static void drawRect(QPainter *, const QRectF &rect);
    static void fillRect(QPainter *, const QRectF &, const QBrush &); 

    static void drawEllipse(QPainter *, const QRectF &);
    static void drawPie(QPainter *, const QRectF & r, int a, int alen);

    static void drawLine(QPainter *, double x1, double y1, double x2, double y2);
    static void drawLine(QPainter *, const QPointF &p1, const QPointF &p2);
    static void drawLine(QPainter *, const QLineF &);

    static void drawPolygon(QPainter *, const QPolygonF &pa);
    static void drawPolyline(QPainter *, const QPolygonF &pa);
    static void drawPoint(QPainter *, double x, double y);

    static void drawRoundFrame(QPainter *, const QRect &,
        int width, const QPalette &, bool sunken);
    static void drawFocusRect(QPainter *, QWidget *);
    static void drawFocusRect(QPainter *, QWidget *, const QRect &);

    static void drawColorBar(QPainter *painter, 
        const QwtColorMap &, const QwtDoubleInterval &,
        const QwtScaleMap &, Qt::Orientation, const QRectF &);

private:
    static void drawColoredArc(QPainter *, const QRect &,
        int peak, int arc, int intervall, const QColor &c1, const QColor &c2);

    static bool d_polylineSplitting;
};

//!  Wrapper for QPainter::drawLine()
inline void QwtPainter::drawLine(QPainter *painter,
    double x1, double y1, double x2, double y2)
{
	painter->drawLine(QPointF(x1, y1), QPointF(x2, y2));
}

//!  Wrapper for QPainter::drawLine()
inline void QwtPainter::drawLine(QPainter *painter, const QLineF &line)
{
	painter->drawLine(line.p1(), line.p2());
}

/*!
  Returns whether line splitting for the raster paint engine is enabled. 
  \sa setPolylineSplitting()
*/
inline bool QwtPainter::polylineSplitting()
{
    return d_polylineSplitting;
}

#endif
