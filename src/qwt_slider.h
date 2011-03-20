/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SLIDER_H
#define QWT_SLIDER_H

#include "qwt_global.h"
#include "qwt_abstract_scale.h"
#include "qwt_abstract_slider.h"

class QwtScaleDraw;

/*!
  \brief The Slider Widget

  QwtSlider is a slider widget which operates on an interval
  of type double. QwtSlider supports different layouts as
  well as a scale.

  \image html sliders.png

  \sa QwtAbstractSlider and QwtAbstractScale for the descriptions
      of the inherited members.
*/

class QWT_EXPORT QwtSlider : public QwtAbstractSlider, public QwtAbstractScale
{
    Q_OBJECT
    Q_ENUMS( ScalePos )
    Q_ENUMS( BackgroundStyle )
    Q_PROPERTY( ScalePos scalePosition READ scalePosition
        WRITE setScalePosition )
    Q_PROPERTY( BackgroundStyles backgroundStyle 
        READ backgroundStyle WRITE setBackgroundStyle )
    Q_PROPERTY( int thumbLength READ thumbLength WRITE setThumbLength )
    Q_PROPERTY( int thumbWidth READ thumbWidth WRITE setThumbWidth )
    Q_PROPERTY( int borderWidth READ borderWidth WRITE setBorderWidth )

public:

    /*!
      Scale position. QwtSlider tries to enforce valid combinations of its
      orientation and scale position:

      - Qt::Horizonal combines with NoScale, TopScale and BottomScale
      - Qt::Vertical combines with NoScale, LeftScale and RightScale

      \sa QwtSlider()
     */
    enum ScalePos
    {
        //! The slider has no scale
        NoScale,

        //! The scale is left of the slider
        LeftScale,

        //! The scale is right of the slider
        RightScale,

        //! The scale is above of the slider
        TopScale,

        //! The scale is below of the slider
        BottomScale
    };

    /*!
      Background style.
      \sa QwtSlider()
     */
    enum BackgroundStyle
    {
        //! Trough background
        Trough = 0x01,

        //! Slot
        Slot = 0x02,
    };

    //! Background styles
    typedef QFlags<BackgroundStyle> BackgroundStyles;

    explicit QwtSlider( QWidget *parent,
        Qt::Orientation = Qt::Horizontal,
        ScalePos = NoScale, BackgroundStyles = Trough );

    virtual ~QwtSlider();

    virtual void setOrientation( Qt::Orientation );

    void setBackgroundStyle( BackgroundStyles );
    BackgroundStyles backgroundStyle() const;

    void setScalePosition( ScalePos s );
    ScalePos scalePosition() const;

    int thumbLength() const;
    int thumbWidth() const;
    int borderWidth() const;

    void setThumbLength( int l );
    void setThumbWidth( int w );
    void setBorderWidth( int bw );
    void setMargins( int x, int y );

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    void setScaleDraw( QwtScaleDraw * );
    const QwtScaleDraw *scaleDraw() const;

protected:
    virtual double getValue( const QPoint &p );
    virtual void getScrollMode( const QPoint &p,
        int &scrollMode, int &direction );

    void draw( QPainter * );
    virtual void drawSlider ( QPainter *, const QRect & );
    virtual void drawThumb( QPainter *, const QRect &, int pos );

    virtual void resizeEvent( QResizeEvent * );
    virtual void paintEvent ( QPaintEvent * );
    virtual void changeEvent( QEvent * );

    virtual void valueChange();
    virtual void rangeChange();
    virtual void scaleChange();

    void layoutSlider( bool update = true );
    int xyPosition( double v ) const;

    QwtScaleDraw *scaleDraw();

private:
    void initSlider( Qt::Orientation, ScalePos, BackgroundStyles );

    class PrivateData;
    PrivateData *d_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtSlider::BackgroundStyles );

#endif
