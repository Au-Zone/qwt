/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_thermo.h"
#include "qwt_math.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_map.h"
#include <qpainter.h>
#include <qevent.h>
#include <qstyle.h>
#include <qpixmap.h>
#include <qdrawutil.h>
#include <qalgorithms.h>
#include <qmath.h>
#include <qstyle.h>
#include <qstyleoption.h>

class QwtThermo::PrivateData
{
public:
    PrivateData():
        fillBrush( Qt::black ),
        alarmBrush( Qt::white ),
        orientation( Qt::Vertical ),
        scalePos( QwtThermo::LeftScale ),
        borderWidth( 2 ),
        scaleDist( 3 ),
        pipeWidth( 10 ),
        minValue( 0.0 ),
        maxValue( 1.0 ),
        value( 0.0 ),
        alarmLevel( 0.0 ),
        alarmEnabled( false )
    {
        map.setScaleInterval( minValue, maxValue );
    }

    QwtScaleMap map;
    QBrush fillBrush;
    QBrush alarmBrush;

    Qt::Orientation orientation;
    ScalePos scalePos;
    int borderWidth;
    int scaleDist;
    int pipeWidth;

    double minValue;
    double maxValue;
    double value;
    double alarmLevel;
    bool alarmEnabled;
};

/*!
  Constructor
  \param parent Parent widget
*/
QwtThermo::QwtThermo( QWidget *parent ):
    QWidget( parent )
{
    d_data = new PrivateData;
    setRange( d_data->minValue, d_data->maxValue, false );

    QSizePolicy policy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
    if ( d_data->orientation == Qt::Vertical )
        policy.transpose();

    setSizePolicy( policy );

    setAttribute( Qt::WA_WState_OwnSizePolicy, false );
}

//! Destructor
QwtThermo::~QwtThermo()
{
    delete d_data;
}

/*!
  Set the maximum value.

  \param max Maximum value
  \sa maxValue(), setMinValue()
*/
void QwtThermo::setMaxValue( double max )
{
    setRange( d_data->minValue, max );
}

//! Return the maximum value.
double QwtThermo::maxValue() const
{
    return d_data->maxValue;
}

/*!
  Set the minimum value.

  \param min Minimum value
  \sa minValue(), setMaxValue()
*/
void QwtThermo::setMinValue( double min )
{
    setRange( min, d_data->maxValue );
}

//! Return the minimum value.
double QwtThermo::minValue() const
{
    return d_data->minValue;
}

/*!
  Set the current value.

  \param value New Value
  \sa value()
*/
void QwtThermo::setValue( double value )
{
    if ( d_data->value != value )
    {
        d_data->value = value;
        update();
    }
}

//! Return the value.
double QwtThermo::value() const
{
    return d_data->value;
}

/*!
  \brief Set a scale draw

  For changing the labels of the scales, it
  is necessary to derive from QwtScaleDraw and
  overload QwtScaleDraw::label().

  \param scaleDraw ScaleDraw object, that has to be created with
                   new and will be deleted in ~QwtThermo or the next
                   call of setScaleDraw().
*/
void QwtThermo::setScaleDraw( QwtScaleDraw *scaleDraw )
{
    setAbstractScaleDraw( scaleDraw );
}

/*!
   \return the scale draw of the thermo
   \sa setScaleDraw()
*/
const QwtScaleDraw *QwtThermo::scaleDraw() const
{
    return static_cast<const QwtScaleDraw *>( abstractScaleDraw() );
}

/*!
   \return the scale draw of the thermo
   \sa setScaleDraw()
*/
QwtScaleDraw *QwtThermo::scaleDraw()
{
    return static_cast<QwtScaleDraw *>( abstractScaleDraw() );
}

/*!
  Qt paint event.
  event Paint event
*/
void QwtThermo::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    const QRect tRect = pipeRect();

    if ( !tRect.contains( event->rect() ) )
    {
        if ( d_data->scalePos != NoScale )
            scaleDraw()->draw( &painter, palette() );
    }

    const int bw = d_data->borderWidth;

    qDrawShadePanel( &painter, 
        tRect.adjusted( -bw, -bw, bw, bw ),
        palette(), true, bw, NULL );

    drawThermo( &painter, tRect );
}

//! Qt resize event handler
void QwtThermo::resizeEvent( QResizeEvent * )
{
    layoutThermo( false );
}

//! Qt change event handler
void QwtThermo::changeEvent( QEvent *event )
{
    switch( event->type() )
    {
        case QEvent::StyleChange:
        case QEvent::FontChange:
        {
            layoutThermo( true );
            break;
        }
        default:
            break;
    }
}

/*!
  Recalculate the QwtThermo geometry and layout based on
  the QwtThermo::rect() and the fonts.

  \param update_geometry notify the layout system and call update
         to redraw the scale
*/
void QwtThermo::layoutThermo( bool update_geometry )
{
    const QRect tRect = pipeRect();

    const int bw = d_data->borderWidth;
    const int sd = d_data->scaleDist;

    if ( d_data->orientation == Qt::Horizontal )
    {
        switch ( d_data->scalePos )
        {
            case TopScale:
            {
                scaleDraw()->setAlignment( QwtScaleDraw::TopScale );
                scaleDraw()->move( tRect.x(), tRect.y() - bw - sd );
                scaleDraw()->setLength( tRect.width() );
                break;
            }

            case BottomScale:
            case NoScale: 
            default:
            {
                scaleDraw()->setAlignment( QwtScaleDraw::BottomScale );
                scaleDraw()->move( tRect.left(), tRect.bottom() + bw + sd );
                scaleDraw()->setLength( tRect.width() );
                break;
            }
        }
        d_data->map.setPaintInterval( tRect.left(), tRect.right() - 1 );
    }
    else // Qt::Vertical
    {
        switch ( d_data->scalePos )
        {
            case RightScale:
            {
                scaleDraw()->setAlignment( QwtScaleDraw::RightScale );
                scaleDraw()->move( tRect.right() + bw + sd, tRect.top() );
                scaleDraw()->setLength( tRect.height() );
                break;
            }

            case LeftScale:
            case NoScale: 
            default:
            {
                scaleDraw()->setAlignment( QwtScaleDraw::LeftScale );
                scaleDraw()->move( tRect.x() - sd - bw, tRect.top() );
                scaleDraw()->setLength( tRect.height() );
                break;
            }
        }
        d_data->map.setPaintInterval( tRect.bottom() - 1, tRect.top() );
    }

    if ( update_geometry )
    {
        updateGeometry();
        update();
    }
}

QRect QwtThermo::pipeRect() const
{
    const QRect r = rect();

    int mbd = 0;
    if ( d_data->scalePos != NoScale )
    {
        int d1, d2;
        scaleDraw()->getBorderDistHint( font(), d1, d2 );
        mbd = qMax( d1, d2 );
    }
    int bw = d_data->borderWidth;

    QRect tRect;
    if ( d_data->orientation == Qt::Horizontal )
    {
        switch ( d_data->scalePos )
        {
            case TopScale:
            {
                tRect.setRect(
                    r.x() + mbd + bw,
                    r.y() + r.height() - d_data->pipeWidth - 2 * bw,
                    r.width() - 2 * ( bw + mbd ),
                    d_data->pipeWidth 
                );
                break;
            }

            case BottomScale:
            case NoScale: 
            default:   
            {
                tRect.setRect(
                    r.x() + mbd + bw,
                    r.y() + d_data->borderWidth,
                    r.width() - 2 * ( bw + mbd ),
                    d_data->pipeWidth 
                );
                break;
            }
        }
    }
    else // Qt::Vertical
    {
        switch ( d_data->scalePos )
        {
            case RightScale:
            {
                tRect.setRect(
                    r.x() + bw,
                    r.y() + mbd + bw,
                    d_data->pipeWidth,
                    r.height() - 2 * ( bw + mbd ) 
                );
                break;
            }
            case LeftScale:
            case NoScale: // like Left but without scale
            default:   // inconsistent orientation and scale position
                // Mapping between values and pixels requires
                // initialization of the scale geometry
            {
                tRect.setRect(
                    r.x() + r.width() - 2 * bw - d_data->pipeWidth,
                    r.y() + mbd + bw,
                    d_data->pipeWidth,
                    r.height() - 2 * ( bw + mbd ) );
                break;
            }
        }
    }

    return tRect;
}

/*!
   \brief Set the thermometer orientation and the scale position.

   The scale position NoScale disables the scale.
   \param o orientation. Possible values are Qt::Horizontal and Qt::Vertical.
         The default value is Qt::Vertical.
   \param s Position of the scale.
         The default value is NoScale.

   A valid combination of scale position and orientation is enforced:
   - a horizontal thermometer can have the scale positions TopScale,
     BottomScale or NoScale;
   - a vertical thermometer can have the scale positions LeftScale,
     RightScale or NoScale;
   - an invalid scale position will default to NoScale.

   \sa setScalePosition()
*/
void QwtThermo::setOrientation( Qt::Orientation o, ScalePos s )
{
    if ( o == d_data->orientation && s == d_data->scalePos )
        return;

    switch ( o )
    {
        case Qt::Horizontal:
        {
            if ( ( s == NoScale ) || ( s == BottomScale ) || ( s == TopScale ) )
                d_data->scalePos = s;
            else
                d_data->scalePos = NoScale;
            break;
        }
        case Qt::Vertical:
        {
            if ( ( s == NoScale ) || ( s == LeftScale ) || ( s == RightScale ) )
                d_data->scalePos = s;
            else
                d_data->scalePos = NoScale;
            break;
        }
    }

    if ( o != d_data->orientation )
    {
        if ( !testAttribute( Qt::WA_WState_OwnSizePolicy ) )
        {
            QSizePolicy sp = sizePolicy();
            sp.transpose();
            setSizePolicy( sp );

            setAttribute( Qt::WA_WState_OwnSizePolicy, false );
        }
    }

    d_data->orientation = o;
    layoutThermo( true );
}

/*!
  \brief Change the scale position (and thermometer orientation).

  \param scalePos Position of the scale.

  A valid combination of scale position and orientation is enforced:
  - if the new scale position is LeftScale or RightScale, the
    scale orientation will become Qt::Vertical;
  - if the new scale position is BottomScale or TopScale, the scale
    orientation will become Qt::Horizontal;
  - if the new scale position is NoScale, the scale orientation will not change.

  \sa setOrientation(), scalePosition()
*/
void QwtThermo::setScalePosition( ScalePos scalePos )
{
    if ( ( scalePos == BottomScale ) || ( scalePos == TopScale ) )
        setOrientation( Qt::Horizontal, scalePos );
    else if ( ( scalePos == LeftScale ) || ( scalePos == RightScale ) )
        setOrientation( Qt::Vertical, scalePos );
    else
        setOrientation( d_data->orientation, NoScale );
}

/*!
   Return the scale position.
   \sa setScalePosition()
*/
QwtThermo::ScalePos QwtThermo::scalePosition() const
{
    return d_data->scalePos;
}

//! Notify a scale change.
void QwtThermo::scaleChange()
{
    layoutThermo( true );
}

/*!
   Redraw the liquid in thermometer pipe.
   \param painter Painter
*/
void QwtThermo::drawThermo( QPainter *painter, const QRect &thermoRect )
{
    double alarm = 0.0; 
    double taval = 0.0;

    QRectF fRect;
    QRectF aRect;
    QRectF bRect;

    const bool inverted = ( d_data->maxValue < d_data->minValue );

    //  Determine if value exceeds alarm threshold.
    //  Note: The alarm value is allowed to lie
    //        outside the interval (minValue, maxValue).
    if ( d_data->alarmEnabled )
    {
        if ( inverted )
        {
            alarm = ( ( d_data->alarmLevel >= d_data->maxValue )
                 && ( d_data->alarmLevel <= d_data->minValue )
                 && ( d_data->value >= d_data->alarmLevel ) );

        }
        else
        {
            alarm = ( ( d_data->alarmLevel >= d_data->minValue )
                 && ( d_data->alarmLevel <= d_data->maxValue )
                 && ( d_data->value >= d_data->alarmLevel ) );
        }
    }

    //  transform values
    int tval = transform( d_data->value );

    if ( alarm )
        taval = transform( d_data->alarmLevel );

    //  calculate recangles
    if ( d_data->orientation == Qt::Horizontal )
    {
        if ( inverted )
        {
            bRect.setRect( thermoRect.x(), thermoRect.y(),
                  tval - thermoRect.x(), thermoRect.height() );

            if ( alarm )
            {
                aRect.setRect( tval, thermoRect.y(),
                      taval - tval + 1, thermoRect.height() );
                fRect.setRect( taval + 1, thermoRect.y(),
                      thermoRect.x() + thermoRect.width() - ( taval + 1 ),
                      thermoRect.height() );
            }
            else
            {
                fRect.setRect( tval, thermoRect.y(),
                      thermoRect.x() + thermoRect.width() - tval,
                      thermoRect.height() );
            }
        }
        else
        {
            bRect.setRect( tval + 1, thermoRect.y(),
                  thermoRect.width() - ( tval + 1 - thermoRect.x() ),
                  thermoRect.height() );

            if ( alarm )
            {
                aRect.setRect( taval, thermoRect.y(),
                      tval - taval + 1,
                      thermoRect.height() );
                fRect.setRect( thermoRect.x(), thermoRect.y(),
                      taval - thermoRect.x(),
                      thermoRect.height() );
            }
            else
            {
                fRect.setRect( thermoRect.x(), thermoRect.y(),
                      tval - thermoRect.x() + 1,
                      thermoRect.height() );
            }

        }
    }
    else // Qt::Vertical
    {
        if ( tval < thermoRect.y() )
            tval = thermoRect.y();
        else
        {
            if ( tval > thermoRect.y() + thermoRect.height() )
                tval = thermoRect.y() + thermoRect.height();
        }

        if ( inverted )
        {
            bRect.setRect( thermoRect.x(), tval + 1,
                thermoRect.width(),
                thermoRect.height() - ( tval + 1 - thermoRect.y() ) );

            if ( alarm )
            {
                aRect.setRect( thermoRect.x(), taval,
                    thermoRect.width(), tval - taval + 1 );
                fRect.setRect( thermoRect.x(), thermoRect.y(),
                    thermoRect.width(),
                    taval - thermoRect.y() );
            }
            else
            {
                fRect.setRect( thermoRect.x(), thermoRect.y(),
                    thermoRect.width(), tval - thermoRect.y() + 1 );
            }
        }
        else
        {
            bRect.setRect( thermoRect.x(), thermoRect.y(),
                thermoRect.width(), tval - thermoRect.y() );
            if ( alarm )
            {
                aRect.setRect( thermoRect.x(), tval,
                    thermoRect.width(),
                    taval - tval + 1 );
                fRect.setRect( thermoRect.x(), taval + 1,
                    thermoRect.width(),
                    thermoRect.y() + thermoRect.height() - ( taval + 1 ) );
            }
            else
            {
                fRect.setRect( thermoRect.x(), tval,
                    thermoRect.width(),
                    thermoRect.y() + thermoRect.height() - tval );
            }
        }
    }

    // paint thermometer
    painter->fillRect( bRect, palette().brush( QPalette::Base ) );

    if ( alarm )
        painter->fillRect( aRect, d_data->alarmBrush );

    painter->fillRect( fRect, d_data->fillBrush );
}

/*!
   Set the border width of the pipe.
   \param width Border width
   \sa borderWidth()
*/
void QwtThermo::setBorderWidth( int width )
{
    if ( width <= 0 )
        width = 0;

    const QRect tRect = pipeRect();
    int dim = qMin( tRect.width(), tRect.height() );

    if ( width < ( dim + d_data->borderWidth ) / 2  - 1 )
    {
        d_data->borderWidth = width;
        layoutThermo( true );
    }
}

/*!
   Return the border width of the thermometer pipe.
   \sa setBorderWidth()
*/
int QwtThermo::borderWidth() const
{
    return d_data->borderWidth;
}

/*!
  \brief Set the range
  \param vmin value corresponding lower or left end of the thermometer
  \param vmax value corresponding to the upper or right end of the thermometer
  \param logarithmic logarithmic mapping, true or false
*/
void QwtThermo::setRange( double vmin, double vmax, bool logarithmic )
{
    d_data->minValue = vmin;
    d_data->maxValue = vmax;

    if ( logarithmic )
        setScaleEngine( new QwtLog10ScaleEngine );
    else
        setScaleEngine( new QwtLinearScaleEngine );

    /*
      There are two different maps, one for the scale, the other
      for the values. This is confusing and will be changed
      in the future. TODO ...
     */

    d_data->map.setTransformation( scaleEngine()->transformation() );
    d_data->map.setScaleInterval( d_data->minValue, d_data->maxValue );

    if ( autoScale() )
        rescale( d_data->minValue, d_data->maxValue );

    layoutThermo( true );
}

/*!
  \brief Change the brush of the liquid.
  \param brush New brush. The default brush is solid black.
  \sa fillBrush()
*/
void QwtThermo::setFillBrush( const QBrush& brush )
{
    d_data->fillBrush = brush;
    update();
}

/*!
  Return the liquid brush.
  \sa setFillBrush()
*/
const QBrush& QwtThermo::fillBrush() const
{
    return d_data->fillBrush;
}

/*!
  \brief Specify the liquid brush above the alarm threshold
  \param brush New brush. The default is solid white.
  \sa alarmBrush()
*/
void QwtThermo::setAlarmBrush( const QBrush& brush )
{
    d_data->alarmBrush = brush;
    update();
}

/*!
  Return the liquid brush above the alarm threshold.
  \sa setAlarmBrush()
*/
const QBrush& QwtThermo::alarmBrush() const
{
    return d_data->alarmBrush;
}

/*!
  Specify the alarm threshold.

  \param level Alarm threshold
  \sa alarmLevel()
*/
void QwtThermo::setAlarmLevel( double level )
{
    d_data->alarmLevel = level;
    d_data->alarmEnabled = 1;
    update();
}

/*!
  Return the alarm threshold.
  \sa setAlarmLevel()
*/
double QwtThermo::alarmLevel() const
{
    return d_data->alarmLevel;
}

/*!
  Change the width of the pipe.

  \param width Width of the pipe
  \sa pipeWidth()
*/
void QwtThermo::setPipeWidth( int width )
{
    if ( width > 0 )
    {
        d_data->pipeWidth = width;
        layoutThermo( true );
    }
}

/*!
  Return the width of the pipe.
  \sa setPipeWidth()
*/
int QwtThermo::pipeWidth() const
{
    return d_data->pipeWidth;
}

/*!
  \brief Enable or disable the alarm threshold
  \param tf true (disabled) or false (enabled)
*/
void QwtThermo::setAlarmEnabled( bool tf )
{
    d_data->alarmEnabled = tf;
    update();
}

//! Return if the alarm threshold is enabled or disabled.
bool QwtThermo::alarmEnabled() const
{
    return d_data->alarmEnabled;
}

/*!
  \return the minimum size hint
  \sa minimumSizeHint()
*/
QSize QwtThermo::sizeHint() const
{
    return minimumSizeHint();
}

/*!
  \brief Return a minimum size hint
  \warning The return value depends on the font and the scale.
  \sa sizeHint()
*/
QSize QwtThermo::minimumSizeHint() const
{
    int w = 0, h = 0;

    if ( d_data->scalePos != NoScale )
    {
        const int sdExtent = qCeil( scaleDraw()->extent( font() ) );
        const int sdLength = scaleDraw()->minLength( font() );

        w = sdLength;
        h = d_data->pipeWidth + sdExtent +
            d_data->borderWidth + d_data->scaleDist;

    }
    else // no scale
    {
        w = 200;
        h = d_data->pipeWidth;
    }

    if ( d_data->orientation == Qt::Vertical )
        qSwap( w, h );

    w += 2 * d_data->borderWidth;
    h += 2 * d_data->borderWidth;

    return QSize( w, h );
}

int QwtThermo::transform( double value ) const
{
    const double min = qMin( d_data->map.s1(), d_data->map.s2() );
    const double max = qMax( d_data->map.s1(), d_data->map.s2() );

    if ( value > max )
        value = max;
    if ( value < min )
        value = min;

    return qRound( d_data->map.transform( value ) );
}
