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
        repeatTimerId( 0 ),
        flyingTimerId( 0 ),
        updateInterval( 150 ),
        mass( 0.0 ),
        readOnly( false ),
        minimum( 0.0 ),
        maximum( 0.0 ),
        singleStep( 1.0 ),
        pageSize( 1 ),
        isValid( false ),
        value( 0.0 ),
        exactValue( 0.0 ),
        prevValue( 0.0 ),
        wrapping( false )
    {
    }

    QwtAbstractSlider::ScrollMode scrollMode;
    double mouseOffset;
    int direction;
    bool tracking;

    int repeatTimerId;
    int flyingTimerId;
    int updateInterval;
    bool timerTick;
    QTime time;
    double speed;
    double mass;
    Qt::Orientation orientation;
    bool readOnly;

    double minimum;
    double maximum;
    double singleStep;
    int pageSize;

    bool isValid;
    double value;
    double exactValue;
    double prevValue;

    bool wrapping;
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
void QwtAbstractSlider::mousePressEvent( QMouseEvent *event )
{
    if ( isReadOnly() )
    {
        event->ignore();
        return;
    }

    if ( !d_data->isValid )
        return;

    d_data->timerTick = false;

    getScrollMode( event->pos(), d_data->scrollMode, d_data->direction );
    stopFlying();

    switch ( d_data->scrollMode )
    {
        case ScrPage:
        case ScrTimer:
		{
            d_data->mouseOffset = 0;
            d_data->repeatTimerId = startTimer( qMax( 250, 2 * d_data->updateInterval ) );
            break;
		}
        case ScrMouse:
		{
            d_data->time.start();
            d_data->speed = 0;
            d_data->mouseOffset = getValue( event->pos() ) - d_data->value;
            Q_EMIT sliderPressed();
            break;
		}
        case ScrNone:
		{
            d_data->mouseOffset = 0;
            d_data->direction = 0;
            break;
		}
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

    if ( d_data->scrollMode == ScrMouse )
    {
		const double exactPrevValue = d_data->exactValue;

		const bool changed = 
			setNewValue( getValue( e->pos() ) - d_data->mouseOffset );

    	if ( changed )
		{
			valueChange();

			if ( d_data->tracking )
				Q_EMIT valueChanged( d_data->value );
		}

        if ( d_data->mass > 0.0 )
        {
            const double ms = d_data->time.restart();
            d_data->speed = ( d_data->exactValue - exactPrevValue ) / qMax( ms, 1.0 );
        }

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

    switch ( d_data->scrollMode )
    {
        case ScrMouse:
        {
			const bool changed = setNewValue( getValue( event->pos() ) - d_data->mouseOffset );

    		if ( changed )
			{   
				valueChange();

				if ( d_data->tracking )
					Q_EMIT valueChanged( d_data->value );
			}   

            d_data->direction = 0;
            d_data->mouseOffset = 0;

            if ( d_data->mass > 0.0 )
            {
                const int ms = d_data->time.elapsed();
                if ( ( qFabs( d_data->speed ) >  0.0 ) && ( ms < 50 ) )
                    d_data->flyingTimerId = startTimer( d_data->updateInterval );
            }
            else
            {
                d_data->scrollMode = ScrNone;
    			if ( ( !d_data->tracking ) || changed )
        			Q_EMIT valueChanged( d_data->value );

            }
            Q_EMIT sliderReleased();

            break;
        }
        case ScrPage:
        case ScrTimer:
        {
			killTimer( d_data->repeatTimerId );
			d_data->repeatTimerId = 0;

            if ( !d_data->timerTick )
			{
				double off = qAbs( d_data->singleStep ) * d_data->direction;
				if ( d_data->scrollMode == ScrPage )
					off *= d_data->pageSize;

               	const bool changed = setNewValue( d_data->value + off );
				if ( changed )
				{   
					valueChange();

					if ( d_data->tracking )
						Q_EMIT valueChanged( d_data->value );
				}   
			}

            d_data->timerTick = false;
            d_data->scrollMode = ScrNone;

    		if ( ( !d_data->tracking ) || ( d_data->value != d_data->prevValue ) )
       			Q_EMIT valueChanged( d_data->value );

            break;
        }
        case ScrNone:
        {
			break;
        }
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

    if ( !d_data->isValid )
        return;

    QwtAbstractSlider::ScrollMode mode = ScrNone; 
    int direction = 0;

    // Give derived classes a chance to say ScrNone
    getScrollMode( e->pos(), mode, direction );
    if ( mode != QwtAbstractSlider::ScrNone )
    {
		stopFlying();

        // Most mouse types work in steps of 15 degrees, in which case
        // the delta value is a multiple of 120

        const int numPages = e->delta() / 120;

        const double stepSize = qAbs( d_data->singleStep );
        const double off = stepSize * d_data->pageSize * numPages;
        const bool changed = setNewValue( d_data->value + off );

		if ( changed )
		{   
			valueChange();

			if ( d_data->tracking )
				Q_EMIT valueChanged( d_data->value );

            Q_EMIT sliderMoved( d_data->value );
		}   
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

    switch ( e->key() )
    {
        case Qt::Key_Down:
		{
            if ( orientation() == Qt::Vertical )
                value -= stepSize;
            break;
		}
        case Qt::Key_Up:
		{
            if ( orientation() == Qt::Vertical )
                value += stepSize;
            break;
		}
        case Qt::Key_Left:
		{
            if ( orientation() == Qt::Horizontal )
                value -= stepSize;
            break;
		}
        case Qt::Key_Right:
		{
            if ( orientation() == Qt::Horizontal )
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

    if ( value != d_data->value )
    {
		stopFlying();

        const bool changed = setNewValue( value );

		if ( changed )
		{   
			valueChange();

			if ( d_data->tracking )
				Q_EMIT valueChanged( d_data->value );

            Q_EMIT sliderMoved( d_data->value );
		}   
    }
}

/*!
   Qt timer event
   \param e Timer event
*/
void QwtAbstractSlider::timerEvent( QTimerEvent *event )
{
	if ( event->timerId() == d_data->flyingTimerId )
    {
		if ( !d_data->isValid || d_data->mass <= 0.0 )
		{
			killTimer( d_data->flyingTimerId );
			d_data->flyingTimerId = 0;
			return;
		}

		const double intv = d_data->updateInterval;

		d_data->speed *= qExp( -intv * 0.001 / d_data->mass );
		const bool changed = setNewValue( d_data->exactValue + d_data->speed * intv );

		if ( changed )
		{   
			valueChange();

			if ( d_data->tracking )
				Q_EMIT valueChanged( d_data->value );
		}   

		// stop if d_data->speed < one step per second
		if ( qFabs( d_data->speed ) < 0.001 * qAbs( d_data->singleStep ) )
		{
			d_data->speed = 0.0;

        	killTimer( d_data->flyingTimerId );
        	d_data->flyingTimerId = 0;

			if ( ( !d_data->tracking ) || changed )
				Q_EMIT valueChanged( d_data->value );
		}
	}
	else if ( event->timerId() == d_data->repeatTimerId )
	{
		if ( !d_data->isValid )
		{
			killTimer( d_data->repeatTimerId );
			d_data->repeatTimerId = 0;
			return;
		}

		d_data->prevValue = d_data->value;

		double off = d_data->direction * qAbs( d_data->singleStep );
		if ( d_data->scrollMode == ScrPage )
			off *= d_data->pageSize;

		const bool changed = setNewValue( d_data->value + off );

		if ( changed )
		{   
			valueChange();

			if ( d_data->tracking )
				Q_EMIT valueChanged( d_data->value );
		}   

		if ( !d_data->timerTick )
		{
			// restart the timer with a shorter interval
			killTimer( d_data->repeatTimerId );
			d_data->repeatTimerId = startTimer( d_data->updateInterval );

    		d_data->timerTick = true;
		}
    }

	QWidget::timerEvent( event );
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
  \brief Set the slider's mass for flywheel effect.

  If the slider's mass is greater then 0, it will continue
  to move after the mouse button has been released. Its speed
  decreases with time at a rate depending on the slider's mass.
  A large mass means that it will continue to move for a
  long time.

  Derived widgets may overload this function to make it public.

  \param mass New mass in kg

  \bug If the mass is smaller than 1g, it is set to zero.
       The maximal mass is limited to 100kg.
  \sa mass()
*/
void QwtAbstractSlider::setMass( double mass )
{
    if ( mass < 0.001 )
        d_data->mass = 0.0;
    else 
		d_data->mass = qMin( 100.0, mass );
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
void QwtAbstractSlider::setValue( double value )
{
    stopFlying();

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
  \brief Increment the value by a specified number of steps
  \param nSteps Number of steps to increment
  \warning As a result of this operation, the new value will always be
       adjusted to the step raster.
*/
void QwtAbstractSlider::incValue( int nSteps )
{
    if ( d_data->isValid )
	{
		const double stepSize = qAbs( d_data->singleStep );
        const bool changed = setNewValue( d_data->value + double( nSteps ) * stepSize );

		if ( changed )
		{   
			valueChange();

			if ( d_data->tracking )
				Q_EMIT valueChanged( d_data->value );
		}   
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

void QwtAbstractSlider::stopFlying()
{
    if ( d_data->flyingTimerId != 0 )
    {
        killTimer( d_data->flyingTimerId );
        d_data->flyingTimerId = 0;
    }
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

    const double previousValue = d_data->value;

	d_data->value = value;
    d_data->exactValue = d_data->value;

	if ( d_data->singleStep != 0.0 )
	{
		d_data->value = d_data->minimum +
			qRound( ( d_data->value - d_data->minimum ) / d_data->singleStep ) * d_data->singleStep;
	}
	else
	{
		d_data->value = d_data->minimum;
	}

	const double minEps = 1.0e-10;
	// correct rounding error at the border
	if ( qFabs( d_data->value - d_data->maximum ) < minEps * qAbs( d_data->singleStep ) )
		d_data->value = d_data->maximum;

	// correct rounding error if value = 0
	if ( qFabs( d_data->value ) < minEps * qAbs( d_data->singleStep ) )
		d_data->value = 0.0;

	return previousValue != d_data->value;
}
