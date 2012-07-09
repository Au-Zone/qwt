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

#if QT_VERSION < 0x040601
#define qFabs(x) ::fabs(x)
#endif

class QwtAbstractSlider::PrivateData
{
public:
    PrivateData():
        isScrolling( false ),
        initialScrollOffset( 0.0 ),
        tracking( true ),
        readOnly( false ),
        minimum( 0.0 ),
        maximum( 0.0 ),
        singleStep( 1.0 ),
        pageSize( 1 ),
        isValid( false ),
        value( 0.0 ),
        exactValue( 0.0 ),
        wrapping( false )
    {
    }

    bool isScrolling;
    double initialScrollOffset;
    bool tracking;

    bool readOnly;

    double minimum;
    double maximum;
    double singleStep;
    int pageSize;

    bool isValid;
    double value;
    double exactValue;

    bool wrapping;
};

/*!
   \brief Constructor

   The range is initialized to [0.0, 100.0], the
   step size to 1.0, and the value to 0.0.

   \param parent Parent widget
*/
QwtAbstractSlider::QwtAbstractSlider( QWidget *parent ):
    QWidget( parent, NULL )
{
    d_data = new QwtAbstractSlider::PrivateData;
    setFocusPolicy( Qt::StrongFocus );
}

//! Destructor
QwtAbstractSlider::~QwtAbstractSlider()
{
    delete d_data;
}

//! Set the value to be valid/invalid
void QwtAbstractSlider::setValid( bool isValid )
{
    if ( isValid != d_data->isValid )
    {
        d_data->isValid = isValid;
        valueChange();

        Q_EMIT valueChanged( d_data->value );
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
  \brief Enables or disables tracking.

  If tracking is enabled, the slider emits the valueChanged() 
  signal while the slider is being dragged. If tracking is disabled, the 
  slider emits the valueChanged() signal only when the user releases the slider.

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
   Mouse press event handler
   \param e Mouse event
*/
void QwtAbstractSlider::mousePressEvent( QMouseEvent *event )
{
    if ( isReadOnly() )
    {
        event->ignore();
        return;
    }

    if ( !d_data->isValid )
        return;

    d_data->isScrolling = isScrollPosition( event->pos() );

    if ( d_data->isScrolling )
    {
        d_data->initialScrollOffset = valueAt( event->pos() ) - d_data->value;

        Q_EMIT sliderPressed();
    }
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

    if ( !d_data->isValid )
        return;

    if ( d_data->isScrolling )
    {
        const double newValue = 
            valueAt( e->pos() ) - d_data->initialScrollOffset;

        const bool changed = updateValue( newValue );
        if ( changed )
            Q_EMIT sliderMoved( d_data->value );
    }
}

/*!
   Mouse Release Event handler
   \param e Mouse event
*/
void QwtAbstractSlider::mouseReleaseEvent( QMouseEvent *event )
{
    if ( isReadOnly() )
    {
        event->ignore();
        return;
    }

    if ( !d_data->isValid )
        return;

    if ( d_data->isScrolling )
    {
        const double newValue = 
            valueAt( event->pos() ) - d_data->initialScrollOffset;

        const bool changed = updateValue( newValue );

        d_data->initialScrollOffset = 0.0;

        d_data->isScrolling = false;
        if ( ( !d_data->tracking ) || changed )
            Q_EMIT valueChanged( d_data->value );

        Q_EMIT sliderReleased();
    }
}

/*!
   Wheel Event handler
   \param e Whell event
*/
void QwtAbstractSlider::wheelEvent( QWheelEvent *event )
{
    if ( isReadOnly() )
    {
        event->ignore();
        return;
    }

    if ( !d_data->isValid )
        return;

    const int numPages = event->delta() / 120;

    const double stepSize = qAbs( d_data->singleStep );
    const double off = stepSize * d_data->pageSize * numPages;

    const bool changed = updateValue( d_data->value + off );
    if ( changed )
    {   
        Q_EMIT sliderMoved( d_data->value );
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

    if ( !d_data->isValid )
        return;

    const double stepSize = qAbs( d_data->singleStep );
    double value = d_data->value;

#if 1
    // better use key mapping from QAbstractSlider or QDial
    switch ( e->key() )
    {
        case Qt::Key_Down:
        case Qt::Key_Left:
        {
            value -= stepSize;
            break;
        }
        case Qt::Key_Up:
        case Qt::Key_Right:
        {
            value += stepSize;
            break;
        }
        case Qt::Key_PageUp:
        {
            value += d_data->pageSize * stepSize;
            break;
        }
        case Qt::Key_PageDown:
        {
            value -= d_data->pageSize * stepSize;
            break;
        }
        case Qt::Key_Home:
        {
            value = d_data->minimum;
            break;
        }
        case Qt::Key_End:
        {
            value = d_data->maximum;
            break;
        }
        default:;
        {
            e->ignore();
        }
    }
#endif

    if ( value != d_data->value )
    {
        const bool changed = updateValue( value );
        if ( changed )
        {   
            Q_EMIT sliderMoved( d_data->value );
        }   
    }
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
    update();
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

  \param minimum   lower boundary of the interval
  \param maximum   higher boundary of the interval
  \warning
  \li A change of the range changes the value if it lies outside the
      new range. The current value
      will *not* be adjusted to the new step raster.
  \li maximum < minimum is allowed.
*/
void QwtAbstractSlider::setRange( double minimum, double maximum )
{   
    if ( d_data->minimum == minimum && d_data->maximum == maximum )
        return;
    
    d_data->minimum = minimum;
    d_data->maximum = maximum;

    const double vmin = qMin( d_data->minimum, d_data->maximum );
    const double vmax = qMax( d_data->minimum, d_data->maximum );
    
    const double value = qBound( vmin, value, vmax );

#if 0
    setSingleStep( singleStep() );
#endif

    const bool changed = value != d_data->value;

    if ( changed )
    {
        d_data->value = value;
        d_data->exactValue = value;
    }
    
    rangeChange();

    if ( d_data->isValid || changed )
    {
        valueChange();
        Q_EMIT valueChanged( d_data->value );
    }   
}

/*!
  \brief Change the step raster
  \param vstep new step width
  \warning The value will \e not be adjusted to the new step raster.
*/
void QwtAbstractSlider::setSingleStep( double vstep )
{
    const double range = d_data->maximum - d_data->minimum;
    
    double newStep;
    if ( vstep == 0.0 )
    {
        const double defaultRelStep = 1.0e-2;
        newStep = range * defaultRelStep;
    }   
    else
    {
        if ( ( range > 0.0 && vstep < 0.0 ) || ( range < 0.0 && vstep > 0.0 ) )
            newStep = -vstep;
        else
            newStep = vstep;
            
        const double minRelStep = 1.0e-10;
        if ( qFabs( newStep ) < qFabs( minRelStep * range ) )
            newStep = minRelStep * range;
    }       
    
    d_data->singleStep = newStep;
}   

/*!
  \return The absolute step size
  \sa setStep(), setRange()
*/
double QwtAbstractSlider::singleStep() const
{
    return d_data->singleStep;
}   

/*!
  Set the maximum value of the range

  \param value Maximum value
  \sa setRange(), setMinimum(), maximum()
*/
void QwtAbstractSlider::setMaximum( double max )
{
    setRange( minimum(), max );
}

/*!
  \brief Returns the value of the second border of the range

  maximum returns the value which has been specified
  as the second parameter in  QwtAbstractSlider::setRange.

  \sa setRange()
*/
double QwtAbstractSlider::maximum() const
{
    return d_data->maximum;
}   

/*!
  Set the minimum value of the range

  \param value Minimum value
  \sa setRange(), setMaximum(), minimum()

  \note The maximum is adjusted if necessary to ensure that the range remains valid.
*/
void QwtAbstractSlider::setMinimum( double min )
{
    setRange( min, maximum() );
}

/*!
  \brief Returns the value at the first border of the range

  minimum returns the value which has been specified
  as the first parameter in  setRange().

  \sa setRange()
*/
double QwtAbstractSlider::minimum() const
{
    return d_data->minimum;
}   

void QwtAbstractSlider::setPageSize( int pageSize )
{
#if 1
    // limit page size
    const int max =
        int( qAbs( ( d_data->maximum - d_data->minimum ) / d_data->singleStep ) );
    d_data->pageSize = qBound( 0, pageSize, max );
#endif
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
  If wrapping is true stepping up from maximum() value will take you to the minimum() 
  value and vica versa. 

  \param on En/Disable wrapping
  \sa wrapping()
*/
void QwtAbstractSlider::setWrapping( bool on )
{
    d_data->wrapping = on;
}   

/*!
  \return True, when wrapping is set
  \sa setWrapping()
 */ 
bool QwtAbstractSlider::wrapping() const
{
    return d_data->wrapping;
}

/*!
  \brief Move the slider to a specified value

  This function can be used to move the slider to a value
  which is not an integer multiple of the step size.
  \param val new value
  \sa fitValue()
*/
void QwtAbstractSlider::setValue( double value )
{
    const double vmin = qMin( d_data->minimum, d_data->maximum );
    const double vmax = qMax( d_data->minimum, d_data->maximum );
    
    value = qBound( vmin, value, vmax );

    const bool changed = ( d_data->value != value ) || !d_data->isValid;

    d_data->value = value;
    d_data->exactValue = value;
    d_data->isValid = true;

    if ( changed )
    {
        valueChange();
        Q_EMIT valueChanged( d_data->value );
    }
}

/*!
  \sa mouseOffset()
*/
void QwtAbstractSlider::setMouseOffset( double offset )
{
    d_data->initialScrollOffset = offset;
}

/*!
  \sa setMouseOffset()
*/
double QwtAbstractSlider::mouseOffset() const
{
    return d_data->initialScrollOffset;
}

bool QwtAbstractSlider::setNewValue( double value )
{
    const double vmin = qMin( d_data->minimum, d_data->maximum );
    const double vmax = qMax( d_data->minimum, d_data->maximum );
    
    if ( d_data->wrapping && vmin != vmax )
    {
        const double range = vmax - vmin;

        if ( value < vmin )
        {
            value += ::ceil( ( vmin - value ) / range ) * range;
        }       
        else if ( value > vmax )
        {
            value -= ::ceil( ( value - vmax ) / range ) * range;
        }
    }
    else
    {
        value = qBound( vmin, value, vmax );
    }

    d_data->exactValue = value;

    if ( d_data->singleStep != 0.0 )
    {
        value = d_data->minimum +
            qRound( ( value - d_data->minimum ) / d_data->singleStep ) * d_data->singleStep;

        // correct rounding error at the border
        if ( qFuzzyCompare( value, d_data->maximum ) )
            value = d_data->maximum;

        // correct rounding error if value = 0
        if ( qFuzzyCompare( value + 1.0, 1.0 ) )
            value = 0.0;
    }
    else
    {
        value = d_data->minimum;
    }

    if ( value != d_data->value )
    {
        d_data->value = value;
        return true;
    }
    else
    {
        return false;
    }
}

bool QwtAbstractSlider::updateValue( double value )
{
    const bool changed = setNewValue( value );
    if ( changed )
    {
        valueChange();

        if ( d_data->tracking )
            Q_EMIT valueChanged( d_data->value );
    }

    return changed;
}
