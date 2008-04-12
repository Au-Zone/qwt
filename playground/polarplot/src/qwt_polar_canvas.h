/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GPL License, Version 2.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_POLAR_CANVAS_H
#define QWT_POLAR_CANVAS_H 1

#include <qframe.h>
#include "qwt_global.h"
#include "qwt_double_rect.h"

class QPainter;
class QwtPolarPlot;

/*!
  Canvas of a QwtPolarPlot.       

  \note In opposite to QwtPlot all axes are painted on the canvas.
  \sa QwtPolarPlot
*/
class QWT_EXPORT QwtPolarCanvas: public QFrame
{
    Q_OBJECT

public:
    /*!
      \brief Paint attributes

      - PaintCached\n
        Paint double buffered and reuse the content of the pixmap buffer
        for some spontaneous repaints that happen when a plot gets unhidden,
        deiconified or changes the focus.

      The default setting enables PaintCached
      \sa setPaintAttribute(), testPaintAttribute(), paintCache()
     */

    enum PaintAttribute
    {
        PaintCached = 1
    };

    explicit QwtPolarCanvas(QwtPolarPlot *);
    virtual ~QwtPolarCanvas();

    QwtPolarPlot *plot();
    const QwtPolarPlot *plot() const;

    void setPaintAttribute(PaintAttribute, bool on = true);
    bool testPaintAttribute(PaintAttribute) const;

    QPixmap *paintCache();
    const QPixmap *paintCache() const;
    void invalidatePaintCache();

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void drawContents(QPainter *);

    void drawCanvas(QPainter *, const QwtDoubleRect &);

    class PrivateData;
    PrivateData *d_data;
};

#endif
