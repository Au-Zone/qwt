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
#include "qwt_scale_map.h"
#include <qevent.h>

#if QT_VERSION < 0x040601
#define qFabs(x) ::fabs(x)
#endif

static double qwtAlignToScaleDiv( 
    const QwtAbstractSlider *slider, double value )
{
    const QwtScaleDiv &sd = slider->scaleDiv();

    const int tValue = qRound( slider->transform( value ) );

    if ( tValue == qRound( slider->transform( sd.lowerBound() ) ) )
        return sd.lowerBound();

    if ( tValue == qRound( slider->transform( sd.lowerBound() ) ) )
        return sd.upperBound();

    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
    {
        const QList<double> ticks = sd.ticks( i );
        for ( int j = 0; j < ticks.size(); j++ )
        {
            if ( qRound( slider->transform( ticks[ j ] ) ) == tValue )
                return ticks[ j ];
        }
    }

    return value;
}

class QwtAbstractSlider::PrivateData
{
public:
    PrivateData():
        isScrolling( false ),
        isTracking( true ),
        pendingValueChanged( false ),
        readOnly( false ),
        totalSteps( 100 ),
        singleSteps( 1 ),
        pageSteps( 10 ),
        stepAlignment( true ),
        isValid( false ),
        value( 0.0 ),
        wrapping( false )
    {
    }

    bool isScrolling;
    bool isTracking;
    bool pendingValueChanged;

    bool readOnly;

    uint totalSteps;
    uint singleSteps;
    uint pageSteps;
    bool stepAlignment;

    bool isValid;
    double value;

    bool wrapping;
};

/*!
  \brief Constructor

  The scale is initialized to [0.0, 100.0], the
  number of steps is set to 100 with 1 and 10 and single
  an page step sizes. Step alignment is enabled.

  The initial value is invalid.

  \param parent Parent widget
*/
QwtAbstractSlider::QwtAbstractSlider( QWidget *parent ):
    QwtAbstractScale( parent )
{
    d_data = new QwtAbstractSlider::PrivateData;

    setScale( 0.0, 100.0 );
    setFocusPolicy( Qt::StrongFocus );
}

//! Destructor
QwtAbstractSlider::~QwtAbstractSlider()
{
    delete d_data;
}

/*! 
  Set the value to be valid/invalid

  \param on When true, the value is invalidated

  \sa setValue()
*/
void QwtAbstractSlider::setValid( bool on )
{
    if ( on != d_data->isValid )
    {
        d_data->isValid = on;
        sliderChange();

        Q_EMIT valueChanged( d_data->value );
    }   
}   

//! \return True, when the value is invalid
bool QwtAbstractSlider::isValid() const
{
    return d_data->isValid;
}   

/*!
  En/Disable read only mode

  In read only mode the slider can't be controlled by mouse
  or keyboard.

  \param on Enables in case of true
  \sa isReadOnly()
*/
void QwtAbstractSlider::setReadOnly( bool on )
{
    if ( d_data->readOnly != on )
    {
        d_data->readOnly = on;
        update();
    }
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
  signal while the movable part of the slider is being dragged. 
  If tracking is disabled, the slider emits the valueChanged() signal 
  only when the user releases the slider.

  Tracking is enabled by default.
  \param on \c true (enable) or \c false (disable) tracking.

  \sa isTracking(), sliderMoved()
*/
void QwtAbstractSlider::setTracking( bool on )
{
    d_data->isTracking = on;
}

/*!
  \return True, when tracking has been enabled
  \sa setTracking()
*/
bool QwtAbstractSlider::isTracking() const
{
    return d_data->isTracking;
}

/*!
   Mouse press event handler
   \param event Mouse event
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
        d_data->pendingValueChanged = false;

        Q_EMIT sliderPressed();
    }
}

/*!
   Mouse Move Event handler
   \param event Mouse event
*/
void QwtAbstractSlider::mouseMoveEvent( QMouseEvent *event )
{
    if ( isReadOnly() )
    {
        event->ignore();
        return;
    }

    if ( d_data->isValid && d_data->isScrolling )
    {
        double value = scrolledTo( event->pos() );
        if ( value != d_data->value )
        {
            value = boundedValue( value );

            if ( d_data->stepAlignment )
            {
                value = alignedValue( value );
            }
            else
            {
                value = qwtAlignToScaleDiv( this, value );
            }

            if ( value != d_data->value )
            {
                d_data->value = value;

                sliderChange();

                Q_EMIT sliderMoved( d_data->value );

                if ( d_data->isTracking )
                    Q_EMIT valueChanged( d_data->value );
                else
                    d_data->pendingValueChanged = true;
            }
        }
    }
}

/*!
   Mouse Release Event handler
   \param event Mouse event
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

    int numSteps = 0;

    if ( ( event->modifiers() & Qt::ControlModifier) ||
        ( event->modifiers() & Qt::ShiftModifier ) )
    {
        // one page regardless of delta
        numSteps = d_data->pageSteps;
        if ( event->delta() < 0 )
            numSteps = -numSteps;
    }
    else
    {
        const int numTurns = ( event->delta() / 120 );
        numSteps = numTurns * d_data->singleSteps;
    }

    const double value = incrementedValue( d_data->value, numSteps );
    if ( value != d_data->value )
    {
        d_data->value = value;
        sliderChange();

        Q_EMIT sliderMoved( d_data->value );
        Q_EMIT valueChanged( d_data->value );
    }
}

/*!
  Handles key events

  \param event Key event
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

    int numSteps = 0;
    double value = d_data->value;

#if 1
    // better use key mapping from QAbstractSlider or QDial
    switch ( event->key() )
    {
        case Qt::Key_Down:
        {
            numSteps = d_data->singleSteps;
            if ( isInverted() )
                numSteps = -numSteps;
            break;
        }
        case Qt::Key_Left:
        {
            numSteps = -d_data->singleSteps;
            if ( isInverted() )
                numSteps = -numSteps;

            break;
        }
        case Qt::Key_Up:
        {
            numSteps = -d_data->singleSteps;
            if ( isInverted() )
                numSteps = -numSteps;

            break;
        }
        case Qt::Key_Right:
        {
            numSteps = d_data->singleSteps;
            if ( isInverted() )
                numSteps = -numSteps;

            break;
        }
        case Qt::Key_PageUp:
        {
            numSteps = d_data->pageSteps;
            break;
        }
        case Qt::Key_PageDown:
        {
            numSteps = -d_data->pageSteps;
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

    if ( numSteps != 0 )
    {
        value = incrementedValue( d_data->value, numSteps );
    }

    if ( value != d_data->value )
    {
        d_data->value = value;
        sliderChange();

        Q_EMIT sliderMoved( d_data->value );
        Q_EMIT valueChanged( d_data->value );
    }
}

/*!
  \brief Set the number of steps

  The range of the slider is divided into a number of steps from
  which the value increments according to user inputs depend. 

  \param stepCount Number of steps

  \sa totalSteps(), setSingleSteps(), setPageSteps()
 */
void QwtAbstractSlider::setTotalSteps( uint stepCount )
{
    d_data->totalSteps = stepCount;
}

/*!
  \return Number of steps
  \sa setTotalSteps(), singleSteps(), pageSteps()
 */
uint QwtAbstractSlider::totalSteps() const
{
    return d_data->totalSteps;
}

/*!
  \brief Set the number of steps for a single increment

  The range of the slider is divided into a number of steps from
  which the value increments according to user inputs depend. 

  \param stepCount Number of steps

  \sa singleSteps(), setTotalSteps(), setPageSteps()
 */

void QwtAbstractSlider::setSingleSteps( uint stepCount )
{
    d_data->singleSteps = stepCount;
}   

/*!
  \return Number of steps
  \sa setSingleSteps(), totalSteps(), pageSteps()
 */
uint QwtAbstractSlider::singleSteps() const
{
    return d_data->singleSteps;
}   

/*! 
  \brief Set the number of steps for a page increment
    
  The range of the slider is divided into a number of steps from
  which the value increments according to user inputs depend. 

  \param stepCount Number of steps

  \sa pageSteps(), setTotalSteps(), setSingleSteps()
 */

void QwtAbstractSlider::setPageSteps( uint stepCount )
{
    d_data->pageSteps = stepCount;
}

/*!
  \return Number of steps
  \sa setPageSteps(), totalSteps(), singleSteps()
 */
uint QwtAbstractSlider::pageSteps() const
{
    return d_data->pageSteps;
}

/*!
  \brief Enable step alignment

  When step alignment is enabled values resulting from slider
  movements are aligned to the step size.

  \param on Enable step alignment when true
  \sa stepAlignment()
*/
void QwtAbstractSlider::setStepAlignment( bool on )
{   
    if ( on != d_data->stepAlignment )
    {
        d_data->stepAlignment = on;
    }
}   
    
/*!
  \return True, when step alignment is enabled
  \sa setStepAlignment()
 */
bool QwtAbstractSlider::stepAlignment() const
{
    return d_data->stepAlignment;
}

/*!
  Set the slider to the specified value

  \param value New value
  \sa setValid(), sliderChange(), valueChanged()
*/
void QwtAbstractSlider::setValue( double value )
{
    value = qBound( minimum(), value, maximum() );

    const bool changed = ( d_data->value != value ) || !d_data->isValid;

    d_data->value = value;
    d_data->isValid = true;

    if ( changed )
    {
        sliderChange();
        Q_EMIT valueChanged( d_data->value );
    }
}

//! Returns the current value.
double QwtAbstractSlider::value() const
{
    return d_data->value;
}

/*!
  If wrapping is true stepping up from upperBound() value will 
  take you to the minimum() value and vica versa. 

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

void QwtAbstractSlider::incrementValue( int stepCount )
{
    const double value = incrementedValue( 
        d_data->value, stepCount );

    if ( value != d_data->value )
    {
        d_data->value = value;
        sliderChange();
    }
}

/*!
  Increment a value 

  \sa value Value 
  \sa stepCount Number of steps

  \return Incremented value
 */
double QwtAbstractSlider::incrementedValue( 
    double value, int stepCount ) const
{
    if ( d_data->totalSteps == 0 )
        return value;

    const QwtTransform *transformation =
        scaleMap().transformation();

    if ( transformation == NULL )
    {
        const double range = maximum() - minimum();
        value += stepCount * range / d_data->totalSteps;
    }
    else
    {
        QwtScaleMap map = scaleMap();
        map.setPaintInterval( 0, d_data->totalSteps );

        // we need equidant steps according to
        // paint device coordinates
        const double range = transformation->transform( maximum() ) 
            - transformation->transform( minimum() );

        const double stepSize = range / d_data->totalSteps;

        double v = transformation->transform( value );

        v = qRound( v / stepSize ) * stepSize; 
        v += stepCount * range / d_data->totalSteps;

        value = transformation->invTransform( v );
    }

    value = boundedValue( value );

    if ( d_data->stepAlignment )
        value = alignedValue( value );

    return value;
}

double QwtAbstractSlider::boundedValue( double value ) const
{
    const double vmin = minimum();
    const double vmax = maximum();

    if ( d_data->wrapping && vmin != vmax )
    {
        const int fullCircle = 360 * 16;

        const double pd = scaleMap().pDist();
        if ( int( pd / fullCircle ) * fullCircle == pd )
        {
            // full circle scales: min and max are the same
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
            if ( value < vmin )
                value = vmax;
            else if ( value > vmax )
                value = vmin;
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
    if ( d_data->totalSteps == 0 )
        return value;

    if ( scaleMap().transformation() == NULL )
    {
        const double stepSize = 
            ( maximum() - minimum() ) / d_data->totalSteps;

        if ( stepSize > 0.0 )
        {
            value = lowerBound() + 
                qRound( ( value - lowerBound() ) / stepSize ) * stepSize;
        }
    }
    else
    {
        const double stepSize = 
            ( scaleMap().p2() - scaleMap().p1() ) / d_data->totalSteps;

        if ( stepSize > 0.0 )
        {
            double v = scaleMap().transform( value );

            v = scaleMap().p1() +
                qRound( ( v - scaleMap().p1() ) / stepSize ) * stepSize;

            value = scaleMap().invTransform( v );
        }
    }

    // correct rounding error if value = 0
    if ( qFuzzyCompare( value + 1.0, 1.0 ) )
    {
        value = 0.0;
    }
    else
    {
        // correct rounding error at the border
        if ( qFuzzyCompare( value, upperBound() ) )
            value = upperBound();
        else if ( qFuzzyCompare( value, lowerBound() ) )
            value = lowerBound();
    }

    return value;
}

/*!
  Update the slider according to modifications of the scale
 */
void QwtAbstractSlider::scaleChange()
{
    const double value = qBound( minimum(), value, maximum() );

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

//! Calling update()
void QwtAbstractSlider::sliderChange()
{
    update();
}
