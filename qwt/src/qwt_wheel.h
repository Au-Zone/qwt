/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_WHEEL_H
#define QWT_WHEEL_H

#include "qwt_global.h"
#include <qwidget.h>

/*!
  \brief The Wheel Widget

  The wheel widget can be used to change values over a very large range
  in very small steps. Using the setMass member, it can be configured
  as a flywheel.

  \sa The radio example.
*/
class QWT_EXPORT QwtWheel: public QWidget
{
    Q_OBJECT

    Q_PROPERTY( Qt::Orientation orientation
                READ orientation WRITE setOrientation )

    Q_PROPERTY( double value READ value WRITE setValue )
    Q_PROPERTY( double minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( double maximum READ maximum WRITE setMaximum )

    Q_PROPERTY( double singleStep READ singleStep WRITE setSingleStep )
    Q_PROPERTY( int pageSize READ pageSize WRITE setPageSize )

    Q_PROPERTY( bool tracking READ isTracking WRITE setTracking )
    Q_PROPERTY( bool wrapping READ wrapping WRITE setWrapping )

    Q_PROPERTY( double mass READ mass WRITE setMass )

    Q_PROPERTY( double totalAngle READ totalAngle WRITE setTotalAngle )
    Q_PROPERTY( double viewAngle READ viewAngle WRITE setViewAngle )
    Q_PROPERTY( int tickCnt READ tickCnt WRITE setTickCnt )
    Q_PROPERTY( int wheelWidth READ wheelWidth WRITE setWheelWidth )
    Q_PROPERTY( int borderWidth READ borderWidth WRITE setBorderWidth )
    Q_PROPERTY( int wheelBorderWidth READ wheelBorderWidth WRITE setWheelBorderWidth )

public:
    explicit QwtWheel( QWidget *parent = NULL );
    virtual ~QwtWheel();

    double value() const;

    void setOrientation( Qt::Orientation );
    Qt::Orientation orientation() const;

    double totalAngle() const;
    double viewAngle() const;

    void setTickCnt( int );
    int tickCnt() const;

    void setWheelWidth( int );
    int wheelWidth() const;

    void setWheelBorderWidth( int );
    int wheelBorderWidth() const;

    void setBorderWidth( int );
    int borderWidth() const;


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

    void setUpdateInterval( int );
    int updateInterval() const;

    void setTracking( bool enable );
    bool isTracking() const;

    void setMass( double val );
    double mass() const;

public Q_SLOTS:
    void setValue( double val );
    void setTotalAngle ( double );
    void setViewAngle( double );

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
    void wheelPressed();

    /*!
      This signal is emitted when the user releases the
      movable part of the slider.
    */

    void wheelReleased();
    /*!
      This signal is emitted when the user moves the
      slider with the mouse.
      \param value new value
    */
    void wheelMoved( double value );

protected:
    virtual void paintEvent( QPaintEvent * );
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void keyPressEvent( QKeyEvent * );
    virtual void wheelEvent( QWheelEvent * );
    virtual void timerEvent( QTimerEvent * );

    bool setNewValue( double value );
    bool updateValue( double value );
    void stopFlying();

    QRect wheelRect() const;

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    virtual void drawTicks( QPainter *, const QRectF & );
    virtual void drawWheelBackground( QPainter *, const QRectF & );

    virtual double valueAt( const QPoint & );

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
