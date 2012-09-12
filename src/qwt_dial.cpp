/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_dial.h"
#include "qwt_dial_needle.h"
#include "qwt_math.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_map.h"
#include "qwt_round_scale_draw.h"
#include "qwt_painter.h"
#include <qpainter.h>
#include <qbitmap.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qalgorithms.h>
#include <qmath.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qapplication.h>

#if QT_VERSION < 0x040601
#define qAtan(x) ::atan(x)
#endif

class QwtDial::PrivateData
{
public:
    PrivateData():
        frameShadow( Sunken ),
        lineWidth( 0 ),
        mode( RotateNeedle ),
        origin( 90.0 ),
        minScaleArc( 0.0 ),
        maxScaleArc( 0.0 ),
        needle( NULL ),
        mouseOffset( 0.0 ),
        previousDir( -1.0 )
    {
    }

    ~PrivateData()
    {
        delete needle;
    }
    Shadow frameShadow;
    int lineWidth;

    QwtDial::Mode mode;

    double origin;
    double minScaleArc;
    double maxScaleArc;

    double scalePenWidth;
    QwtDialNeedle *needle;

    double mouseOffset;
    double previousDir;
};

/*!
  \brief Constructor
  \param parent Parent widget

  Create a dial widget with no scale and no needle.
  The default origin is 90.0 with no valid value. It accepts
  mouse and keyboard inputs and has no step size. The default mode
  is QwtDial::RotateNeedle.
*/
QwtDial::QwtDial( QWidget* parent ):
    QwtAbstractSlider( parent )
{
    d_data = new PrivateData;

    setFocusPolicy( Qt::TabFocus );

    QPalette p = palette();
    for ( int i = 0; i < QPalette::NColorGroups; i++ )
    {
        const QPalette::ColorGroup colorGroup =
            static_cast<QPalette::ColorGroup>( i );

        // Base: background color of the circle inside the frame.
        // WindowText: background color of the circle inside the scale

        p.setColor( colorGroup, QPalette::WindowText,
            p.color( colorGroup, QPalette::Base ) );
    }
    setPalette( p );

    QwtRoundScaleDraw* scaleDraw = new QwtRoundScaleDraw();
    scaleDraw->setRadius( 0 );

    setScaleDraw( scaleDraw );

    setScaleMaxMajor( 36 );
    setScaleMaxMinor( 10 );

    setScaleArc( 0.0, 360.0 ); // scale as a full circle
    setScale( 0.0, 360.0 ); // degrees as default

    setValue( 0.0 );
}

//!  Destructor
QwtDial::~QwtDial()
{
    delete d_data;
}

/*!
  Sets the frame shadow value from the frame style.
  \param shadow Frame shadow
  \sa setLineWidth(), QFrame::setFrameShadow()
*/
void QwtDial::setFrameShadow( Shadow shadow )
{
    if ( shadow != d_data->frameShadow )
    {
        d_data->frameShadow = shadow;
        if ( lineWidth() > 0 )
            update();
    }
}

/*!
  \return Frame shadow
  /sa setFrameShadow(), lineWidth(), QFrame::frameShadow
*/
QwtDial::Shadow QwtDial::frameShadow() const
{
    return d_data->frameShadow;
}

/*!
  Sets the line width

  \param lineWidth Line width
  \sa setFrameShadow()
*/
void QwtDial::setLineWidth( int lineWidth )
{
    if ( lineWidth < 0 )
        lineWidth = 0;

    if ( d_data->lineWidth != lineWidth )
    {
        d_data->lineWidth = lineWidth;
        update();
    }
}

/*!
  \return Line width of the frame
  \sa setLineWidth(), frameShadow(), lineWidth()
*/
int QwtDial::lineWidth() const
{
    return d_data->lineWidth;
}

/*!
  \return bounding rect of the circle inside the frame
  \sa setLineWidth(), scaleInnerRect(), boundingRect()
*/
QRectF QwtDial::innerRect() const
{
    const double lw = lineWidth();
    return boundingRect().adjusted( lw, lw, -lw, -lw );
}

/*!
  \return bounding rect of the dial including the frame
  \sa setLineWidth(), scaleInnerRect(), innerRect()
*/
QRectF QwtDial::boundingRect() const
{
    const QRectF cr = contentsRect();

    const double dim = qMin( cr.width(), cr.height() );

    QRectF inner( 0, 0, dim, dim );
    inner.moveCenter( cr.center() );

    return inner;
}

/*!
  \return rect inside the scale
  \sa setLineWidth(), boundingRect(), innerRect()
*/
QRectF QwtDial::scaleInnerRect() const
{
    QRectF rect = innerRect();

    const QwtAbstractScaleDraw *sd = scaleDraw();
    if ( sd )
    {
        double scaleDist = qCeil( sd->extent( font() ) );
        scaleDist++; // margin

        rect.adjust( scaleDist, scaleDist, -scaleDist, -scaleDist );
    }

    return rect;
}

/*!
  \brief Change the mode of the meter.
  \param mode New mode

  The value of the meter is indicated by the difference
  between north of the scale and the direction of the needle.
  In case of QwtDial::RotateNeedle north is pointing
  to the origin() and the needle is rotating, in case of
  QwtDial::RotateScale, the needle points to origin()
  and the scale is rotating.

  The default mode is QwtDial::RotateNeedle.

  \sa mode(), setValue(), setOrigin()
*/
void QwtDial::setMode( Mode mode )
{
    if ( mode != d_data->mode )
    {
        d_data->mode = mode;
        update();
    }
}

/*!
  \return mode of the dial.

  The value of the dial is indicated by the difference
  between the origin and the direction of the needle.
  In case of QwtDial::RotateNeedle the scale arc is fixed
  to the origin() and the needle is rotating, in case of
  QwtDial::RotateScale, the needle points to origin()
  and the scale is rotating.

  The default mode is QwtDial::RotateNeedle.

  \sa setMode(), origin(), setScaleArc(), value()
*/
QwtDial::Mode QwtDial::mode() const
{
    return d_data->mode;
}

/*!
   Paint the dial
   \param event Paint event
*/
void QwtDial::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    painter.setRenderHint( QPainter::Antialiasing, true );

    painter.save();
    drawContents( &painter );
    painter.restore();

    painter.save();
    drawFrame( &painter );
    painter.restore();

    if ( hasFocus() )
        drawFocusIndicator( &painter );
}

/*!
  Draw a dotted round circle, if !isReadOnly()

  \param painter Painter
*/
void QwtDial::drawFocusIndicator( QPainter *painter ) const
{
    if ( !isReadOnly() )
    {
        QRectF focusRect = innerRect();

        const int margin = 2;
        focusRect.adjust( margin, margin, -margin, -margin );

        QColor color = palette().color( QPalette::Base );
        if ( color.isValid() )
        {
            const QColor gray( Qt::gray );

            int h, s, v;
            color.getHsv( &h, &s, &v );
            color = ( v > 128 ) ? gray.dark( 120 ) : gray.light( 120 );
        }
        else
            color = Qt::darkGray;

        painter->save();
        painter->setBrush( Qt::NoBrush );
        painter->setPen( QPen( color, 0, Qt::DotLine ) );
        painter->drawEllipse( focusRect );
        painter->restore();
    }
}

/*!
  Draw the frame around the dial

  \param painter Painter
  \sa lineWidth(), frameShadow()
*/
void QwtDial::drawFrame( QPainter *painter )
{
    if ( lineWidth() <= 0 )
        return;

    const double lw2 = 0.5 * lineWidth();

    QRectF r = boundingRect();
    r.adjust( lw2, lw2, -lw2, -lw2 );

    QPen pen;

    switch ( d_data->frameShadow )
    {
        case QwtDial::Raised:
        case QwtDial::Sunken:
        {
            QColor c1 = palette().color( QPalette::Light );
            QColor c2 = palette().color( QPalette::Dark );

            if ( d_data->frameShadow == QwtDial::Sunken )
                qSwap( c1, c2 );

            QLinearGradient gradient( r.topLeft(), r.bottomRight() );
            gradient.setColorAt( 0.0, c1 );
#if 0
            gradient.setColorAt( 0.3, c1 );
            gradient.setColorAt( 0.7, c2 );
#endif
            gradient.setColorAt( 1.0, c2 );

            pen = QPen( gradient, lineWidth() );
            break;
        }
        default: // Plain
        {
            pen = QPen( palette().brush( QPalette::Dark ), lineWidth() );
        }
    }

    painter->save();

    painter->setPen( pen );
    painter->setBrush( Qt::NoBrush );
    painter->drawEllipse( r );

    painter->restore();
}

/*!
  \brief Draw the contents inside the frame

  QPalette::Window is the background color outside of the frame.
  QPalette::Base is the background color inside the frame.
  QPalette::WindowText is the background color inside the scale.

  \param painter Painter
  \sa boundingRect(), innerRect(),
    scaleInnerRect(), QWidget::setPalette()
*/
void QwtDial::drawContents( QPainter *painter ) const
{
    if ( testAttribute( Qt::WA_NoSystemBackground ) ||
        palette().brush( QPalette::Base ) !=
            palette().brush( QPalette::Window ) )
    {
        const QRectF br = boundingRect();

        painter->save();
        painter->setPen( Qt::NoPen );
        painter->setBrush( palette().brush( QPalette::Base ) );
        painter->drawEllipse( br );
        painter->restore();
    }


    const QRectF insideScaleRect = scaleInnerRect();
    if ( palette().brush( QPalette::WindowText ) !=
            palette().brush( QPalette::Base ) )
    {
        painter->save();
        painter->setPen( Qt::NoPen );
        painter->setBrush( palette().brush( QPalette::WindowText ) );
        painter->drawEllipse( insideScaleRect );
        painter->restore();
    }

    QwtDial *that = const_cast< QwtDial * >( this );
    that->setAngleRange( d_data->origin + d_data->minScaleArc, 
        d_data->maxScaleArc - d_data->minScaleArc );

    if ( mode() == RotateScale )
    {
        double a = transform( value() ) - d_data->origin;
        that->setAngleRange( scaleMap().p1() - a, 
            scaleMap().p2() - scaleMap().p1() );
    }

    const QPointF center = insideScaleRect.center();
    const double radius = 0.5 * insideScaleRect.width();

    painter->save();
    drawScale( painter, center, radius, d_data->origin );
    painter->restore();

    painter->save();
    drawScaleContents( painter, center, radius );
    painter->restore();

    if ( isValid() )
    {
        QPalette::ColorGroup cg;
        if ( isEnabled() )
            cg = hasFocus() ? QPalette::Active : QPalette::Inactive;
        else
            cg = QPalette::Disabled;

        const double direction2 = transform( value() ) + 270.0;

        painter->save();
        drawNeedle( painter, center, radius, direction2, cg );
        painter->restore();
    }
}

/*!
  Draw the needle

  \param painter Painter
  \param center Center of the dial
  \param radius Length for the needle
  \param direction Direction of the needle in degrees, counter clockwise
  \param cg ColorGroup
*/
void QwtDial::drawNeedle( QPainter *painter, const QPointF &center,
    double radius, double direction, QPalette::ColorGroup cg ) const
{
    if ( d_data->needle )
    {
        direction = 360.0 - direction; // counter clockwise
        d_data->needle->draw( painter, center, radius, direction, cg );
    }
}

/*!
  Draw the scale

  \param painter Painter
  \param center Center of the dial
  \param radius Radius of the scale
  \param origin Origin of the scale
*/
void QwtDial::drawScale( QPainter *painter, const QPointF &center,
    double radius, double origin ) const
{
    Q_UNUSED( origin );

    QwtRoundScaleDraw *sd = const_cast<QwtRoundScaleDraw *>( scaleDraw() );
    if ( sd == NULL )
        return;

    sd->setRadius( radius );
    sd->moveCenter( center );

    QPalette pal = palette();

    const QColor textColor = pal.color( QPalette::Text );
    pal.setColor( QPalette::WindowText, textColor ); // ticks, backbone

    painter->setFont( font() );
    painter->setPen( QPen( textColor, sd->penWidth() ) );

    painter->setBrush( Qt::red );
    sd->draw( painter, pal );
}

/*!
  Draw the contents inside the scale

  Paints nothing.

  \param painter Painter
  \param center Center of the contents circle
  \param radius Radius of the contents circle
*/

void QwtDial::drawScaleContents( QPainter *painter,
    const QPointF &center, double radius ) const
{
    Q_UNUSED(painter);
    Q_UNUSED(center);
    Q_UNUSED(radius);
}

/*!
  Set a needle for the dial

  Qwt is missing a set of good looking needles.
  Contributions are very welcome.

  \param needle Needle
  \warning The needle will be deleted, when a different needle is
    set or in ~QwtDial()
*/
void QwtDial::setNeedle( QwtDialNeedle *needle )
{
    if ( needle != d_data->needle )
    {
        if ( d_data->needle )
            delete d_data->needle;

        d_data->needle = needle;
        update();
    }
}

/*!
  \return needle
  \sa setNeedle()
*/
const QwtDialNeedle *QwtDial::needle() const
{
    return d_data->needle;
}

/*!
  \return needle
  \sa setNeedle()
*/
QwtDialNeedle *QwtDial::needle()
{
    return d_data->needle;
}

//! Return the scale draw
QwtRoundScaleDraw *QwtDial::scaleDraw()
{
    return static_cast<QwtRoundScaleDraw *>( abstractScaleDraw() );
}

//! Return the scale draw
const QwtRoundScaleDraw *QwtDial::scaleDraw() const
{
    return static_cast<const QwtRoundScaleDraw *>( abstractScaleDraw() );
}

/*!
  Set an individual scale draw

  \param scaleDraw Scale draw
  \warning The previous scale draw is deleted
*/
void QwtDial::setScaleDraw( QwtRoundScaleDraw *scaleDraw )
{
    setAbstractScaleDraw( scaleDraw );
}

//! \return Lower limit of the scale arc
double QwtDial::minScaleArc() const
{
    return d_data->minScaleArc;
}

//! \return Upper limit of the scale arc
double QwtDial::maxScaleArc() const
{
    return d_data->maxScaleArc;
}

/*!
  \brief Change the origin

  The origin is the angle where scale and needle is relative to.

  \param origin New origin
  \sa origin()
*/
void QwtDial::setOrigin( double origin )
{
    d_data->origin = origin;
    update();
}

/*!
  The origin is the angle where scale and needle is relative to.

  \return Origin of the dial
  \sa setOrigin()
*/
double QwtDial::origin() const
{
    return d_data->origin;
}

/*!
  Change the arc of the scale

  \param minArc Lower limit
  \param maxArc Upper limit
*/
void QwtDial::setScaleArc( double minArc, double maxArc )
{
    if ( minArc != 360.0 && minArc != -360.0 )
        minArc = ::fmod( minArc, 360.0 );
    if ( maxArc != 360.0 && maxArc != -360.0 )
        maxArc = ::fmod( maxArc, 360.0 );

    d_data->minScaleArc = qMin( minArc, maxArc );
    d_data->maxScaleArc = qMax( minArc, maxArc );
    if ( d_data->maxScaleArc - d_data->minScaleArc > 360.0 )
        d_data->maxScaleArc = d_data->minScaleArc + 360.0;

    update();
}

/*!
  \return Size hint
*/
QSize QwtDial::sizeHint() const
{
    int sh = 0;
    if ( scaleDraw() )
        sh = qCeil( scaleDraw()->extent( font() ) );

    const int d = 6 * sh + 2 * lineWidth();

    QSize hint( d, d ); 
    if ( !isReadOnly() )
        hint = hint.expandedTo( QApplication::globalStrut() );

    return hint;
}

/*!
  \brief Return a minimum size hint
  \warning The return value of QwtDial::minimumSizeHint() depends on the
           font and the scale.
*/
QSize QwtDial::minimumSizeHint() const
{
    int sh = 0;
    if ( scaleDraw() )
        sh = qCeil( scaleDraw()->extent( font() ) );

    const int d = 3 * sh + 2 * lineWidth();

    return QSize( d, d );
}

double QwtDial::scrolledTo( const QPoint &pos ) const
{
    return valueAt( pos ) - d_data->mouseOffset;
}

/*!
  Find the value for a given position

  \param pos Position
  \return Value
*/
double QwtDial::valueAt( const QPoint &pos ) const
{
    if ( d_data->maxScaleArc == d_data->minScaleArc || upperBound() == lowerBound() )
        return lowerBound();

    double dir = 360.0 - QLineF( innerRect().center(), pos ).angle() - d_data->origin;
    if ( dir < 0.0 )
        dir += 360.0;

    if ( mode() == RotateScale )
        dir = 360.0 - dir;

    // The position might be in the area that is outside the scale arc.
    // We need the range of the scale if it were a complete circle.

    const double completeCircle = 360.0 / ( d_data->maxScaleArc - d_data->minScaleArc )
        * ( upperBound() - lowerBound() );

    double posValue = lowerBound() + completeCircle * dir / 360.0;

    if ( d_data->previousDir >= 0.0 ) // valid direction
    {
        // We have to find out whether the mouse is moving
        // clock or counter clockwise

        bool clockWise = false;

        const double angle = dir - d_data->previousDir;
        if ( ( angle >= 0.0 && angle <= 180.0 ) || angle < -180.0 )
            clockWise = true;

        if ( clockWise )
        {
            if ( ( dir < d_data->previousDir ) 
                && ( d_data->mouseOffset > 0.0 ) )
            {
                // We passed 360 -> 0
                d_data->mouseOffset -= completeCircle;
            }

            if ( wrapping() )
            {
                if ( posValue - d_data->mouseOffset > upperBound() )
                {
                    // We passed maximum and the value will be set
                    // to minimum. We have to adjust the mouseOffset.

                    d_data->mouseOffset = posValue - lowerBound();
                }
            }
            else
            {
                if ( ( posValue - d_data->mouseOffset > upperBound() ) ||
                        ( value() == upperBound() ) )
                {
                    // We fix the value at maximum by adjusting
                    // the mouse offset.

                    d_data->mouseOffset = posValue - upperBound();
                }
            }
        }
        else
        {
            if ( dir > d_data->previousDir && d_data->mouseOffset < 0.0 )
            {
                // We passed 0 -> 360
                d_data->mouseOffset += completeCircle;
            }

            if ( wrapping() )
            {
                if ( posValue - d_data->mouseOffset < lowerBound() )
                {
                    // We passed minimum and the value will be set
                    // to maximum. We have to adjust the mouseOffset.

                    d_data->mouseOffset = posValue - upperBound();
                }
            }
            else
            {
                if ( posValue - d_data->mouseOffset < lowerBound() ||
                    value() == lowerBound() )
                {
                    // We fix the value at minimum by adjusting
                    // the mouse offset.

                    d_data->mouseOffset = posValue - lowerBound();
                }
            }
        }
    }
    d_data->previousDir = dir;

    return posValue;
}

bool QwtDial::isScrollPosition( const QPoint &pos ) const
{
    const QRegion region( innerRect().toRect(), QRegion::Ellipse );
    if ( region.contains( pos ) && ( pos != innerRect().center() ) )
    {
        d_data->mouseOffset = valueAt( pos ) - value();
        return true;
    }

    return false;
}

void QwtDial::mousePressEvent( QMouseEvent *event )
{
    d_data->previousDir = -1.0;
    QwtAbstractSlider::mousePressEvent( event );
}

void QwtDial::wheelEvent( QWheelEvent *event )
{
    const QRegion region( innerRect().toRect(), QRegion::Ellipse );
    if ( region.contains( event->pos() ) )
        QwtAbstractSlider::wheelEvent( event );
}

void QwtDial::setAngleRange( double angle, double span )
{
    QwtRoundScaleDraw *sd = const_cast<QwtRoundScaleDraw *>( scaleDraw() );
    if ( sd  )
    {
        double arc1 = angle - 270.0;
        if ( arc1 < -360.0 )
            arc1 = ::fmod( arc1, 360.0 );

        double arc2 = arc1 + span;
        if ( arc2 > 360.0 )
        {
            // QwtRoundScaleDraw::setAngleRange accepts only values
            // in the range [-360.0..360.0]
            arc1 -= 360.0;
            arc2 -= 360.0;
        }

        sd->setAngleRange( arc1, arc2 );
    }
}
