/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_wheel.h"
#include "qwt_math.h"
#include "qwt_painter.h"
#include <qevent.h>
#include <qdrawutil.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qapplication.h>
#include <qdatetime.h>

#if QT_VERSION < 0x040601
#define qFabs(x) ::fabs(x)
#define qFastSin(x) ::sin(x)
#define qExp(x) ::exp(x)
#endif

class QwtWheel::PrivateData
{
public:
    PrivateData():
        orientation( Qt::Horizontal ),
        viewAngle( 175.0 ),
        totalAngle( 360.0 ),
        tickCnt( 10 ),
        wheelBorderWidth( 2 ),
        borderWidth( 2 ),
        wheelWidth( 20 ),
        isScrolling( false ),
        initialScrollOffset( 0.0 ),
        tracking( true ),
        timerId( 0 ),
        updateInterval( 50 ),
        mass( 0.0 ),
        minimum( 0.0 ),
        maximum( 0.0 ),
        singleStep( 1.0 ),
        pageSize( 1 ),
        value( 0.0 ),
        exactValue( 0.0 ),
        wrapping( false )
    {
    };

    Qt::Orientation orientation;
    double viewAngle;
    double totalAngle;
    int tickCnt;
    int wheelBorderWidth;
    int borderWidth;
    int wheelWidth;

    bool isScrolling;
    double initialScrollOffset;
    bool tracking;

    int timerId;
    int updateInterval;
    QTime time;
    double speed;
    double mass;

    double minimum;
    double maximum;
    double singleStep;
    int pageSize;

    double value;
    double exactValue;

    bool wrapping;
};

//! Constructor
QwtWheel::QwtWheel( QWidget *parent ):
    QWidget( parent )
{
    d_data = new PrivateData;

    setFocusPolicy( Qt::StrongFocus );
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    setAttribute( Qt::WA_WState_OwnSizePolicy, false );
}

//! Destructor
QwtWheel::~QwtWheel()
{
    delete d_data;
}

void QwtWheel::setTracking( bool enable )
{
    d_data->tracking = enable;
}

bool QwtWheel::isTracking() const
{
    return d_data->tracking;
}

void QwtWheel::setUpdateInterval( int interval )
{
    d_data->updateInterval = qMax( interval, 50 );
}

int QwtWheel::updateInterval() const
{
    return d_data->updateInterval;
}

void QwtWheel::mousePressEvent( QMouseEvent *event )
{
    stopFlying();

    d_data->isScrolling = wheelRect().contains( event->pos() );

    if ( d_data->isScrolling )
    {
        d_data->time.start();
        d_data->speed = 0;
        d_data->initialScrollOffset = valueAt( event->pos() ) - d_data->value;

        Q_EMIT wheelPressed();
    }
}

void QwtWheel::mouseMoveEvent( QMouseEvent *e )
{
    if ( d_data->isScrolling )
    {
        const double exactPrevValue = d_data->exactValue;
        const double newValue =
            valueAt( e->pos() ) - d_data->initialScrollOffset;

        const bool changed = updateValue( newValue );

        if ( d_data->mass > 0.0 )
        {
            const double ms = d_data->time.restart();
            d_data->speed = ( d_data->exactValue - exactPrevValue ) / qMax( ms, 1.0 );
        }

        if ( changed )
            Q_EMIT wheelMoved( d_data->value );
    }
}

void QwtWheel::mouseReleaseEvent( QMouseEvent *event )
{
    if ( d_data->isScrolling )
    {
        const double newValue =
            valueAt( event->pos() ) - d_data->initialScrollOffset;

        const bool changed = updateValue( newValue );

        d_data->initialScrollOffset = 0.0;

        if ( d_data->mass > 0.0 )
        {
            const int ms = d_data->time.elapsed();
            if ( ( qFabs( d_data->speed ) > 0.0 ) && ( ms < 50 ) )
                d_data->timerId = startTimer( d_data->updateInterval );
        }
        else
        {
            d_data->isScrolling = false;
            if ( ( !d_data->tracking ) || changed )
                Q_EMIT valueChanged( d_data->value );

        }
        Q_EMIT wheelReleased();
    }
}

void QwtWheel::wheelEvent( QWheelEvent *event )
{
    if ( !wheelRect().contains( event->pos() ) )
    {
        event->ignore();
        return;
    }

    stopFlying();

    const int numPages = event->delta() / 120;

    const double stepSize = qAbs( d_data->singleStep );
    const double off = stepSize * d_data->pageSize * numPages;

    const bool changed = updateValue( d_data->value + off );
    if ( changed )
    {
        Q_EMIT wheelMoved( d_data->value );
    }
}

void QwtWheel::keyPressEvent( QKeyEvent *e )
{
    const double stepSize = qAbs( d_data->singleStep );
    double value = d_data->value;

    switch ( e->key() )
    {
        case Qt::Key_Down:
        {
            if ( d_data->orientation == Qt::Vertical )
                value -= stepSize;

            break;
        }
        case Qt::Key_Left:
        {
            if ( d_data->orientation == Qt::Horizontal )
                value -= stepSize;

            break;
        }
        case Qt::Key_Up:
        {
            if ( d_data->orientation == Qt::Vertical )
                value += stepSize;
            break;
        }
        case Qt::Key_Right:
        {
            if ( d_data->orientation == Qt::Horizontal )
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

        const bool changed = updateValue( value );
        if ( changed )
        {
            Q_EMIT wheelMoved( d_data->value );
        }
    }
}

void QwtWheel::timerEvent( QTimerEvent *event )
{
    if ( event->timerId() == d_data->timerId )
    {
        if ( d_data->mass <= 0.0 )
        {
            killTimer( d_data->timerId );
            d_data->timerId = 0;
            return;
        }

        const double intv = d_data->updateInterval;

        d_data->speed *= qExp( -intv * 0.001 / d_data->mass );
        const bool changed = updateValue( d_data->exactValue + d_data->speed * intv );

        // stop if d_data->speed < one step per second
        if ( qFabs( d_data->speed ) < 0.001 * qAbs( d_data->singleStep ) )
        {
            d_data->speed = 0.0;

            killTimer( d_data->timerId );
            d_data->timerId = 0;

            if ( ( !d_data->tracking ) || changed )
                Q_EMIT valueChanged( d_data->value );
        }

        return;
    }

    QWidget::timerEvent( event );
}

/*!
  \brief Adjust the number of grooves in the wheel's surface.

  The number of grooves is limited to 6 <= cnt <= 50.
  Values outside this range will be clipped.
  The default value is 10.

  \param cnt Number of grooves per 360 degrees
  \sa tickCnt()
*/
void QwtWheel::setTickCnt( int cnt )
{
    d_data->tickCnt = qBound( 6, cnt, 50 );
    update();
}

/*!
  \return Number of grooves in the wheel's surface.
  \sa setTickCnt()
*/
int QwtWheel::tickCnt() const
{
    return d_data->tickCnt;
}

/*!
  \brief Set the wheel border width of the wheel.

  The wheel border must not be smaller than 1
  and is limited in dependence on the wheel's size.
  Values outside the allowed range will be clipped.

  The wheel border defaults to 2.

  \param borderWidth Border width
  \sa internalBorder()
*/
void QwtWheel::setWheelBorderWidth( int borderWidth )
{
    const int d = qMin( width(), height() ) / 3;
    borderWidth = qMin( borderWidth, d );
    d_data->wheelBorderWidth = qMax( borderWidth, 1 );
    update();
}

/*!
   \return Wheel border width 
   \sa setWheelBorderWidth()
*/
int QwtWheel::wheelBorderWidth() const
{
    return d_data->wheelBorderWidth;
}

/*!
  \brief Set the border width 

  The border defaults to 2.

  \param width Border width
  \sa borderWidth()
*/
void QwtWheel::setBorderWidth( int width )
{
    d_data->borderWidth = qMax( width, 0 );
    update();
}

/*!
   \return Border width 
   \sa setBorderWidth()
*/
int QwtWheel::borderWidth() const
{
    return d_data->borderWidth;
}

/*!
   \return Rectangle of the wheel without the outer border
*/
QRect QwtWheel::wheelRect() const
{
    const int bw = d_data->borderWidth;
    return contentsRect().adjusted( bw, bw, -bw, -bw );
}

/*!
  \brief Set the total angle which the wheel can be turned.

  One full turn of the wheel corresponds to an angle of
  360 degrees. A total angle of n*360 degrees means
  that the wheel has to be turned n times around its axis
  to get from the minimum value to the maximum value.

  The default setting of the total angle is 360 degrees.

  \param angle total angle in degrees
  \sa totalAngle()
*/
void QwtWheel::setTotalAngle( double angle )
{
    if ( angle < 0.0 )
        angle = 0.0;

    d_data->totalAngle = angle;
    update();
}

/*!
  \return Total angle which the wheel can be turned.
  \sa setTotalAngle()
*/
double QwtWheel::totalAngle() const
{
    return d_data->totalAngle;
}

/*!
  \brief Set the wheel's orientation.
  \param o Orientation. Allowed values are
           Qt::Horizontal and Qt::Vertical.
   Defaults to Qt::Horizontal.
  \sa orientation()
*/
void QwtWheel::setOrientation( Qt::Orientation orientation )
{
    if ( d_data->orientation == orientation )
        return;

    if ( !testAttribute( Qt::WA_WState_OwnSizePolicy ) )
    {
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy( sp );

        setAttribute( Qt::WA_WState_OwnSizePolicy, false );
    }

    d_data->orientation = orientation;
    update();
}

/*!
  \return Orientation
  \sa setOrientation()
*/
Qt::Orientation QwtWheel::orientation() const
{
    return d_data->orientation;
}

/*!
  \brief Specify the visible portion of the wheel.

  You may use this function for fine-tuning the appearance of
  the wheel. The default value is 175 degrees. The value is
  limited from 10 to 175 degrees.

  \param angle Visible angle in degrees
  \sa viewAngle(), setTotalAngle()
*/
void QwtWheel::setViewAngle( double angle )
{
    d_data->viewAngle = qBound( 10.0, angle, 175.0 );
    update();
}

/*!
  \return Visible portion of the wheel
  \sa setViewAngle(), totalAngle()
*/
double QwtWheel::viewAngle() const
{
    return d_data->viewAngle;
}

//! Determine the value corresponding to a specified point
double QwtWheel::valueAt( const QPoint &pos )
{
    const QRectF rect = wheelRect();

    // The reference position is arbitrary, but the
    // sign of the offset is important
    double w, dx;
    if ( d_data->orientation == Qt::Vertical )
    {
        w = rect.height();
        dx = rect.y() - pos.y();
    }
    else
    {
        w = rect.width();
        dx = pos.x() - rect.x();
    }

    if ( w == 0.0 )
        return 0.0;

    // w pixels is an arc of viewAngle degrees,
    // so we convert change in pixels to change in angle
    const double ang = dx * d_data->viewAngle / w;

    // value range maps to totalAngle degrees,
    // so convert the change in angle to a change in value
    const double val = ang * ( maximum() - minimum() ) / d_data->totalAngle;

    return val;
}

/*! 
   \brief Qt Paint Event
   \param event Paint event
*/
void QwtWheel::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    qDrawShadePanel( &painter, 
        contentsRect(), palette(), true, d_data->borderWidth );

    drawWheelBackground( &painter, wheelRect() );
    drawTicks( &painter, wheelRect() );

    if ( hasFocus() )
        QwtPainter::drawFocusRect( &painter, this );
}

/*!
   Draw the Wheel's background gradient

   \param painter Painter
   \param rect Rectangle for the wheel
*/
void QwtWheel::drawWheelBackground( 
    QPainter *painter, const QRectF &rect )
{
    painter->save();

    QPalette pal = palette();

    //  draw shaded background
    QLinearGradient gradient( rect.topLeft(), 
        ( d_data->orientation == Qt::Horizontal ) ? rect.topRight() : rect.bottomLeft() );
    gradient.setColorAt( 0.0, pal.color( QPalette::Button ) );
    gradient.setColorAt( 0.2, pal.color( QPalette::Light ) );
    gradient.setColorAt( 0.7, pal.color( QPalette::Mid ) );
    gradient.setColorAt( 1.0, pal.color( QPalette::Dark ) );

    painter->fillRect( rect, gradient );

    // draw internal border

    const QPen lightPen( palette().color( QPalette::Light ), 
        d_data->wheelBorderWidth, Qt::SolidLine, Qt::FlatCap );
    const QPen darkPen( pal.color( QPalette::Dark ), 
        d_data->wheelBorderWidth, Qt::SolidLine, Qt::FlatCap );

    const double bw2 = 0.5 * d_data->wheelBorderWidth;

    if ( d_data->orientation == Qt::Horizontal )
    {
        painter->setPen( lightPen );
        painter->drawLine( QPointF( rect.left(), rect.top() + bw2 ), 
            QPointF( rect.right(), rect.top() + bw2 ) );

        painter->setPen( darkPen );
        painter->drawLine( QPointF( rect.left(), rect.bottom() - bw2 ), 
            QPointF( rect.right(), rect.bottom() - bw2 ) );
    }
    else // Qt::Vertical
    {
        painter->setPen( lightPen );
        painter->drawLine( QPointF( rect.left() + bw2, rect.top() ), 
            QPointF( rect.left() + bw2, rect.bottom() ) );

        painter->setPen( darkPen );
        painter->drawLine( QPointF( rect.right() - bw2, rect.top() ), 
            QPointF( rect.right() - bw2, rect.bottom() ) );
    }

    painter->restore();
}

/*!
   Draw the Wheel's ticks

   \param painter Painter
   \param rect Rectangle for the wheel
*/
void QwtWheel::drawTicks( QPainter *painter, const QRectF &rect )
{
    if ( maximum() == minimum() || d_data->totalAngle == 0.0 )
    {
        return;
    }

    const QPen lightPen( palette().color( QPalette::Light ), 
        0, Qt::SolidLine, Qt::FlatCap );
    const QPen darkPen( palette().color( QPalette::Dark ), 
        0, Qt::SolidLine, Qt::FlatCap );

    const double sign = ( minimum() < maximum() ) ? 1.0 : -1.0;
    const double cnvFactor = qAbs( d_data->totalAngle / ( maximum() - minimum() ) );
    const double halfIntv = 0.5 * d_data->viewAngle / cnvFactor;
    const double loValue = value() - halfIntv;
    const double hiValue = value() + halfIntv;
    const double tickWidth = 360.0 / double( d_data->tickCnt ) / cnvFactor;
    const double sinArc = qFastSin( d_data->viewAngle * M_PI / 360.0 );

    if ( d_data->orientation == Qt::Horizontal )
    {
        const double halfSize = rect.width() * 0.5;

        double l1 = rect.top() + d_data->wheelBorderWidth;
        double l2 = rect.bottom() - d_data->wheelBorderWidth - 1;

        // draw one point over the border if border > 1
        if ( d_data->wheelBorderWidth > 1 )
        {
            l1--;
            l2++;
        }

        const double maxpos = rect.right() - 2;
        const double minpos = rect.left() + 2;

        // draw tick marks
        for ( double tickValue = ::ceil( loValue / tickWidth ) * tickWidth;
            tickValue < hiValue; tickValue += tickWidth )
        {
            const double angle = ( tickValue - value() ) * M_PI / 180.0;
            const double s = qFastSin( angle * cnvFactor );

            const double tickPos = 
                rect.right() - halfSize * ( sinArc + sign * s ) / sinArc;

            if ( ( tickPos <= maxpos ) && ( tickPos > minpos ) )
            {
                painter->setPen( darkPen );
                painter->drawLine( QPointF( tickPos - 1 , l1 ), 
                    QPointF( tickPos - 1,  l2 ) );
                painter->setPen( lightPen );
                painter->drawLine( QPointF( tickPos, l1 ), 
                    QPointF( tickPos, l2 ) );
            }
        }
    }
    else // Qt::Vertical
    {
        const double halfSize = rect.height() * 0.5;

        double l1 = rect.left() + d_data->wheelBorderWidth;
        double l2 = rect.right() - d_data->wheelBorderWidth - 1;

        if ( d_data->wheelBorderWidth > 1 )
        {
            l1--;
            l2++;
        }

        const double maxpos = rect.bottom() - 2;
        const double minpos = rect.top() + 2;

        for ( double tickValue = ::ceil( loValue / tickWidth ) * tickWidth;
            tickValue < hiValue; tickValue += tickWidth )
        {
            const double angle = ( tickValue - value() ) * M_PI / 180.0;
            const double s = qFastSin( angle * cnvFactor );

            const double tickPos = 
                rect.y() + halfSize * ( sinArc + sign * s ) / sinArc;

            if ( ( tickPos <= maxpos ) && ( tickPos > minpos ) )
            {
                painter->setPen( darkPen );
                painter->drawLine( QPointF( l1, tickPos - 1 ), 
                    QPointF( l2, tickPos - 1 ) );
                painter->setPen( lightPen );
                painter->drawLine( QPointF( l1, tickPos ), 
                    QPointF( l2, tickPos ) );
            }
        }
    }
}

/*!
  \brief Set the width of the wheel

  Corresponds to the wheel height for horizontal orientation,
  and the wheel width for vertical orientation.

  \param width the wheel's width
  \sa wheelWidth()
*/
void QwtWheel::setWheelWidth( int width )
{
    d_data->wheelWidth = width;
    update();
}

/*!
  \return Width of the wheel
  \sa setWheelWidth()
*/
int QwtWheel::wheelWidth() const
{
    return d_data->wheelWidth;
}

/*!
  \return a size hint
*/
QSize QwtWheel::sizeHint() const
{
    const QSize hint = minimumSizeHint();
    return hint.expandedTo( QApplication::globalStrut() );
}

/*!
  \brief Return a minimum size hint
  \warning The return value is based on the wheel width.
*/
QSize QwtWheel::minimumSizeHint() const
{
    QSize sz( 3 * d_data->wheelWidth + 2 * d_data->borderWidth,
        d_data->wheelWidth + 2 * d_data->borderWidth );
    if ( d_data->orientation != Qt::Horizontal )
        sz.transpose();

    return sz;
}

void QwtWheel::setRange( double minimum, double maximum )
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

    if ( changed )
    {
        update();
        Q_EMIT valueChanged( d_data->value );
    }
}

void QwtWheel::setSingleStep( double vstep )
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

double QwtWheel::singleStep() const
{
    return d_data->singleStep;
}

void QwtWheel::setMaximum( double max )
{
    setRange( minimum(), max );
}

double QwtWheel::maximum() const
{
    return d_data->maximum;
}

void QwtWheel::setMinimum( double min )
{
    setRange( min, maximum() );
}

double QwtWheel::minimum() const
{
    return d_data->minimum;
}

void QwtWheel::setPageSize( int pageSize )
{
#if 1
    // limit page size
    const int max =
        int( qAbs( ( d_data->maximum - d_data->minimum ) / d_data->singleStep ) );
    d_data->pageSize = qBound( 0, pageSize, max );
#endif
}

int QwtWheel::pageSize() const
{
    return d_data->pageSize;
}

double QwtWheel::value() const
{
    return d_data->value;
}

void QwtWheel::setWrapping( bool on )
{
    d_data->wrapping = on;
}

bool QwtWheel::wrapping() const
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
void QwtWheel::setMass( double mass )
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
double QwtWheel::mass() const
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
void QwtWheel::setValue( double value )
{
    stopFlying();

    const double vmin = qMin( d_data->minimum, d_data->maximum );
    const double vmax = qMax( d_data->minimum, d_data->maximum );

    value = qBound( vmin, value, vmax );

    const bool changed = d_data->value != value;

    d_data->value = value;
    d_data->exactValue = value;

    if ( changed )
    {
        update();
        Q_EMIT valueChanged( d_data->value );
    }
}

void QwtWheel::stopFlying()
{
    if ( d_data->timerId != 0 )
    {
        killTimer( d_data->timerId );
        d_data->timerId = 0;
    }
}

bool QwtWheel::setNewValue( double value )
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

bool QwtWheel::updateValue( double value )
{
    const bool changed = setNewValue( value );
    if ( changed )
    {
        update();

        if ( d_data->tracking )
            Q_EMIT valueChanged( d_data->value );
    }

    return changed;
}


