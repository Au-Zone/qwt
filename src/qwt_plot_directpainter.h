/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_DIRECT_PAINTER_H
#define QWT_PLOT_DIRECT_PAINTER_H

#include "qwt_global.h"
#include <qobject.h>

class QRegion;
class QwtPlotAbstractSeriesItem;

/*!
    \brief Painter object trying to paint incrementally

    Often applications want to display samples while they are
    collected. When there are too many samples complete replots
    will be expensive to be processed in a collection cycle.

    QwtPlotDirectPainter offers an API to paint
    subsets ( f.e all additions points ) without erasing/repainting
    the plot canvas.

    On certain environments it might be important to calculate a proper
    clip region before painting. F.e. for Qt Embedded only the clipped part
    of the backing store will be copied to a ( maybe unaccelerated ) 
    frame buffer.

    \warning Incremental painting will only help when no replot is triggered
             by another operation ( like changing scales ) and nothing needs
             to be erased.
*/
class QWT_EXPORT QwtPlotDirectPainter: public QObject
{
public:
    /*!
      - AtomicPainter\n
        Initializing a QPainter is an expensive operation.
        When AtomicPainter is set each call of drawSeries() opens/closes
        a temporary QPainter. Otherwise QwtPlotDirectPainter tries to
        use the same QPainter as long as possible.

      - FullRepaint\n
        When FullRepaint is set the plot canvas is explicitely repainted
        after the samples have been rendered.

      - CopyCanvasCache\n
        When QwtPlotCanvas::PaintCached is enabled the painter
        has to paint to the cache and the widget. In certain 
        situations/environments it might be faster to paint to 
        the cache only and then copy the cache to the canvas.
        This flag can also be useful for settings, where Qt fills the
        the clip region with the widget background.

      \sa setAttribute(), testAttribute(), drawSeries()
    */
    enum Attribute
    {
        AtomicPainter = 1,
        FullRepaint = 2,
        CopyCanvasCache = 4
    };

    QwtPlotDirectPainter( QObject *parent = NULL );
    virtual ~QwtPlotDirectPainter();

    void setAttribute( Attribute, bool on );
    bool testAttribute( Attribute ) const;

    void setClipping( bool );
    bool hasClipping() const;

    void setClipRegion( const QRegion & );
    QRegion clipRegion() const;

    void drawSeries( QwtPlotAbstractSeriesItem *, int from, int to );
    void reset();

    virtual bool eventFilter( QObject *, QEvent * );

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
