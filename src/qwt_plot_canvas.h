/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_CANVAS_H
#define QWT_PLOT_CANVAS_H

#include "qwt_global.h"
#include <qframe.h>
#include <qpen.h>

class QwtPlot;
class QPixmap;

/*!
  \brief Canvas of a QwtPlot.
  \sa QwtPlot
*/
class QWT_EXPORT QwtPlotCanvas : public QFrame
{
    Q_OBJECT

public:

    /*!
      \brief Paint attributes

      The default setting enables BackingStore and Opaque.

      \sa setPaintAttribute(), testPaintAttribute()
     */
    enum PaintAttribute
    {
        /*!
          \brief Paint double buffered reusing the content 
                 of the pixmap buffer when possible. 

          Using a backing store might improve the performance
          significantly, when workin with widget overlays ( like rubberbands ).
          Disabling the cache might improve the performance for
          incremental paints (using QwtPlotDirectPainter ).

          \sa backingStore(), invalidateBackingStore()
         */
        BackingStore = 1,

        /*!
          \brief Try to fill the complete contents rectangle
                 of the plot canvas

          When using styled backgrounds Qt assumes, that the
          canvas doesn't fill its area completely 
          ( f.e because of rounded borders ) and fills the area
          below the canvas. When this is done with gradients it might
          result in a serious performance bottleneck - depending on the size.

          When the Opaque attribute is enabled the canvas tries to
          identify the gaps with some heuristics and to fill those only. 

          \warning Will not work for semitransparent backgrounds 
         */
        Opaque       = 2
    };

    /*!
      \brief Focus indicator

      - NoFocusIndicator\n
        Don't paint a focus indicator

      - CanvasFocusIndicator\n
        The focus is related to the complete canvas.
        Paint the focus indicator using paintFocus()

      - ItemFocusIndicator\n
        The focus is related to an item (curve, point, ...) on
        the canvas. It is up to the application to display a
        focus indication using f.e. highlighting.

      \sa setFocusIndicator(), focusIndicator(), paintFocus()
    */

    enum FocusIndicator
    {
        NoFocusIndicator,
        CanvasFocusIndicator,
        ItemFocusIndicator
    };

    explicit QwtPlotCanvas( QwtPlot * );
    virtual ~QwtPlotCanvas();

    QwtPlot *plot();
    const QwtPlot *plot() const;

    void setFocusIndicator( FocusIndicator );
    FocusIndicator focusIndicator() const;

    void setPaintAttribute( PaintAttribute, bool on = true );
    bool testPaintAttribute( PaintAttribute ) const;

    QPixmap *backingStore();
    const QPixmap *backingStore() const;
    void invalidateBackingStore();

    void replot();

    virtual bool event( QEvent * );

protected:
    virtual void paintEvent( QPaintEvent * );
    virtual void resizeEvent( QResizeEvent * );
    virtual void changeEvent( QEvent * );

    virtual void drawBackground( QPainter * );
    virtual void drawContents( QPainter * );
    virtual void drawFocusIndicator( QPainter * );

    void drawCanvas( QPainter *painter = NULL );
    void updateCanvasClip();

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
