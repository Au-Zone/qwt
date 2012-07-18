/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_slider.h"
#include "qwt_painter.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_map.h"
#include <qevent.h>
#include <qdrawutil.h>
#include <qpainter.h>
#include <qalgorithms.h>
#include <qmath.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qapplication.h>

static QSize qwtHandleSize( const QSize &size, 
	Qt::Orientation orientation, QwtSlider::BackgroundStyles bgStyles )
{
	QSize handleSize = size;

	if ( handleSize.isEmpty() )
	{
    	const int handleThickness = 16;
    	handleSize.setWidth( 2 * handleThickness );
    	handleSize.setHeight( handleThickness );

    	if ( !( bgStyles & QwtSlider::Trough ) )
        	handleSize.transpose();

    	if ( orientation == Qt::Vertical )
        	handleSize.transpose();
	}

	return handleSize;
}

static QwtScaleDraw::Alignment qwtScaleDrawAlignment( 
	Qt::Orientation orientation, QwtSlider::ScalePosition scalePos )
{
    QwtScaleDraw::Alignment align;

    if ( orientation == Qt::Vertical )
    {
        // NoScale lays out like Left
        if ( scalePos == QwtSlider::LeadingScale )
            align = QwtScaleDraw::RightScale;
        else
            align = QwtScaleDraw::LeftScale;
    }
    else
    {
        // NoScale lays out like Bottom
        if ( scalePos == QwtSlider::TrailingScale )
            align = QwtScaleDraw::TopScale;
        else
            align = QwtScaleDraw::BottomScale;
    }

	return align;
}

class QwtSlider::PrivateData
{
public:
    PrivateData():
        repeatTimerId( 0 ),
        updateInterval( 150 ),
        valueIncrement( 0.0 ),
		pendingValueChange( false )
    {
    }

    int repeatTimerId;
    bool timerTick;
    int updateInterval;
    double valueIncrement;
	bool pendingValueChange;

    QRect sliderRect;

    QSize handleSize;
    int borderWidth;
    int spacing;

    Qt::Orientation orientation;
    QwtSlider::ScalePosition scalePosition;
    QwtSlider::BackgroundStyles bgStyle;

    /*
      Scale and values might have different maps. This is
      confusing and I can't see strong arguments for such
      a feature. TODO ...
     */
    QwtScaleMap map; // linear map
    mutable QSize sizeHintCache;
};

/*!
  \brief Constructor
  \param parent parent widget
  \param orientation Orientation of the slider. Can be Qt::Horizontal
         or Qt::Vertical. Defaults to Qt::Horizontal.
  \param scalePosition Position of the scale.
         Defaults to QwtSlider::NoScale.
  \param bgStyle Background style. QwtSlider::Trough draws the
         slider button in a trough, QwtSlider::Slot draws
         a slot underneath the button. An or-combination of both
         may also be used. The default is QwtSlider::Trough.
*/
QwtSlider::QwtSlider( QWidget *parent,
        Qt::Orientation orientation, ScalePosition scalePosition, 
        BackgroundStyles bgStyle ):
    QwtAbstractSlider( parent )
{
    if ( orientation == Qt::Vertical )
        setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
    else
        setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

    setAttribute( Qt::WA_WState_OwnSizePolicy, false );

    d_data = new QwtSlider::PrivateData;

    d_data->borderWidth = 2;
    d_data->spacing = 4;
    d_data->orientation = orientation;
    d_data->scalePosition = scalePosition;
    d_data->bgStyle = bgStyle;

    d_data->sliderRect.setRect( 0, 0, 8, 8 );

    scaleDraw()->setAlignment( 
		qwtScaleDrawAlignment( orientation, scalePosition ) );
    scaleDraw()->setLength( 100 );

    setRange( 0.0, 100.0 );
    setSingleStep( 1.0 );
    setValue( 0.0 );

	connect( this, SIGNAL( valueChanged( double ) ), SLOT( emitScaleValue() ) );
}

QwtSlider::~QwtSlider()
{
    delete d_data;
}

/*!
  \brief Set the orientation.
  \param orientation Allowed values are Qt::Horizontal and Qt::Vertical.

  \sa orientation(), scalePosition()
*/
void QwtSlider::setOrientation( Qt::Orientation orientation )
{
    if ( orientation == d_data->orientation )
        return;

    d_data->orientation = orientation;

    scaleDraw()->setAlignment( 
        qwtScaleDrawAlignment( orientation, d_data->scalePosition ) );

    if ( !testAttribute( Qt::WA_WState_OwnSizePolicy ) )
    {
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy( sp );

        setAttribute( Qt::WA_WState_OwnSizePolicy, false );
    }

    layoutSlider( true );
}

/*!
  \return Orientation
  \sa setOrientation()
*/
Qt::Orientation QwtSlider::orientation() const
{
    return d_data->orientation;
}

/*!
  \brief Change the scale position (and slider orientation).

  \param scalePos Position of the scale.

  A valid combination of scale position and orientation is enforced:
  - if the new scale position is Left or Right, the scale orientation will
    become Qt::Vertical;
  - if the new scale position is Bottom or Top the scale orientation will
    become Qt::Horizontal;
  - if the new scale position is QwtSlider::NoScale, the scale
    orientation will not change.
*/
void QwtSlider::setScalePosition( ScalePosition scalePosition )
{
    if ( d_data->scalePosition == scalePosition )
        return;

    d_data->scalePosition = scalePosition;
    scaleDraw()->setAlignment( 
		qwtScaleDrawAlignment( d_data->orientation, scalePosition ) );

    layoutSlider( true );
}

//! Return the scale position.
QwtSlider::ScalePosition QwtSlider::scalePosition() const
{
    return d_data->scalePosition;
}

/*!
  \brief Change the slider's border width
  \param width Border width
*/
void QwtSlider::setBorderWidth( int width )
{
    if ( width < 0 )
        width = 0;

    if ( width != d_data->borderWidth )
    {
        d_data->borderWidth = width;
        layoutSlider( true );
    }
}

/*!
  \return the border width.
  \sa setBorderWidth()
*/
int QwtSlider::borderWidth() const
{
    return d_data->borderWidth;
}

/*!
  \brief Change the spacing between pipe and scale

  A spacing of 0 means, that the backbone of the scale is below
  the trough.

  The default setting is 4 pixels.

  \param spacing Number of pixels
  \sa spacing();
*/
void QwtSlider::setSpacing( int spacing )
{
    if ( spacing <= 0 )
        spacing = 0;

    if ( spacing != d_data->spacing  )
    {
        d_data->spacing = spacing;
        layoutSlider( true );
    }
}

/*!
  \return Number of pixels between slider and scale
  \sa setSpacing()
*/
int QwtSlider::spacing() const
{
    return d_data->spacing;
}

/*!
  \brief Set the slider's handle size

  When the size is empty the slider handle will be painted with a
  default size depending on its orientation() and backgroundStyle().

  \param size New size

  \sa handleSize()
*/
void QwtSlider::setHandleSize( const QSize &size )
{
    if ( size != d_data->handleSize )
    {
        d_data->handleSize = size;
        layoutSlider( true );
    }
}

/*!
  \return Size of the handle.
  \sa setHandleSize()
*/
QSize QwtSlider::handleSize() const
{
    return d_data->handleSize;
}

/*!
  \brief Set a scale draw

  For changing the labels of the scales, it
  is necessary to derive from QwtScaleDraw and
  overload QwtScaleDraw::label().

  \param scaleDraw ScaleDraw object, that has to be created with
                   new and will be deleted in ~QwtSlider or the next
                   call of setScaleDraw().

  \sa scaleDraw()
*/
void QwtSlider::setScaleDraw( QwtScaleDraw *scaleDraw )
{
    const QwtScaleDraw *previousScaleDraw = this->scaleDraw();
    if ( scaleDraw == NULL || scaleDraw == previousScaleDraw )
        return;

    if ( previousScaleDraw )
        scaleDraw->setAlignment( previousScaleDraw->alignment() );

    setAbstractScaleDraw( scaleDraw );
    layoutSlider( true );
}

/*!
  \return the scale draw of the slider
  \sa setScaleDraw()
*/
const QwtScaleDraw *QwtSlider::scaleDraw() const
{
    return static_cast<const QwtScaleDraw *>( abstractScaleDraw() );
}

/*!
  \return the scale draw of the slider
  \sa setScaleDraw()
*/
QwtScaleDraw *QwtSlider::scaleDraw()
{
    return static_cast<QwtScaleDraw *>( abstractScaleDraw() );
}

//! Notify changed scale
void QwtSlider::scaleChange()
{
    layoutSlider( true );
}

/*!
  \brief Specify the update interval for automatic scrolling
  \param interval Update interval in milliseconds
*/
void QwtSlider::setUpdateInterval( int interval )
{
    d_data->updateInterval = qMax( interval, 50 );
}

int QwtSlider::updateInterval() const
{
    return d_data->updateInterval;
}

/*!
   Draw the slider into the specified rectangle.

   \param painter Painter
   \param sliderRect Bounding rectangle of the slider
*/
void QwtSlider::drawSlider( 
    QPainter *painter, const QRect &sliderRect ) const
{
    QRect innerRect( sliderRect );

    if ( d_data->bgStyle & QwtSlider::Trough )
    {
        const int bw = d_data->borderWidth;

        qDrawShadePanel( painter, sliderRect, palette(), true, bw, NULL );

        innerRect = sliderRect.adjusted( bw, bw, -bw, -bw );
        painter->fillRect( innerRect, palette().brush( QPalette::Mid ) );
    }

    const QSize handleSize = qwtHandleSize( d_data->handleSize,
		d_data->orientation, d_data->bgStyle );

    if ( d_data->bgStyle & QwtSlider::Groove )
    {
        int ws = 4;
        int ds = handleSize.width() / 2 - 4;
        if ( ds < 1 )
            ds = 1;

        QRect rSlot;
        if ( orientation() == Qt::Horizontal )
        {
            if ( innerRect.height() & 1 )
                ws++;

            rSlot = QRect( innerRect.x() + ds,
                    innerRect.y() + ( innerRect.height() - ws ) / 2,
                    innerRect.width() - 2 * ds, ws );
        }
        else
        {
            if ( innerRect.width() & 1 )
                ws++;

            rSlot = QRect( innerRect.x() + ( innerRect.width() - ws ) / 2,
                           innerRect.y() + ds,
                           ws, innerRect.height() - 2 * ds );
        }

        QBrush brush = palette().brush( QPalette::Dark );
        qDrawShadePanel( painter, rSlot, palette(), true, 1 , &brush );
    }

    if ( isValid() )
        drawHandle( painter, innerRect, transform( value() ) );
}

/*!
  Draw the thumb at a position

  \param painter Painter
  \param sliderRect Bounding rectangle of the slider
  \param pos Position of the slider thumb
*/
void QwtSlider::drawHandle( QPainter *painter, 
    const QRect &sliderRect, int pos ) const
{
    const QSize handleSize = qwtHandleSize( d_data->handleSize,
        d_data->orientation, d_data->bgStyle );

    const int bw = d_data->borderWidth;

    pos++; // shade line points one pixel below
    if ( orientation() == Qt::Horizontal )
    {
        QRect handleRect(
            pos - handleSize.width() / 2,
            sliderRect.y(), 
            handleSize.width(), 
            sliderRect.height()
        );

        qDrawShadePanel( painter, 
            handleRect, palette(), false, bw,
            &palette().brush( QPalette::Button ) );

        qDrawShadeLine( painter, pos, sliderRect.top() + bw,
            pos, sliderRect.bottom() - bw,
            palette(), true, 1 );
    }
    else // Vertical
    {
        QRect handleRect(
            sliderRect.left(), 
            pos - handleSize.height() / 2,
            sliderRect.width(), 
            handleSize.height()
        );

        qDrawShadePanel( painter, 
            handleRect, palette(), false, bw,
            &palette().brush( QPalette::Button ) );

        qDrawShadeLine( painter, sliderRect.left() + bw, pos,
            sliderRect.right() - bw, pos,
            palette(), true, 1 );
    }
}

/*!
   Map and round a value into widget coordinates
   \param value Value
*/
int QwtSlider::transform( double value ) const
{
    return qRound( d_data->map.transform( value ) );
}

/*!
   Determine the value corresponding to a specified mouse location.
   \param pos Mouse position
*/
double QwtSlider::valueAt( const QPoint &pos )
{
    return d_data->map.invTransform(
        orientation() == Qt::Horizontal ? pos.x() : pos.y() );
}

bool QwtSlider::isScrollPosition( const QPoint &pos ) const
{
    return handleRect().contains( pos );
}

void QwtSlider::mousePressEvent( QMouseEvent *event )
{
    if ( isReadOnly() )
    {
        event->ignore();
        return;
    }

	const QPoint pos = event->pos();

    if ( isValid() && d_data->sliderRect.contains( pos ) )
    {
        if ( !handleRect().contains( pos ) )
        {
        	const int markerPos = transform( value() );

			d_data->valueIncrement = qAbs( singleStep() );
			if ( d_data->orientation == Qt::Horizontal )
			{
				if ( pos.x() < markerPos )
					d_data->valueIncrement = -d_data->valueIncrement;
			}
			else
			{
				if ( pos.y() < markerPos )
					d_data->valueIncrement = -d_data->valueIncrement;
			}

			if ( d_data->map.isInverting() )
				d_data->valueIncrement = -d_data->valueIncrement;

    		if ( pageStepCount() > 0 )
			{
#if 0
				if ( ( event->modifiers() & Qt::ControlModifier) ||
					( event->modifiers() & Qt::ShiftModifier ) )
#endif
				{
					d_data->valueIncrement *= pageStepCount();
				}
			}

            d_data->timerTick = false;
            d_data->repeatTimerId = startTimer( qMax( 250, 2 * updateInterval() ) );

            return;
        }
    }

    QwtAbstractSlider::mousePressEvent( event );
}

void QwtSlider::mouseReleaseEvent( QMouseEvent *event )
{
    if ( d_data->repeatTimerId > 0 )
    {
        killTimer( d_data->repeatTimerId );
        d_data->repeatTimerId = 0;
        d_data->timerTick = false;
        d_data->valueIncrement = 0.0;
    }

    if ( d_data->pendingValueChange )
	{
		d_data->pendingValueChange = false;
        Q_EMIT valueChanged( value() );
	}

    QwtAbstractSlider::mouseReleaseEvent( event );
}

void QwtSlider::keyPressEvent( QKeyEvent *event )
{
    if ( d_data->orientation == Qt::Vertical )
    {
        if ( event->key() == Qt::Key_Left || event->key() == Qt::Key_Right )
            return;
    }
    else
    {
        if ( event->key() == Qt::Key_Up || event->key() == Qt::Key_Down )
            return;
    }

    QwtAbstractSlider::keyPressEvent( event );
}

void QwtSlider::timerEvent( QTimerEvent *event )
{
    if ( event->timerId() != d_data->repeatTimerId )
    {
		QwtAbstractSlider::timerEvent( event );
		return;
	}

    if ( !isValid() )
	{
		killTimer( d_data->repeatTimerId );
		d_data->repeatTimerId = 0;
		return;
	}

    const double v = value();
    incrementValue( d_data->valueIncrement );

	if ( v != value() )
	{
       	if ( isTracking() )
           	Q_EMIT valueChanged( value() );
		else
			d_data->pendingValueChange = true;

       	Q_EMIT sliderMoved( value() );
    }

    if ( !d_data->timerTick )
	{
		// restart the timer with a shorter interval
		killTimer( d_data->repeatTimerId );
		d_data->repeatTimerId = startTimer( updateInterval() );
		
		d_data->timerTick = true;
	}   
}

void QwtSlider::wheelEvent( QWheelEvent *event )
{
    if ( d_data->sliderRect.contains( event->pos() ) )
        QwtAbstractSlider::wheelEvent( event );
}

/*!
   Qt paint event
   \param event Paint event
*/
void QwtSlider::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    if ( d_data->scalePosition != QwtSlider::NoScale )
    {
        if ( !d_data->sliderRect.contains( event->rect() ) )
            scaleDraw()->draw( &painter, palette() );
    }

    drawSlider( &painter, d_data->sliderRect );

    if ( hasFocus() )
        QwtPainter::drawFocusRect( &painter, this, d_data->sliderRect );
}

//! Qt resize event
void QwtSlider::resizeEvent( QResizeEvent * )
{
    layoutSlider( false );
}

//! Qt change event handler
void QwtSlider::changeEvent( QEvent *event )
{
	if ( event->type() == QEvent::StyleChange || 
		event->type() == QEvent::FontChange )
	{
		layoutSlider( true );
	}

	QwtAbstractSlider::changeEvent( event );
}

/*!
  Recalculate the slider's geometry and layout based on
  the current rect and fonts.
  \param update_geometry  notify the layout system and call update
         to redraw the scale
*/
void QwtSlider::layoutSlider( bool update_geometry )
{
    const QSize handleSize = qwtHandleSize( d_data->handleSize,
        d_data->orientation, d_data->bgStyle );

    int handleThickness;
    if ( d_data->orientation == Qt::Horizontal )
        handleThickness = handleSize.width();
    else
        handleThickness = handleSize.height();

    int sld1 = handleThickness / 2 - 1;
    int sld2 = handleThickness / 2 + handleThickness % 2;

    if ( d_data->bgStyle & QwtSlider::Trough )
    {
        sld1 += d_data->borderWidth;
        sld2 += d_data->borderWidth;
    }

    int scd = 0;
    if ( d_data->scalePosition != QwtSlider::NoScale )
    {
        int d1, d2;
        scaleDraw()->getBorderDistHint( font(), d1, d2 );
        scd = qMax( d1, d2 );
    }

    int slo = scd - sld1;
    if ( slo < 0 )
        slo = 0;

    int x, y, length;
    QRect sliderRect;

    length = x = y = 0;

    const QRect cr = contentsRect();
    if ( d_data->orientation == Qt::Horizontal )
    {
        int sh = handleSize.height();
        if ( d_data->bgStyle & QwtSlider::Trough )
            sh += 2 * d_data->borderWidth;

        sliderRect.setLeft( cr.left() + slo );
        sliderRect.setRight( cr.right() - slo );
        sliderRect.setTop( cr.top() );
        sliderRect.setBottom( cr.top() + sh - 1);

        if ( d_data->scalePosition == QwtSlider::LeadingScale )
        {
            y = sliderRect.bottom() + d_data->spacing;
        }
        else if ( d_data->scalePosition == QwtSlider::TrailingScale )
        {
            sliderRect.setTop( cr.bottom() - sh + 1 );
            sliderRect.setBottom( cr.bottom() );

            y = sliderRect.top() - d_data->spacing;
        }

        x = sliderRect.left() + sld1;
        length = sliderRect.width() - ( sld1 + sld2 );
    }
    else // Qt::Vertical
    {
        int sw = handleSize.width();
        if ( d_data->bgStyle & QwtSlider::Trough )
            sw += 2 * d_data->borderWidth;

        sliderRect.setLeft( cr.right() - sw + 1 );
        sliderRect.setRight( cr.right() );
        sliderRect.setTop( cr.top() + slo );
        sliderRect.setBottom( cr.bottom() - slo );

        if ( d_data->scalePosition == QwtSlider::TrailingScale )
        {
            x = sliderRect.left() - d_data->spacing;
        }
        else if ( d_data->scalePosition == QwtSlider::LeadingScale )
        {
            sliderRect.setLeft( cr.left() );
            sliderRect.setRight( cr.left() + sw - 1);

            x = sliderRect.right() + d_data->spacing;
        }

        y = sliderRect.top() + sld1;
        length = sliderRect.height() - ( sld1 + sld2 );
    }

    d_data->sliderRect = sliderRect;

    scaleDraw()->move( x, y );
    scaleDraw()->setLength( length );

    d_data->map.setPaintInterval( scaleDraw()->scaleMap().p1(),
        scaleDraw()->scaleMap().p2() );

    if ( update_geometry )
    {
        d_data->sizeHintCache = QSize(); // invalidate
        updateGeometry();
        update();
    }
}

void QwtSlider::emitScaleValue()
{
	Q_EMIT scaleValueChanged( scaleValue() );
}

double QwtSlider::scaleValue() const
{
    const double v = d_data->map.transform( value() );
    return scaleDraw()->scaleMap().invTransform( v );
}

void QwtSlider::setScaleValue( double value )
{
    const double v = scaleDraw()->scaleMap().transform( value );
    setValue( d_data->map.invTransform( v ) );
}

//! Notify change of range
void QwtSlider::rangeChange()
{
    d_data->map.setScaleInterval( minimum(), maximum() );

    if ( autoScale() )
        rescale( minimum(), maximum() );

    QwtAbstractSlider::rangeChange();
    layoutSlider( true );
}

/*!
  Set the background style.
*/
void QwtSlider::setBackgroundStyle( BackgroundStyles style )
{
    d_data->bgStyle = style;
    layoutSlider( true );
}

/*!
  \return the background style.
*/
QwtSlider::BackgroundStyles QwtSlider::backgroundStyle() const
{
    return d_data->bgStyle;
}

/*!
  \return QwtSlider::minimumSizeHint()
*/
QSize QwtSlider::sizeHint() const
{
    const QSize hint = minimumSizeHint();
    return hint.expandedTo( QApplication::globalStrut() );
}

/*!
  \brief Return a minimum size hint
  \warning The return value of QwtSlider::minimumSizeHint() depends on
           the font and the scale.
*/
QSize QwtSlider::minimumSizeHint() const
{
    if ( !d_data->sizeHintCache.isEmpty() )
        return d_data->sizeHintCache;

    const int minLength = 84; // from QSlider

    const QSize handleSize = qwtHandleSize( d_data->handleSize,
        d_data->orientation, d_data->bgStyle );

    int handleLength = handleSize.width();
    int handleThickness = handleSize.height();

    if ( d_data->orientation == Qt::Vertical )
        qSwap( handleLength, handleThickness );

    int w = minLength; 
    int h = handleThickness;

    if ( d_data->scalePosition != QwtSlider::NoScale )
    {
        int d1, d2;
        scaleDraw()->getBorderDistHint( font(), d1, d2 );

        const int sdBorderDist = 2 * qMax( d1, d2 );
        const int sdExtent = qCeil( scaleDraw()->extent( font() ) );
        const int sdLength = scaleDraw()->minLength( font() );

        int l = sdLength;
        if ( handleLength > sdBorderDist )
        {
            // We need additional space for the overlapping handle
            l += handleLength - sdBorderDist;
        }

        w = qMax( l, w );
        h += sdExtent + d_data->spacing;
    }

    if ( d_data->bgStyle & QwtSlider::Trough )
        h += 2 * d_data->borderWidth;

    if ( d_data->orientation == Qt::Vertical )
        qSwap( w, h );

    int left, right, top, bottom;
    getContentsMargins( &left, &top, &right, &bottom );

    w += left + right;
    h += top + bottom;

    d_data->sizeHintCache = QSize( w, h );
    return d_data->sizeHintCache;
}

QRect QwtSlider::handleRect() const
{
    const int markerPos = transform( value() );

	QPoint center = d_data->sliderRect.center();
	if ( d_data->orientation == Qt::Horizontal )
		center.setX( markerPos );
	else
		center.setY( markerPos );

    QRect rect;
	rect.setSize( qwtHandleSize( d_data->handleSize,
        d_data->orientation, d_data->bgStyle ) );
	rect.moveCenter( center );

	return rect;
}
