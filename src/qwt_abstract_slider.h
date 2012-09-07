/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_ABSTRACT_SLIDER_H
#define QWT_ABSTRACT_SLIDER_H

#include "qwt_global.h"
#include "qwt_abstract_scale.h"

/*!
  \brief An abstract base class for slider widgets

  QwtAbstractSlider is a base class for slider widgets. 
  It handles mouse events and updates the slider's value accordingly. 
  Derived classes only have to implement the valueAt() and
  getScrollMode() members, and should react to a
  valueChange(), which normally requires repainting.
*/

class QWT_EXPORT QwtAbstractSlider: public QwtAbstractScale
{
    Q_OBJECT

    Q_PROPERTY( double value READ value WRITE setValue )

    Q_PROPERTY( uint totalSteps READ totalSteps WRITE setTotalSteps )
    Q_PROPERTY( uint singleSteps READ singleSteps WRITE setSingleSteps )
    Q_PROPERTY( uint pageSteps READ pageSteps WRITE setPageSteps )
    Q_PROPERTY( bool stepAlignment READ stepAlignment WRITE setStepAlignment )

    Q_PROPERTY( bool readOnly READ isReadOnly WRITE setReadOnly )
    Q_PROPERTY( bool tracking READ isTracking WRITE setTracking )
    Q_PROPERTY( bool wrapping READ wrapping WRITE setWrapping )

public:
    explicit QwtAbstractSlider( QWidget *parent = NULL );
    virtual ~QwtAbstractSlider();

    void setValid( bool );
    bool isValid() const;

    double value() const;

    void setWrapping( bool tf );
    bool wrapping() const;

    void setTotalSteps( uint );
    uint totalSteps() const;

    void setSingleSteps( uint );
    uint singleSteps() const;

    void setPageSteps( uint );
    uint pageSteps() const;

    void setStepAlignment( bool on ); 
    bool stepAlignment() const;

    void setTracking( bool enable );
    bool isTracking() const;

    void setReadOnly( bool );
    bool isReadOnly() const;

public Q_SLOTS:
    void setValue( double val );

Q_SIGNALS:

   /*!
      \brief Notify a change of value translated into 
             scale coordinates.

      \param value New value
    */
    void scaleValueChanged( double value );

    /*!
      \brief Notify a change of value.

      In the default setting
      (tracking enabled), this signal will be emitted every
      time the value changes ( see setTracking() ).

      \param value New value
    */
    void valueChanged( double value );

    /*!
      This signal is emitted when the user presses the
      movable part of the slider (start ScrMouse Mode).
    */
    void sliderPressed();

    /*!
      This signal is emitted when the user releases the
      movable part of the slider.
    */

    void sliderReleased();
    /*!
      This signal is emitted when the user moves the
      slider with the mouse.
      \param value new value
    */
    void sliderMoved( double value );

protected:
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void keyPressEvent( QKeyEvent * );
    virtual void wheelEvent( QWheelEvent * );

    /*!
      \brief Determine the value corresponding to a specified poind

      This is an abstract virtual function which is called when
      the user presses or releases a mouse button or moves the
      mouse. It has to be implemented by the derived class.
      \param p point
    */
    virtual double valueAt( const QPoint & ) = 0;

    /*!
      \brief Determine what to do when the user presses a mouse button.

      This function is abstract and has to be implemented by derived classes.
      It is called on a mousePress and mouseMove events. 

      \param pos point where the mouse was pressed
      \param initial True for press and false for move events

      \retval True, when pos is a valid scroll position
    */
    virtual bool isScrollPosition( const QPoint &pos ) const = 0;

    void setMouseOffset( double );
    double mouseOffset() const;

    void incrementValue( int numSteps );

    virtual void scaleChange();

protected:
    double incrementedValue( 
        double value, int stepCount ) const;

private:
    double alignedValue( double ) const;
    double boundedValue( double ) const;

    class PrivateData;
    PrivateData *d_data;
};

#endif
