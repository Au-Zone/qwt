/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_dial_needle.h"
#include "qwt_global.h"
#include "qwt_math.h"
#include "qwt_painter.h"
#include <qapplication.h>
#include <qpainter.h>

//! Constructor
QwtDialNeedle::QwtDialNeedle():
    d_palette( QApplication::palette() )
{
}

//! Destructor
QwtDialNeedle::~QwtDialNeedle()
{
}

/*!
    Sets the palette for the needle.

    \param palette New Palette
*/
void QwtDialNeedle::setPalette( const QPalette &palette )
{
    d_palette = palette;
}

/*!
  \return the palette of the needle.
*/
const QPalette &QwtDialNeedle::palette() const
{
    return d_palette;
}

//!  Draw the knob
void QwtDialNeedle::drawKnob( QPainter *painter,
    const QPointF &pos, double width, const QBrush &brush, bool sunken )
{
    painter->save();

    QRectF rect( 0, 0, width, width );
    rect.moveCenter( pos );

    painter->setPen( Qt::NoPen );
    painter->setBrush( brush );
    painter->drawEllipse( rect );

    painter->setBrush( Qt::NoBrush );

    const int colorOffset = 20;

    int startAngle = 45;
    if ( sunken )
        startAngle += 180;

    QPen pen;
    pen.setWidth( 1 );

    pen.setColor( brush.color().dark( 100 - colorOffset ) );
    painter->setPen( pen );
    painter->drawArc( rect, startAngle * 16, 180 * 16 );

    pen.setColor( brush.color().dark( 100 + colorOffset ) );
    painter->setPen( pen );
    painter->drawArc( rect, ( startAngle + 180 ) * 16, 180 * 16 );

    painter->restore();
}

/*!
  Constructor

  \param style Style
  \param hasKnob With/Without knob
  \param mid Middle color
  \param base Base color
*/
QwtDialSimpleNeedle::QwtDialSimpleNeedle( Style style, bool hasKnob,
        const QColor &mid, const QColor &base ):
    d_style( style ),
    d_hasKnob( hasKnob ),
    d_width( -1 )
{
    QPalette palette;
    for ( int i = 0; i < QPalette::NColorGroups; i++ )
    {
        palette.setColor( ( QPalette::ColorGroup )i,
            QPalette::Mid, mid );
        palette.setColor( ( QPalette::ColorGroup )i,
            QPalette::Base, base );
    }

    setPalette( palette );
}

/*!
  Set the width of the needle
  \param width Width
  \sa width()
*/
void QwtDialSimpleNeedle::setWidth( double width )
{
    d_width = width;
}

/*!
  \return the width of the needle
  \sa setWidth()
*/
double QwtDialSimpleNeedle::width() const
{
    return d_width;
}

/*!
 Draw the needle

 \param painter Painter
 \param center Center of the dial, start position for the needle
 \param length Length of the needle
 \param direction Direction of the needle, in degrees counter clockwise
 \param colorGroup Color group, used for painting
*/
void QwtDialSimpleNeedle::draw( QPainter *painter, const QPointF &center,
    double length, double direction, QPalette::ColorGroup colorGroup ) const
{
    if ( d_style == Arrow )
    {
        drawArrowNeedle( painter, palette(), colorGroup,
            center, length, d_width, direction, d_hasKnob );
    }
    else
    {
        drawRayNeedle( painter, palette(), colorGroup,
            center, length, d_width, direction, d_hasKnob );
    }
}

/*!
  Draw a needle looking like a ray

  \param painter Painter
  \param palette Palette
  \param colorGroup Color group
  \param center center of the needle
  \param length Length of the needle
  \param width Width of the needle
  \param direction Current Direction
  \param hasKnob With/Without knob
*/
void QwtDialSimpleNeedle::drawRayNeedle( QPainter *painter,
    const QPalette &palette, QPalette::ColorGroup colorGroup,
    const QPointF &center, double length, double width, double direction,
    bool hasKnob )
{
    if ( width <= 0 )
        width = 5;

    direction *= M_PI / 180.0;

    painter->save();

    const QPointF p1( center.x() + 1, center.y() + 2 );
    const QPointF p2 = qwtPolar2Pos( p1, length, direction );

    if ( width == 1 )
    {
        const QColor midColor =
            palette.color( colorGroup, QPalette::Mid );

        painter->setPen( QPen( midColor, 1 ) );
        painter->drawLine( p1, p2 );
    }
    else
    {
        QPolygonF pa;
        pa += qwtPolar2Pos( p1, width / 2, direction + M_PI_2 );
        pa += qwtPolar2Pos( p2, width / 2, direction + M_PI_2 );
        pa += qwtPolar2Pos( p2, width / 2, direction - M_PI_2 );
        pa += qwtPolar2Pos( p1, width / 2, direction - M_PI_2 );

        painter->setPen( Qt::NoPen );
        painter->setBrush( palette.brush( colorGroup, QPalette::Mid ) );
        painter->drawPolygon( pa );
    }
    if ( hasKnob )
    {
        const double knobWidth = qMax( qRound( width * 0.7 ), 5 );
        drawKnob( painter, center, knobWidth,
            palette.brush( colorGroup, QPalette::Base ),
            false );
    }

    painter->restore();
}

/*!
  Draw a needle looking like an arrow

  \param painter Painter
  \param palette Palette
  \param colorGroup Color group
  \param center center of the needle
  \param length Length of the needle
  \param width Width of the needle
  \param direction Current Direction
  \param hasKnob With/Without knob
*/
void QwtDialSimpleNeedle::drawArrowNeedle( QPainter *painter,
    const QPalette &palette, QPalette::ColorGroup colorGroup,
    const QPointF &center, double length, double width,
    double direction, bool hasKnob )
{
    direction *= M_PI / 180.0;

    painter->save();

    if ( width <= 0 )
        width = qMax( length * 0.06, 9.0 );

    const int peak = 3;
    const QPointF p1( center.x() + 1, center.y() + 1 );
    const QPointF p2 = qwtPolar2Pos( p1, length - peak, direction );
    const QPointF p3 = qwtPolar2Pos( p1, length, direction );

    QPolygonF pa;
    pa += qwtPolar2Pos( p1, width / 2, direction - M_PI_2 ) ;
    pa += qwtPolar2Pos( p2, 1, direction - M_PI_2 );
    pa += p3;
    pa += qwtPolar2Pos( p2, 1, direction + M_PI_2 );
    pa += qwtPolar2Pos( p1, width / 2, direction + M_PI_2 );

    painter->setPen( Qt::NoPen );
    painter->setBrush( palette.brush( colorGroup, QPalette::Mid ) );
    painter->drawPolygon( pa );

    const int colorOffset = 10;

    const QColor midColor = palette.color( colorGroup, QPalette::Mid );

    painter->setPen( midColor.dark( 100 + colorOffset ) );
    painter->drawPolyline( pa.data(), 3 );

    painter->setPen( midColor.dark( 100 - colorOffset ) );
    painter->drawPolyline( pa.data() + 2, 3 );

    if ( hasKnob )
    {
        drawKnob( painter, center, qRound( width * 1.3 ),
            palette.brush( colorGroup, QPalette::Base ),
            false );
    }

    painter->restore();
}

//! Constructor

QwtCompassMagnetNeedle::QwtCompassMagnetNeedle( Style style,
        const QColor &light, const QColor &dark ):
    d_style( style )
{
    QPalette palette;
    for ( int i = 0; i < QPalette::NColorGroups; i++ )
    {
        palette.setColor( ( QPalette::ColorGroup )i,
            QPalette::Light, light );
        palette.setColor( ( QPalette::ColorGroup )i,
            QPalette::Dark, dark );
        palette.setColor( ( QPalette::ColorGroup )i,
            QPalette::Base, Qt::darkGray );
    }

    setPalette( palette );
}

/*!
    Draw the needle

    \param painter Painter
    \param center Center of the dial, start position for the needle
    \param length Length of the needle
    \param direction Direction of the needle, in degrees counter clockwise
    \param colorGroup Color group, used for painting
*/
void QwtCompassMagnetNeedle::draw( QPainter *painter, const QPointF &center,
   double length, double direction, QPalette::ColorGroup colorGroup ) const
{
    if ( d_style == ThinStyle )
    {
        drawThinNeedle( painter, palette(), colorGroup,
            center, length, direction );
    }
    else
    {
        drawTriangleNeedle( painter, palette(), colorGroup,
            center, length, direction );
    }
}

/*!
  Draw a compass needle

  \param painter Painter
  \param palette Palette
  \param colorGroup Color group
  \param center Center, where the needle starts
  \param length Length of the needle
  \param direction Direction
*/
void QwtCompassMagnetNeedle::drawTriangleNeedle( QPainter *painter,
    const QPalette &palette, QPalette::ColorGroup colorGroup,
    const QPointF &center, double length, double direction )
{
    const QBrush darkBrush = palette.brush( colorGroup, QPalette::Dark );
    const QBrush lightBrush = palette.brush( colorGroup, QPalette::Light );

    QBrush brush;

    const double width = qRound( length / 3.0 );
    const int colorOffset =  10;

    painter->save();
    painter->setPen( Qt::NoPen );

    const QPoint arrowCenter( center.x() + 1, center.y() + 1 );

    QPolygonF pa;
    pa += arrowCenter;
    pa += qwtDegree2Pos( arrowCenter, length, direction );

    pa += qwtDegree2Pos( arrowCenter, width / 2, direction + 90.0 );

    brush = darkBrush;
    brush.setColor( brush.color().dark( 100 + colorOffset ) );
    painter->setBrush( brush );
    painter->drawPolygon( pa );

    pa[2] = qwtDegree2Pos( arrowCenter, width / 2, direction - 90.0 );

    brush = darkBrush;
    brush.setColor( brush.color().dark( 100 - colorOffset ) );
    painter->setBrush( brush );
    painter->drawPolygon( pa );

    // --

    pa[1] = qwtDegree2Pos( arrowCenter, length, direction + 180.0 );
    pa[2] = qwtDegree2Pos( arrowCenter, width / 2, direction + 90.0 );

    brush = lightBrush;
    brush.setColor( brush.color().dark( 100 + colorOffset ) );
    painter->setBrush( brush );
    painter->drawPolygon( pa );

    pa[2] = qwtDegree2Pos( arrowCenter, width / 2, direction - 90.0 );

    brush = lightBrush;
    brush.setColor( brush.color().dark( 100 - colorOffset ) );
    painter->setBrush( brush );
    painter->drawPolygon( pa );

    painter->restore();
}

/*!
  Draw a compass needle

  \param painter Painter
  \param palette Palette
  \param colorGroup Color group
  \param center Center, where the needle starts
  \param length Length of the needle
  \param direction Direction
*/
void QwtCompassMagnetNeedle::drawThinNeedle( QPainter *painter,
    const QPalette &palette, QPalette::ColorGroup colorGroup,
    const QPointF &center, double length, double direction )
{
    const QBrush darkBrush = palette.brush( colorGroup, QPalette::Dark );
    const QBrush lightBrush = palette.brush( colorGroup, QPalette::Light );
    const QBrush baseBrush = palette.brush( colorGroup, QPalette::Base );

    const int colorOffset = 10;
    const double width = qMax( length / 6.0, 3.0 );

    painter->save();

    const QPointF arrowCenter( center.x() + 1, center.y() + 1 );

    drawPointer( painter, darkBrush, colorOffset,
        arrowCenter, length, width, direction );
    drawPointer( painter, lightBrush, -colorOffset,
        arrowCenter, length, width, direction + 180.0 );

    drawKnob( painter, arrowCenter, width, baseBrush, true );

    painter->restore();
}

/*!
  Draw a compass needle

  \param painter Painter
  \param brush Brush
  \param colorOffset Color offset
  \param center Center, where the needle starts
  \param length Length of the needle
  \param width Width of the needle
  \param direction Direction
*/
void QwtCompassMagnetNeedle::drawPointer(
    QPainter *painter, const QBrush &brush,
    int colorOffset, const QPointF &center, double length,
    double width, double direction )
{
    painter->save();

    const double peak = qMax( length / 10.0, 5.0 );

    const double knobWidth = width + 8;
    QRectF knobRect( 0, 0, knobWidth, knobWidth );
    knobRect.moveCenter( center );

    QPolygonF pa;

    pa += qwtDegree2Pos( center, width / 2, direction + 90.0 );
    pa += center;
    pa += qwtDegree2Pos( pa[1], length - peak, direction );
    pa += qwtDegree2Pos( center, length, direction );
    pa += qwtDegree2Pos( pa[0], length - peak, direction );

    painter->setPen( Qt::NoPen );

    QBrush darkBrush = brush;
    darkBrush.setColor( darkBrush.color().dark( 100 + colorOffset ) );
    painter->setBrush( darkBrush );
    painter->drawPolygon( pa );
    painter->drawPie( knobRect, qRound( direction * 16 ), 90 * 16 );

    pa[0] = qwtDegree2Pos( center, 0.5 * width, direction - 90.0 );
    pa[4] = qwtDegree2Pos( pa[0], length - peak, direction );

    QBrush lightBrush = brush;
    lightBrush.setColor( lightBrush.color().dark( 100 - colorOffset ) );
    painter->setBrush( lightBrush );
    painter->drawPolygon( pa );
    painter->drawPie( knobRect, qRound( direction * 16 ), -90 * 16 );

    painter->restore();
}

/*!
   Constructor

   \param style Arrow style
   \param light Light color
   \param dark Dark color
*/
QwtCompassWindArrow::QwtCompassWindArrow( Style style,
        const QColor &light, const QColor &dark ):
    d_style( style )
{
    QPalette palette;
    for ( int i = 0; i < QPalette::NColorGroups; i++ )
    {
        palette.setColor( ( QPalette::ColorGroup )i,
            QPalette::Light, light );
        palette.setColor( ( QPalette::ColorGroup )i,
            QPalette::Dark, dark );
    }

    setPalette( palette );
}

/*!
 Draw the needle

 \param painter Painter
 \param center Center of the dial, start position for the needle
 \param length Length of the needle
 \param direction Direction of the needle, in degrees counter clockwise
 \param colorGroup Color group, used for painting
*/
void QwtCompassWindArrow::draw( QPainter *painter, const QPointF &center,
    double length, double direction, QPalette::ColorGroup colorGroup ) const
{
    if ( d_style == Style1 )
    {
        drawStyle1Needle( painter, palette(), colorGroup,
            center, length, direction );
    }
    else
    {
        drawStyle2Needle( painter, palette(), colorGroup,
            center, length, direction );
    }
}

/*!
  Draw a compass needle

 \param painter Painter
 \param palette Palette
 \param colorGroup colorGroup
 \param center Center of the dial, start position for the needle
 \param length Length of the needle
 \param direction Direction of the needle, in degrees counter clockwise
*/
void QwtCompassWindArrow::drawStyle1Needle( QPainter *painter,
    const QPalette &palette, QPalette::ColorGroup colorGroup,
    const QPointF &center, double length, double direction )
{
    const double AR1[] = {0, 0.4, 0.3, 1, 0.8, 1, 0.3, 0.4};
    const double AW1[] = {0, -45, -20, -15, 0, 15, 20, 45};

    const QPointF arrowCenter( center.x() + 1, center.y() + 1 );

    QPolygonF pa;
    pa += arrowCenter;
    for ( int i = 1; i < 8; i++ )
        pa += qwtDegree2Pos( center, AR1[i] * length, direction + AW1[i] );

    painter->save();
    painter->setPen( Qt::NoPen );
    painter->setBrush( palette.brush( colorGroup, QPalette::Light ) );
    painter->drawPolygon( pa );
    painter->restore();
}

/*!
  Draw a compass needle

 \param painter Painter
 \param palette Palette
 \param colorGroup colorGroup
 \param center Center of the dial, start position for the needle
 \param length Length of the needle
 \param direction Direction of the needle, in degrees counter clockwise
*/
void QwtCompassWindArrow::drawStyle2Needle( QPainter *painter,
    const QPalette &palette, QPalette::ColorGroup colorGroup,
    const QPointF &center, double length, double direction )
{
    const QBrush lightBrush = palette.brush( colorGroup, QPalette::Light );
    const QBrush darkBrush = palette.brush( colorGroup, QPalette::Dark );

    painter->save();
    painter->setPen( Qt::NoPen );

    const double angle = 12.0;
    const double ratio = 0.7;

    const QPointF arrowCenter( center.x() + 1, center.y() + 1 );

    QPolygonF pa;

    pa += center;
    pa += qwtDegree2Pos( arrowCenter, length, direction + angle );
    pa += qwtDegree2Pos( arrowCenter, ratio * length, direction );

    painter->setBrush( darkBrush );
    painter->drawPolygon( pa );

    pa[1] = qwtDegree2Pos( arrowCenter, length, direction - angle );
    painter->setBrush( lightBrush );
    painter->drawPolygon( pa );

    painter->restore();
}

