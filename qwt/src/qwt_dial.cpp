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

static inline double qwtAngleDist( double a1, double a2 )
{
    double dist = qAbs( a2 - a1 );
    if ( dist > 360.0 )
        dist -= 360.0;

    return dist;
}

static inline bool qwtIsOnArc( double angle, double min, double max )
{
    if ( min < max )
    {
        return ( angle >= min ) && ( angle <= max );
    }
    else
    {
        return ( angle >= min ) || ( angle <= max );
    }
}

static inline double qwtBoundedAngle( double min, double angle, double max )
{
    double from = qwtNormalizeDegrees( min );
    double to = qwtNormalizeDegrees( max );

    double a;

    if ( qwtIsOnArc( angle, from, to ) )
    {
        a = angle;
        if ( a < min )
            a += 360.0;
    }
    else
    {
        if ( qwtAngleDist( angle, from ) <
            qwtAngleDist( angle, to ) )
        {
            a = min;
        }
        else
        {
            a = max;
        }
    }

    return a;
}

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
        arcOffset( 0.0 ),
        mouseOffset( 0.0 )
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

    double arcOffset;
    double mouseOffset;
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
QRect QwtDial::innerRect() const
{
    const int lw = lineWidth();
    return boundingRect().adjusted( lw, lw, -lw, -lw );
}

/*!
  \return bounding rect of the dial including the frame
  \sa setLineWidth(), scaleInnerRect(), innerRect()
*/
QRect QwtDial::boundingRect() const
{
    const QRect cr = contentsRect();

    const double dim = qMin( cr.width(), cr.height() );

    QRect inner( 0, 0, dim, dim );
    inner.moveCenter( cr.center() );

    return inner;
}

/*!
  \return rect inside the scale
  \sa setLineWidth(), boundingRect(), innerRect()
*/
QRect QwtDial::scaleInnerRect() const
{
    QRect rect = innerRect();

    const QwtAbstractScaleDraw *sd = scaleDraw();
    if ( sd )
    {
        int scaleDist = qCeil( sd->extent( font() ) );
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
        sliderChange();
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
    if ( lineWidth() > 0 )
    {
        QwtPainter::drawRoundFrame( painter, boundingRect(),
            palette(), lineWidth(), d_data->frameShadow );
    }
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

    const QPointF center = insideScaleRect.center();
    const double radius = 0.5 * insideScaleRect.width();

    painter->save();
    drawScale( painter, center, radius );
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
*/
void QwtDial::drawScale( QPainter *painter, 
    const QPointF &center, double radius ) const
{
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
    sliderChange();
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
    sliderChange();
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

    sliderChange();
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

bool QwtDial::isScrollPosition( const QPoint &pos ) const
{
    const QRegion region( innerRect(), QRegion::Ellipse );
    if ( region.contains( pos ) && ( pos != innerRect().center() ) )
    {
        double angle = QLineF( rect().center(), pos ).angle();
        if ( d_data->mode == QwtDial::RotateScale )
            angle = 360.0 - angle;

        double valueAngle = 
            qwtNormalizeDegrees( 90.0 - transform( value() ) );

        d_data->mouseOffset = qwtNormalizeDegrees( angle - valueAngle );
        d_data->arcOffset = scaleMap().p1();

        return true;
    }

    return false;
}


double QwtDial::scrolledTo( const QPoint &pos ) const
{
    double angle = QLineF( rect().center(), pos ).angle();
    if ( d_data->mode == QwtDial::RotateScale )
    {
        angle += scaleMap().p1() - d_data->arcOffset;
        angle = 360.0 - angle;
    }

    angle = qwtNormalizeDegrees( angle - d_data->mouseOffset );
    angle = qwtNormalizeDegrees( 90.0 - angle );

    if ( scaleMap().pDist() >= 360.0 )
    {
        if ( angle < scaleMap().p1() )
            angle += 360.0;

        if ( !wrapping() )
        {
            // ???
        }
    }
    else
    {
        const double boundedAngle =
            qwtBoundedAngle( scaleMap().p1(), angle, scaleMap().p2() );

        if ( !wrapping() )
            d_data->mouseOffset += ( boundedAngle - angle );

        angle = boundedAngle;
    }

    return invTransform( angle );
}

void QwtDial::wheelEvent( QWheelEvent *event )
{
    const QRegion region( innerRect(), QRegion::Ellipse );
    if ( region.contains( event->pos() ) )
        QwtAbstractSlider::wheelEvent( event );
}

void QwtDial::setAngleRange( double angle, double span )
{
    QwtRoundScaleDraw *sd = const_cast<QwtRoundScaleDraw *>( scaleDraw() );
    if ( sd  )
    {
        angle = qwtNormalizeDegrees( angle - 270.0 );
        sd->setAngleRange( angle, angle + span );
    }
}

void QwtDial::sliderChange()
{
    setAngleRange( d_data->origin + d_data->minScaleArc,
        d_data->maxScaleArc - d_data->minScaleArc );

    if ( mode() == RotateScale )
    {
        const double arc = transform( value() ) - scaleMap().p1();
        setAngleRange( d_data->origin - arc,
            d_data->maxScaleArc - d_data->minScaleArc );
    }

    QwtAbstractSlider::sliderChange();
}
