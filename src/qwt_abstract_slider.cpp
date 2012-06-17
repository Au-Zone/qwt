/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_abstract_slider.h"
#include "qwt_math.h"
#include <qevent.h>
#include <qdatetime.h>

#if QT_VERSION < 0x040601
#define qFabs(x) ::fabs(x)
#define qExp(x) ::exp(x)
#endif

class QwtAbstractSlider::PrivateData
{
public:
    PrivateData():
        scrollMode( QwtAbstractSlider::ScrNone ),
        mouseOffset( 0.0 ),
        tracking( true ),
        timerId( 0 ),
        updateInterval( 150 ),
        mass( 0.0 ),
        readOnly( false ),
        minValue( 0.0 ),
        maxValue( 0.0 ),
        step( 1.0 ),
        pageSize( 1 ),
        isValid( false ),
        value( 0.0 ),
        exactValue( 0.0 ),
        exactPrevValue( 0.0 ),
        prevValue( 0.0 ),
        periodic( false )
    {
    }

    QwtAbstractSlider::ScrollMode scrollMode;
    double mouseOffset;
    int direction;
    bool tracking;

    int timerId;
    int updateInterval;
    int timerTick;
    QTime time;
    double speed;
    double mass;
    Qt::Orientation orientation;
    bool readOnly;

    double minValue;
    double maxValue;
    double step;
    int pageSize;

    bool isValid;
    double value;
    double exactValue;
    double exactPrevValue;
    double prevValue;

    bool periodic;
};

/*!
   \brief Constructor

   The range is initialized to [0.0, 100.0], the
   step size to 1.0, and the value to 0.0.

   \param orientation Orientation
   \param parent Parent widget
*/
QwtAbstractSlider::QwtAbstractSlider(
        Qt::Orientation orientation, QWidget *parent ):
    QWidget( parent, NULL )
{
    d_data = new QwtAbstractSlider::PrivateData;
    d_data->orientation = orientation;

    setFocusPolicy( Qt::TabFocus );
}

//! Destructor
QwtAbstractSlider::~QwtAbstractSlider()
{
    if ( d_data->timerId )
        killTimer( d_data->timerId );

    delete d_data;
}

//! Set the value to be valid/invalid
void QwtAbstractSlider::setValid( bool isValid )
{
    if ( isValid != d_data->isValid )
    {
        d_data->isValid = isValid;
        valueChange();
    }   
}   

//! Indicates if the value is valid
bool QwtAbstractSlider::isValid() const
{
    return d_data->isValid;
}   

/*!
  En/Disable read only mode

  In read only mode the slider can't be controlled by mouse
  or keyboard.

  \param readOnly Enables in case of true
  \sa isReadOnly()
*/
void QwtAbstractSlider::setReadOnly( bool readOnly )
{
    d_data->readOnly = readOnly;
    update();
}

/*!
  In read only mode the slider can't be controlled by mouse
  or keyboard.

  \return true if read only
  \sa setReadOnly()
*/
bool QwtAbstractSlider::isReadOnly() const
{
    return d_data->readOnly;
}

/*!
  \brief Set the orientation.
  \param o Orientation. Allowed values are
           Qt::Horizontal and Qt::Vertical.
*/
void QwtAbstractSlider::setOrientation( Qt::Orientation o )
{
    d_data->orientation = o;
}

/*!
  \return Orientation
  \sa setOrientation()
*/
Qt::Orientation QwtAbstractSlider::orientation() const
{
    return d_data->orientation;
}

//! Stop updating if automatic scrolling is active

void QwtAbstractSlider::stopMoving()
{
    if ( d_data->timerId )
    {
        killTimer( d_data->timerId );
        d_data->timerId = 0;
    }
}

/*!
  \brief Specify the update interval for automatic scrolling
  \param interval Update interval in milliseconds
  \sa getScrollMode()
*/
void QwtAbstractSlider::setUpdateTime( int interval )
{
    if ( interval < 50 )
        interval = 50;

    d_data->updateInterval = interval;
}


/*!
   Mouse press event handler
   \param e Mouse event
*/
void QwtAbstractSlider::mousePressEvent( QMouseEvent *e )
{
    if ( isReadOnly() )
    {
        e->ignore();
        return;
    }
    if ( !isValid() )
        return;

    const QPoint &p = e->pos();

    d_data->timerTick = 0;

    getScrollMode( p, d_data->scrollMode, d_data->direction );
    stopMoving();

    switch ( d_data->scrollMode )
    {
        case ScrPage:
        case ScrTimer:
            d_data->mouseOffset = 0;
            d_data->timerId = startTimer( qMax( 250, 2 * d_data->updateInterval ) );
            break;

        case ScrMouse:
            d_data->time.start();
            d_data->speed = 0;
            d_data->mouseOffset = getValue( p ) - value();
            Q_EMIT sliderPressed();
            break;

        default:
            d_data->mouseOffset = 0;
            d_data->direction = 0;
            break;
    }
}


//! Emits a valueChanged() signal if necessary
void QwtAbstractSlider::buttonReleased()
{
    if ( ( !d_data->tracking ) || ( value() != prevValue() ) )
        Q_EMIT valueChanged( value() );
}


/*!
   Mouse Release Event handler
   \param e Mouse event
*/
void QwtAbstractSlider::mouseReleaseEvent( QMouseEvent *e )
{
    if ( isReadOnly() )
    {
        e->ignore();
        return;
    }
    if ( !isValid() )
        return;

    const double inc = step();

    switch ( d_data->scrollMode )
    {
        case ScrMouse:
        {
            setPosition( e->pos() );
            d_data->direction = 0;
            d_data->mouseOffset = 0;
            if ( d_data->mass > 0.0 )
            {
                const int ms = d_data->time.elapsed();
                if ( ( qFabs( d_data->speed ) >  0.0 ) && ( ms < 50 ) )
                    d_data->timerId = startTimer( d_data->updateInterval );
            }
            else
            {
                d_data->scrollMode = ScrNone;
                buttonReleased();
            }
            Q_EMIT sliderReleased();

            break;
        }

        case ScrDirect:
        {
            setPosition( e->pos() );
            d_data->direction = 0;
            d_data->mouseOffset = 0;
            d_data->scrollMode = ScrNone;
            buttonReleased();
            break;
        }

        case ScrPage:
        {
            stopMoving();
            if ( !d_data->timerTick )
                incPages( d_data->direction );
            d_data->timerTick = 0;
            buttonReleased();
            d_data->scrollMode = ScrNone;
            break;
        }

        case ScrTimer:
        {
            stopMoving();
            if ( !d_data->timerTick )
                setNewValue( value() + double( d_data->direction ) * inc, true );
            d_data->timerTick = 0;
            buttonReleased();
            d_data->scrollMode = ScrNone;
            break;
        }

        default:
        {
            d_data->scrollMode = ScrNone;
            buttonReleased();
        }
    }
}


/*!
  Move the slider to a specified point, adjust the value
  and emit signals if necessary.
*/
void QwtAbstractSlider::setPosition( const QPoint &p )
{
    setNewValue( getValue( p ) - d_data->mouseOffset, true );
}


/*!
  \brief Enables or disables tracking.

  If tracking is enabled, the slider emits a
  valueChanged() signal whenever its value
  changes (the default behaviour). If tracking
  is disabled, the value changed() signal will only
  be emitted if:<ul>
  <li>the user releases the mouse
      button and the value has changed or
  <li>at the end of automatic scrolling.</ul>
  Tracking is enabled by default.
  \param enable \c true (enable) or \c false (disable) tracking.

  \sa isTracking()
*/
void QwtAbstractSlider::setTracking( bool enable )
{
    d_data->tracking = enable;
}

/*!
  \return True, when tracking has been enabled
  \sa setTracking()
*/
bool QwtAbstractSlider::isTracking() const
{
    return d_data->tracking;
}

/*!
   Mouse Move Event handler
   \param e Mouse event
*/
void QwtAbstractSlider::mouseMoveEvent( QMouseEvent *e )
{
    if ( isReadOnly() )
    {
        e->ignore();
        return;
    }

    if ( !isValid() )
        return;

    if ( d_data->scrollMode == ScrMouse )
    {
        setPosition( e->pos() );
        if ( d_data->mass > 0.0 )
        {
            double ms = double( d_data->time.elapsed() );
            if ( ms < 1.0 )
                ms = 1.0;
            d_data->speed = ( exactValue() - exactPrevValue() ) / ms;
            d_data->time.start();
        }
        if ( value() != prevValue() )
            Q_EMIT sliderMoved( value() );
    }
}

/*!
   Wheel Event handler
   \param e Whell event
*/
void QwtAbstractSlider::wheelEvent( QWheelEvent *e )
{
    if ( isReadOnly() )
    {
        e->ignore();
        return;
    }

    if ( !isValid() )
        return;

    QwtAbstractSlider::ScrollMode mode = ScrNone; 
    int direction = 0;

    // Give derived classes a chance to say ScrNone
    getScrollMode( e->pos(), mode, direction );
    if ( mode != QwtAbstractSlider::ScrNone )
    {
        // Most mouse types work in steps of 15 degrees, in which case
        // the delta value is a multiple of 120

        const int inc = e->delta() / 120;
        incPages( inc );
        if ( value() != prevValue() )
            Q_EMIT sliderMoved( value() );
    }
}

/*!
  Handles key events

  - Key_Down, KeyLeft\n
    Decrement by 1
  - Key_Up, Key_Right\n
    Increment by 1

  \param e Key event
  \sa isReadOnly()
*/
void QwtAbstractSlider::keyPressEvent( QKeyEvent *e )
{
    if ( isReadOnly() )
    {
        e->ignore();
        return;
    }

    if ( !isValid() )
        return;

    int increment = 0;
    switch ( e->key() )
    {
        case Qt::Key_Down:
            if ( orientation() == Qt::Vertical )
                increment = -1;
            break;
        case Qt::Key_Up:
            if ( orientation() == Qt::Vertical )
                increment = 1;
            break;
        case Qt::Key_Left:
            if ( orientation() == Qt::Horizontal )
                increment = -1;
            break;
        case Qt::Key_Right:
            if ( orientation() == Qt::Horizontal )
                increment = 1;
            break;
        default:;
            e->ignore();
    }

    if ( increment != 0 )
    {
        incValue( increment );
        if ( value() != prevValue() )
            Q_EMIT sliderMoved( value() );
    }
}

/*!
   Qt timer event
   \param e Timer event
*/
void QwtAbstractSlider::timerEvent( QTimerEvent * )
{
    const double inc = step();

    switch ( d_data->scrollMode )
    {
        case ScrMouse:
        {
            if ( d_data->mass > 0.0 )
            {
                d_data->speed *= qExp( - double( d_data->updateInterval ) * 0.001 / d_data->mass );
                const double newval =
                    exactValue() + d_data->speed * double( d_data->updateInterval );
                setNewValue( newval, true );
                // stop if d_data->speed < one step per second
                if ( qFabs( d_data->speed ) < 0.001 * qFabs( step() ) )
                {
                    d_data->speed = 0;
                    stopMoving();
                    buttonReleased();
                }

            }
            else
            {
                stopMoving();
            }
            break;
        }

        case ScrPage:
        {
            incPages( d_data->direction );
            if ( !d_data->timerTick )
            {
                killTimer( d_data->timerId );
                d_data->timerId = startTimer( d_data->updateInterval );
            }
            break;
        }
        case ScrTimer:
        {
            setNewValue( value() +  double( d_data->direction ) * inc, true );
            if ( !d_data->timerTick )
            {
                killTimer( d_data->timerId );
                d_data->timerId = startTimer( d_data->updateInterval );
            }
            break;
        }
        default:
        {
            stopMoving();
            break;
        }
    }

    d_data->timerTick = 1;
}


/*!
  Notify change of value

  This function can be reimplemented by derived classes
  in order to keep track of changes, i.e. repaint the widget.
  The default implementation emits a valueChanged() signal
  if tracking is enabled.
*/
void QwtAbstractSlider::valueChange()
{
    if ( d_data->tracking )
        Q_EMIT valueChanged( value() );
}

/*!
  \brief Notify a change of the range

  This virtual function is called whenever the range changes.
  The default implementation does nothing.
*/
void QwtAbstractSlider::rangeChange()
{
}

/*!
  \brief Specify  range and step size

  \param vmin   lower boundary of the interval
  \param vmax   higher boundary of the interval
  \param vstep  step width
  \param pageSize  page size in steps
  \warning
  \li A change of the range changes the value if it lies outside the
      new range. The current value
      will *not* be adjusted to the new step raster.
  \li vmax < vmin is allowed.
  \li If the step size is left out or set to zero, it will be
      set to 1/100 of the interval length.
  \li If the step size has an absurd value, it will be corrected
      to a better one.
*/
void QwtAbstractSlider::setRange(
    double vmin, double vmax, double vstep, int pageSize )
{   
    const bool rchg = ( d_data->maxValue != vmax || d_data->minValue != vmin );
    
    if ( rchg )
    {
        d_data->minValue = vmin;
        d_data->maxValue = vmax;
    }

    // look if the step width has an acceptable
    // value or otherwise change it.
    setStep( vstep );

    // limit page size
    const int max =
        int( qAbs( ( d_data->maxValue - d_data->minValue ) / d_data->step ) );
    d_data->pageSize = qBound( 0, pageSize, max );

    // If the value lies out of the range, it
    // will be changed. Note that it will not be adjusted to
    // the new step width.
    setNewValue( d_data->value, false );

    // call notifier after the step width has been
    // adjusted.
    if ( rchg )
        rangeChange();
}

/*!
  \brief Change the step raster
  \param vstep new step width
  \warning The value will \e not be adjusted to the new step raster.
*/
void QwtAbstractSlider::setStep( double vstep )
{
    const double intv = d_data->maxValue - d_data->minValue;
    
    double newStep;
    if ( vstep == 0.0 )
    {
        const double defaultRelStep = 1.0e-2;
        newStep = intv * defaultRelStep;
    }   
    else
    {
        if ( ( intv > 0.0 && vstep < 0.0 ) || ( intv < 0.0 && vstep > 0.0 ) )
            newStep = -vstep;
        else
            newStep = vstep;
            
        const double minRelStep = 1.0e-10;
        if ( qFabs( newStep ) < qFabs( minRelStep * intv ) )
            newStep = minRelStep * intv;
    }       
    
    if ( newStep != d_data->step )
    {
        d_data->step = newStep;
    }   
}   

/*!
  \return the step size
  \sa setStep(), setRange()
*/
double QwtAbstractSlider::step() const
{
    return qAbs( d_data->step );
}   

/*!
  \brief Returns the value of the second border of the range

  maxValue returns the value which has been specified
  as the second parameter in  QwtAbstractSlider::setRange.

  \sa setRange()
*/
double QwtAbstractSlider::maxValue() const
{
    return d_data->maxValue;
}   

/*!
  \brief Returns the value at the first border of the range

  minValue returns the value which has been specified
  as the first parameter in  setRange().

  \sa setRange()
*/
double QwtAbstractSlider::minValue() const
{
    return d_data->minValue;
}   

//! Returns the page size in steps.
int QwtAbstractSlider::pageSize() const
{
    return d_data->pageSize;
}

//! Returns the current value.
double QwtAbstractSlider::value() const
{
    return d_data->value;
}

/*!
  \brief Returns the exact value

  The exact value is the value which QwtAbstractSlider::value would return
  if the value were not adjusted to the step raster. It differs from
  the current value only if fitValue() or incValue() have been used before. 
  This function is intended for internal use in derived classes.
*/
double QwtAbstractSlider::exactValue() const
{
    return d_data->exactValue;
}

//! Returns the exact previous value
double QwtAbstractSlider::exactPrevValue() const
{
    return d_data->exactPrevValue;
}

//! Returns the previous value
double QwtAbstractSlider::prevValue() const
{
    return d_data->prevValue;
}

/*!
  \brief Make the range periodic

  When the range is periodic, the value will be set to a point
  inside the interval such that

  \verbatim point = value + n * width \endverbatim

  if the user tries to set a new value which is outside the range.
  If the range is nonperiodic (the default), values outside the
  range will be clipped.

  \param tf true for a periodic range
*/
void QwtAbstractSlider::setPeriodic( bool tf )
{
    d_data->periodic = tf;
}   

/*!
  \brief Returns true if the range is periodic
  \sa setPeriodic()
*/
bool QwtAbstractSlider::periodic() const
{
    return d_data->periodic;
}

/*!
  \brief Set the slider's mass for flywheel effect.

  If the slider's mass is greater then 0, it will continue
  to move after the mouse button has been released. Its speed
  decreases with time at a rate depending on the slider's mass.
  A large mass means that it will continue to move for a
  long time.

  Derived widgets may overload this function to make it public.

  \param val New mass in kg

  \bug If the mass is smaller than 1g, it is set to zero.
       The maximal mass is limited to 100kg.
  \sa mass()
*/
void QwtAbstractSlider::setMass( double val )
{
    if ( val < 0.001 )
        d_data->mass = 0.0;
    else if ( val > 100.0 )
        d_data->mass = 100.0;
    else
        d_data->mass = val;
}

/*!
    \return mass
    \sa setMass()
*/
double QwtAbstractSlider::mass() const
{
    return d_data->mass;
}


/*!
  \brief Move the slider to a specified value

  This function can be used to move the slider to a value
  which is not an integer multiple of the step size.
  \param val new value
  \sa fitValue()
*/
void QwtAbstractSlider::setValue( double val )
{
    if ( d_data->scrollMode == ScrMouse )
        stopMoving();

    setNewValue( val, false );
}

/*!
  \brief Increment the value by a specified number of steps
  \param nSteps Number of steps to increment
  \warning As a result of this operation, the new value will always be
       adjusted to the step raster.
*/
void QwtAbstractSlider::incValue( int nSteps )
{
    if ( isValid() )
        setNewValue( value() + double( nSteps ) * step(), true );
}

/*!
  \brief Increment the value by a specified number of pages
  \param nPages Number of pages to increment.
        A negative number decrements the value.
  \warning The Page size is specified in the constructor.
*/
void QwtAbstractSlider::incPages( int nPages )
{
    if ( isValid() )
    {
        const double off = step() * pageSize() * nPages;
        setNewValue( value() + off, true );
    }
}

/*!
  \sa mouseOffset()
*/
void QwtAbstractSlider::setMouseOffset( double offset )
{
    d_data->mouseOffset = offset;
}

/*!
  \sa setMouseOffset()
*/
double QwtAbstractSlider::mouseOffset() const
{
    return d_data->mouseOffset;
}

//! sa ScrollMode
int QwtAbstractSlider::scrollMode() const
{
    return d_data->scrollMode;
}

void QwtAbstractSlider::setNewValue( double value, bool align )
{
    d_data->prevValue = d_data->value;
    
    const double vmin = qMin( d_data->minValue, d_data->maxValue );
    const double vmax = qMax( d_data->minValue, d_data->maxValue );
    
    if ( value < vmin )
    {
        if ( d_data->periodic && vmin != vmax )
        {
            d_data->value = value +
                ::ceil( ( vmin - value ) / ( vmax - vmin ) ) * ( vmax - vmin );
        }       
        else    
            d_data->value = vmin;
    }       
    else if ( value > vmax )
    {
        if ( ( d_data->periodic ) && ( vmin != vmax ) )
        {
            d_data->value = value -
                ::ceil( ( value - vmax ) / ( vmax - vmin ) ) * ( vmax - vmin );
        }
        else
            d_data->value = vmax;
    }
    else
    {
        d_data->value = value;
    }

    d_data->exactPrevValue = d_data->exactValue;
    d_data->exactValue = d_data->value;

    if ( align )
    {
        if ( d_data->step != 0.0 )
        {
            d_data->value = d_data->minValue +
                qRound( ( d_data->value - d_data->minValue ) / d_data->step ) * d_data->step;
        }
        else
            d_data->value = d_data->minValue;

        const double minEps = 1.0e-10;
        // correct rounding error at the border
        if ( qFabs( d_data->value - d_data->maxValue ) < minEps * qAbs( d_data->step ) )
            d_data->value = d_data->maxValue;

        // correct rounding error if value = 0
        if ( qFabs( d_data->value ) < minEps * qAbs( d_data->step ) )
            d_data->value = 0.0;
    }

    if ( !d_data->isValid || d_data->prevValue != d_data->value )
    {
        d_data->isValid = true;
        valueChange();
    }
}

