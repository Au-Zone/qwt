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
#include <qwidget.h>

/*!
  \brief An abstract base class for slider widgets

  QwtAbstractSlider is a base class for
  slider widgets. It handles mouse events
  and updates the slider's value accordingly. Derived classes
  only have to implement the getValue() and
  getScrollMode() members, and should react to a
  valueChange(), which normally requires repainting.
*/

class QWT_EXPORT QwtAbstractSlider : public QWidget
{
    Q_OBJECT

    Q_PROPERTY( double value READ value WRITE setValue )
    Q_PROPERTY( double minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( double maximum READ maximum WRITE setMaximum )

    Q_PROPERTY( int pageSize READ pageSize WRITE setPageSize )

    Q_PROPERTY( bool readOnly READ isReadOnly WRITE setReadOnly )
    Q_PROPERTY( bool tracking READ isTracking WRITE setTracking )
    Q_PROPERTY( bool wrapping READ wrapping WRITE setWrapping )

    Q_PROPERTY( double mass READ mass WRITE setMass )
    Q_PROPERTY( Qt::Orientation orientation
                READ orientation WRITE setOrientation )

public:
    /*!
      Scroll mode
      \sa getScrollMode()
     */
    enum ScrollMode
    {
        //! Scrolling switched off. Don't change the value.
        ScrNone,

        /*!
          Change the value while the user keeps the
          button pressed and moves the mouse.
         */
        ScrMouse,

        /*!
          Automatic scrolling. Increment the value in the specified direction 
          as long as the user keeps the button pressed.
         */
        ScrTimer,

        ScrDirect,

        //! Automatic scrolling. Same as ScrTimer, but increment by page size.
        ScrPage
    };

    explicit QwtAbstractSlider( Qt::Orientation, QWidget *parent = NULL );
    virtual ~QwtAbstractSlider();

    void setValid( bool );
    bool isValid() const;

    double value() const;

    void setWrapping( bool tf );
    bool wrapping() const;

    void setSingleStep( double );
    double singleStep() const;

    void setRange( double vmin, double vmax );

	void setMinimum( double min );
    double minimum() const;

	void setMaximum( double max );
    double maximum() const;

    void setPageSize( int );
    int pageSize() const;

    void setUpdateTime( int t );
    void stopMoving();

    void setTracking( bool enable );
    bool isTracking() const;

    virtual void setMass( double val );
    virtual double mass() const;

    virtual void setOrientation( Qt::Orientation o );
    Qt::Orientation orientation() const;

    void setReadOnly( bool );
    bool isReadOnly() const;

public Q_SLOTS:
    void setValue( double val );
    void incValue( int steps );
    void incPages( int pages );

Q_SIGNALS:

    /*!
      \brief Notify a change of value.

      In the default setting
      (tracking enabled), this signal will be emitted every
      time the value changes ( see setTracking() ).
      \param value new value
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
    virtual void timerEvent( QTimerEvent *e );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e );
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void wheelEvent( QWheelEvent *e );

    /*!
      \brief Determine the value corresponding to a specified poind

      This is an abstract virtual function which is called when
      the user presses or releases a mouse button or moves the
      mouse. It has to be implemented by the derived class.
      \param p point
    */
    virtual double getValue( const QPoint & p ) = 0;

    /*!
      \brief Determine what to do when the user presses a mouse button.

      This function is abstract and has to be implemented by derived classes.
      It is called on a mousePress event. The derived class can determine
      what should happen next in dependence of the position where the mouse
      was pressed by returning scrolling mode and direction. 

      \param pos point where the mouse was pressed
      \retval scrollMode The scrolling mode
      \retval direction  direction: 1, 0, or -1.
    */
    virtual void getScrollMode( const QPoint &pos,
        ScrollMode &scrollMode, int &direction ) const = 0;

    void setMouseOffset( double );
    double mouseOffset() const;

    int scrollMode() const;

    virtual void valueChange();
    virtual void rangeChange();


private:
    void setNewValue( double value );

    void setPosition( const QPoint & );
    void buttonReleased();

    class PrivateData;
    PrivateData *d_data;
};

#endif
