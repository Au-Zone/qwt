/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_thermo.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_map.h"
#include <qpainter.h>
#include <qevent.h>
#include <qdrawutil.h>
#include <qstyle.h>
#include <qstyleoption.h>

static inline bool qwtIsLogarithmic( const QwtThermo *thermo )
{
    const QwtScaleTransformation::Type scaleType =
        thermo->scaleEngine()->transformation()->type();

    return ( scaleType == QwtScaleTransformation::Log10 );
}

class QwtThermo::PrivateData
{
public:
    PrivateData():
        fillBrush( Qt::blue ),
        alarmBrush( Qt::red ),
        orientation( Qt::Vertical ),
        scalePos( QwtThermo::LeftScale ),
        borderWidth( 2 ),
        scaleDist( 3 ),
        pipeWidth( 10 ),
        minValue( 0.0 ),
        maxValue( 0.0 ),
        value( 0.0 ),
        alarmLevel( 0.0 ),
        alarmEnabled( false ),
        autoFillPipe( true )
    {
        rangeFlags = QwtInterval::IncludeBorders;
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
    int rangeFlags;
    double value;
    double alarmLevel;
    bool alarmEnabled;
    bool autoFillPipe;
};

/*!
  Constructor
  \param parent Parent widget
*/
QwtThermo::QwtThermo( QWidget *parent ):
    QWidget( parent )
{
    d_data = new PrivateData;
    setRange( 0.0, 1.0, false );

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

void QwtThermo::setRangeFlags( int flags )
{
    if ( d_data->rangeFlags != flags )
    {
        d_data->rangeFlags = flags;
        update();
    }
}

int QwtThermo::rangeFlags() const
{
    return d_data->rangeFlags;
}

/*!
  Set the maximum value.

  \param maxValue Maximum value
  \sa maxValue(), setMinValue(), setRange()
*/
void QwtThermo::setMaxValue( double maxValue )
{
    setRange( d_data->minValue, maxValue, qwtIsLogarithmic( this ) );
}

//! Return the maximum value.
double QwtThermo::maxValue() const
{
    return d_data->maxValue;
}

/*!
  Set the minimum value.

  \param minValue Minimum value
  \sa minValue(), setMaxValue(), setRange()
*/
void QwtThermo::setMinValue( double minValue )
{
    setRange( minValue, d_data->maxValue, qwtIsLogarithmic( this ) );
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

    const QBrush brush = palette().brush( QPalette::Base );
    qDrawShadePanel( &painter, 
        tRect.adjusted( -bw, -bw, bw, bw ),
        palette(), true, bw, 
        d_data->autoFillPipe ? &brush : NULL );

    drawLiquid( &painter, tRect );
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
  the QwtThermo::contentsRect() and the fonts.

  \param update_geometry notify the layout system and call update
         to redraw the scale
*/
void QwtThermo::layoutThermo( bool update_geometry )
{
    const QRect tRect = pipeRect();
    const int bw = d_data->borderWidth + d_data->scaleDist;
    const bool inverted = ( maxValue() < minValue() );

    int from, to;

    if ( d_data->orientation == Qt::Horizontal )
    {
        from = tRect.left();
        to = tRect.right();

        if ( d_data->rangeFlags & QwtInterval::ExcludeMinimum )
        {
            if ( inverted )
                to++;
            else
                from--;
        }
        if ( d_data->rangeFlags & QwtInterval::ExcludeMaximum )
        {
            if ( inverted )
                from--;
            else
                to++;
        }

        switch ( d_data->scalePos )
        {
            case TopScale:
            {
                scaleDraw()->setAlignment( QwtScaleDraw::TopScale );
                scaleDraw()->move( from, tRect.top() - bw );
                scaleDraw()->setLength( to - from );
                break;
            }

            case BottomScale:
            case NoScale: 
            default:
            {
                scaleDraw()->setAlignment( QwtScaleDraw::BottomScale );
                scaleDraw()->move( from, tRect.bottom() + bw );
                scaleDraw()->setLength( to - from );
                break;
            }
        }

        d_data->map.setPaintInterval( from, to );
    }
    else // Qt::Vertical
    {
        from = tRect.top();
        to = tRect.bottom();

        if ( d_data->rangeFlags & QwtInterval::ExcludeMinimum )
        {
            if ( inverted )
                from--;
            else
                to++;
        }
        if ( d_data->rangeFlags & QwtInterval::ExcludeMaximum )
        {
            if ( inverted )
                to++;
            else
                from--;
        }

        switch ( d_data->scalePos )
        {
            case RightScale:
            {
                scaleDraw()->setAlignment( QwtScaleDraw::RightScale );
                scaleDraw()->move( tRect.right() + bw, from );
                scaleDraw()->setLength( to - from );
                break;
            }

            case LeftScale:
            case NoScale: 
            default:
            {
                scaleDraw()->setAlignment( QwtScaleDraw::LeftScale );
                scaleDraw()->move( tRect.left() - bw, from );
                scaleDraw()->setLength( to - from );
                break;
            }
        }
        d_data->map.setPaintInterval( to, from );
    }



    if ( update_geometry )
    {
        updateGeometry();
        update();
    }
}

QRect QwtThermo::pipeRect() const
{
    const QRect cr = contentsRect();

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
                    cr.x() + mbd + bw,
                    cr.y() + cr.height() - d_data->pipeWidth - 2 * bw,
                    cr.width() - 2 * ( bw + mbd ),
                    d_data->pipeWidth 
                );
                break;
            }

            case BottomScale:
            case NoScale: 
            default:   
            {
                tRect.setRect(
                    cr.x() + mbd + bw,
                    cr.y() + d_data->borderWidth,
                    cr.width() - 2 * ( bw + mbd ),
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
                    cr.x() + bw,
                    cr.y() + mbd + bw,
                    d_data->pipeWidth,
                    cr.height() - 2 * ( bw + mbd ) 
                );
                break;
            }
            case LeftScale:
            case NoScale: 
            default:   
            {
                tRect.setRect(
                    cr.x() + cr.width() - 2 * bw - d_data->pipeWidth,
                    cr.y() + mbd + bw,
                    d_data->pipeWidth,
                    cr.height() - 2 * ( bw + mbd ) );
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
void QwtThermo::drawLiquid( QPainter *painter, const QRect &pipeRect )
{
    painter->save();
    painter->setClipRect( pipeRect, Qt::IntersectClip );

    const bool inverted = ( maxValue() < minValue() );
    const int tval = qRound( d_data->map.transform( d_data->value ) );

    QRect fillRect = pipeRect;
    if ( d_data->orientation == Qt::Horizontal )
    {
        if ( inverted )
            fillRect.setLeft( tval );
        else
            fillRect.setRight( tval );
    }
    else // Qt::Vertical
    {
        if ( inverted )
            fillRect.setBottom( tval );
        else
            fillRect.setTop( tval );
    }

    if ( d_data->alarmEnabled &&
        d_data->value >= d_data->alarmLevel )
    {
        QRect alarmRect = fillRect;

        const int taval = qRound( d_data->map.transform( d_data->alarmLevel ) );
        if ( d_data->orientation == Qt::Horizontal )
        {
            if ( inverted )
                alarmRect.setRight( taval );
            else
                alarmRect.setLeft( taval );
        }
        else
        {
            if ( inverted )
                alarmRect.setTop( taval );
            else
                alarmRect.setBottom( taval );
        }

        fillRect = QRegion( fillRect ).subtracted( alarmRect ).boundingRect();

        painter->fillRect( alarmRect, d_data->alarmBrush );
    }

    painter->fillRect( fillRect, d_data->fillBrush );

    painter->restore();
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

    if ( width != d_data->borderWidth  )
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
  \param minValue value corresponding lower or left end of the thermometer
  \param maxValue value corresponding to the upper or right end of the thermometer
  \param logarithmic logarithmic mapping, true or false
*/
void QwtThermo::setRange( 
    double minValue, double maxValue, bool logarithmic )
{
    if ( minValue == d_data->minValue && maxValue == d_data->maxValue
        && logarithmic == qwtIsLogarithmic( this ) )
    {
        return;
    }

    if ( logarithmic != qwtIsLogarithmic( this ) )
    {
        if ( logarithmic )
            setScaleEngine( new QwtLog10ScaleEngine );
        else
            setScaleEngine( new QwtLinearScaleEngine );
    }

    d_data->minValue = minValue;
    d_data->maxValue = maxValue;

    /*
      There are two different maps, one for the scale, the other
      for the values. This is confusing and will be changed
      in the future. TODO ...
     */

    d_data->map.setTransformation( scaleEngine()->transformation() );
    d_data->map.setScaleInterval( minValue, maxValue );

    if ( autoScale() )
        rescale( minValue, maxValue );

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

    int left, right, top, bottom;
    getContentsMargins( &left, &top, &right, &bottom );
    w += left + right;
    h += top + bottom;

    return QSize( w, h );
}
