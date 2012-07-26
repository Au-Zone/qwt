/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_abstract_slider.h"
#include "qwt_abstract_scale_draw.h"
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
        mouseOffset( 0.0 ),
        tracking( true ),
        pendingValueChanged( false ),
        readOnly( false ),
        singleStep( 1.0 ),
        pageStepCount( 1 ),
        stepAlignment( true ),
        isValid( false ),
        value( 0.0 ),
        wrapping( false )
    {
    }

    bool isScrolling;
    double mouseOffset;
    bool tracking;
    bool pendingValueChanged;

    bool readOnly;

    double singleStep;
    int pageStepCount;
    bool stepAlignment;

    bool isValid;
    double value;

    bool wrapping;
};

/*!
   \brief Constructor

   The range is initialized to [0.0, 100.0], the
   step size to 1.0, and the value to 0.0.

   \param parent Parent widget
*/
QwtAbstractSlider::QwtAbstractSlider( QWidget *parent ):
    QwtAbstractScale( parent )
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
        update();

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

    if ( !d_data->isValid || lowerBound() == upperBound() )
        return;

    d_data->isScrolling = isScrollPosition( event->pos() );

    if ( d_data->isScrolling )
    {
        d_data->mouseOffset = valueAt( event->pos() ) - d_data->value;
        d_data->pendingValueChanged = false;

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

    if ( d_data->isValid && d_data->isScrolling )
    {
        double value = valueAt( e->pos() ) - d_data->mouseOffset;

        value = boundedValue( value );

        if ( d_data->stepAlignment )
            value = alignedValue( value );

        if ( value != d_data->value )
        {
            d_data->value = value;

            update();

            if ( d_data->tracking )
                Q_EMIT valueChanged( d_data->value );
            else
                d_data->pendingValueChanged = true;

            Q_EMIT sliderMoved( d_data->value );
        }
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

    if ( d_data->isScrolling && d_data->isValid )
    {
        d_data->mouseOffset = 0.0;
        d_data->isScrolling = false;

        if ( d_data->pendingValueChanged )
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

    if ( !d_data->isValid || d_data->isScrolling )
        return;

    double increment = 0.0;

    if ( ( event->modifiers() & Qt::ControlModifier) ||
        ( event->modifiers() & Qt::ShiftModifier ) )
    {
        // one page regardless of delta
        increment = d_data->singleStep * d_data->pageStepCount;
        if ( event->delta() < 0 )
            increment = -increment;
    }
    else
    {
        const int numSteps = event->delta() / 120;
        increment = d_data->singleStep * numSteps;
    }

    double value = boundedValue( d_data->value + increment );

    if ( d_data->stepAlignment )
        value = alignedValue( value );

    if ( value != d_data->value )
    {
        d_data->value = value;
        update();

        Q_EMIT valueChanged( d_data->value );
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
void QwtAbstractSlider::keyPressEvent( QKeyEvent *event )
{
    if ( isReadOnly() )
    {
        event->ignore();
        return;
    }

    if ( !d_data->isValid || d_data->isScrolling )
        return;

    const double stepSize = qAbs( d_data->singleStep );

    double value = d_data->value;
    double increment = 0.0;

#if 1
    // better use key mapping from QAbstractSlider or QDial
    switch ( event->key() )
    {
        case Qt::Key_Down:
        case Qt::Key_Left:
        {
            increment = -stepSize;
            break;
        }
        case Qt::Key_Up:
        case Qt::Key_Right:
        {
            increment = stepSize;
            break;
        }
        case Qt::Key_PageUp:
        {
            increment = d_data->pageStepCount * stepSize;
            break;
        }
        case Qt::Key_PageDown:
        {
            increment = -d_data->pageStepCount * stepSize;
            break;
        }
        case Qt::Key_Home:
        {
            value = lowerBound();
            break;
        }
        case Qt::Key_End:
        {
            value = upperBound();
            break;
        }
        default:;
        {
            event->ignore();
        }
    }
#endif

    if ( increment != 0.0 )
    {
        value = boundedValue( d_data->value + increment );

        if ( d_data->stepAlignment )
            value = alignedValue( value );
    }

    if ( value != d_data->value )
    {
        d_data->value = value;
        update();

        Q_EMIT valueChanged( d_data->value );
        Q_EMIT sliderMoved( d_data->value );
    }
}

/*!
  \brief Change the step raster
  \param vstep new step width
  \warning The value will \e not be adjusted to the new step raster.
*/
void QwtAbstractSlider::setSingleStep( double vstep )
{
    const double range = upperBound() - lowerBound();
    
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
  \brief Set the page step count 

  pageStepCount is a multiplicator for the single step size
  that typically corresponds to the user pressing PageUp or PageDown.

  A value of 0 disables page stepping. The value is floored to
  ( upperBound() - lowerBound() ) / singleStep().

  The default value is 1.

  \param count Multiplicator for the single step size

  \sa pageStepCount(), setSingleStep()
 */
void QwtAbstractSlider::setPageStepCount( int count )
{
    const double range = upperBound() - lowerBound();

    const int max = int( qAbs( range / d_data->singleStep ) );
    d_data->pageStepCount = qBound( 0, count, max );
}

/*! 
  \return Page step count
  \sa setPageStepCount(), singleStep()
 */
int QwtAbstractSlider::pageStepCount() const
{
    return d_data->pageStepCount;
}

void QwtAbstractSlider::setStepAlignment( bool on )
{   
    if ( on != d_data->stepAlignment )
    {
        d_data->stepAlignment = on;
    }
}   
    
bool QwtAbstractSlider::stepAlignment() const
{
    return d_data->stepAlignment;
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
  \brief Move the slider to a specified value

  This function can be used to move the slider to a value
  which is not an integer multiple of the step size.
  \param val new value
  \sa fitValue()
*/
void QwtAbstractSlider::setValue( double value )
{
    value = qBound( minimum(), value, maximum() );

    const bool changed = ( d_data->value != value ) || !d_data->isValid;

    d_data->value = value;
    d_data->isValid = true;

    if ( changed )
    {
        update();
        Q_EMIT valueChanged( d_data->value );
    }
}

//! Returns the current value.
double QwtAbstractSlider::value() const
{
    return d_data->value;
}

/*!
  If wrapping is true stepping up from upperBound() value will take you to the minimum() 
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

void QwtAbstractSlider::incrementValue( double increment )
{
    double value = boundedValue( d_data->value + increment );
#if 0
    if ( d_data->stepAlignment )
        value = alignedValue( value );
#endif

    if ( value != d_data->value )
    {
        d_data->value = value;
        update();
    }
}

double QwtAbstractSlider::boundedValue( double value ) const
{
    const double vmin = minimum();
    const double vmax = maximum();

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

    return value;
}

double QwtAbstractSlider::alignedValue( double value ) const
{
    const double stepSize = d_data->singleStep;

    if ( stepSize > 0.0 )
    {
        value = lowerBound() + qRound( ( value - lowerBound() ) / stepSize ) * stepSize;

        // correct rounding error at the border
        if ( qFuzzyCompare( value, upperBound() ) )
            value = upperBound();

        // correct rounding error if value = 0
        if ( qFuzzyCompare( value + 1.0, 1.0 ) )
            value = 0.0;
    }

    return value;
}

void QwtAbstractSlider::scaleChange()
{
    const double value = qBound( minimum(), value, maximum() );

    setSingleStep( singleStep() );

    const bool changed = ( value != d_data->value );

    if ( changed )
    {
        d_data->value = value;
    }

    if ( d_data->isValid || changed )
        Q_EMIT valueChanged( d_data->value );

    updateGeometry();
    update();
}
