/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_knob.h"
#include "qwt_round_scale_draw.h"
#include "qwt_math.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include <qpainter.h>
#include <qpalette.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qevent.h>
#include <qmath.h>
#include <qapplication.h>

#if QT_VERSION < 0x040601
#define qAtan2(y, x) ::atan2(y, x)
#define qFabs(x) ::fabs(x)
#define qFastCos(x) qCos(x)
#define qFastSin(x) qSin(x)
#endif

static double qwtAngleOfValue( const QwtKnob *knob, double value )
{
    double angle = knob->transform( value );

    if ( knob->scaleMap().pDist() > 360.0 )
    {
        angle = ::fmod( angle, 360.0 );
        if ( angle < 0 )
            angle += 360.0;
    }

    return angle;
}

class QwtKnob::PrivateData
{
public:
    PrivateData():
        knobStyle( QwtKnob::Raised ),
        markerStyle( QwtKnob::Notch ),
        borderWidth( 2 ),
        borderDist( 4 ),
        scaleDist( 4 ),
        maxScaleTicks( 11 ),
        knobWidth( 50 ),
        markerSize( 8 ),
        totalAngle( 270.0 ),
        mouseOffset( 0.0 )
    {
    }

    QwtKnob::KnobStyle knobStyle;
    QwtKnob::MarkerStyle markerStyle;

    int borderWidth;
    int borderDist;
    int scaleDist;
    int maxScaleTicks;
    int knobWidth;
    int markerSize;

    double totalAngle;

    double mouseOffset;
};

/*!
  Constructor
  \param parent Parent widget
*/
QwtKnob::QwtKnob( QWidget* parent ):
    QwtAbstractSlider( parent )
{
    d_data = new PrivateData;

    setScaleDraw( new QwtRoundScaleDraw() );

    setTotalAngle( 270.0 );
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );

    setScale( 0.0, 10.0 );
    setValue( 0.0 );
}

//! Destructor
QwtKnob::~QwtKnob()
{
    delete d_data;
}

/*!
  \brief Set the knob type 

  \param knobStyle Knob type
  \sa knobStyle(), setBorderWidth()
*/
void QwtKnob::setKnobStyle( KnobStyle knobStyle )
{
    if ( d_data->knobStyle != knobStyle )
    {
        d_data->knobStyle = knobStyle;
        update();
    }
}

/*!
    \return Marker type of the knob
    \sa setKnobStyle(), setBorderWidth()
*/
QwtKnob::KnobStyle QwtKnob::knobStyle() const
{
    return d_data->knobStyle;
}

/*!
  \brief Set the marker type of the knob

  \param markerStyle Marker type
  \sa markerStyle(), setMarkerSize()
*/
void QwtKnob::setMarkerStyle( MarkerStyle markerStyle )
{
    if ( d_data->markerStyle != markerStyle )
    {
        d_data->markerStyle = markerStyle;
        update();
    }
}

/*!
    \return Marker type of the knob
    \sa setMarkerStyle(), setMarkerSize()
*/
QwtKnob::MarkerStyle QwtKnob::markerStyle() const
{
    return d_data->markerStyle;
}

/*!
  \brief Set the total angle by which the knob can be turned
  \param angle Angle in degrees.

  The default angle is 270 degrees. It is possible to specify
  an angle of more than 360 degrees so that the knob can be
  turned several times around its axis.
*/
void QwtKnob::setTotalAngle ( double angle )
{
    angle = qBound( 10.0, angle, 360.0 );

    if ( angle != d_data->totalAngle )
    {
        d_data->totalAngle = angle;

        scaleDraw()->setAngleRange( -0.5 * d_data->totalAngle,
            0.5 * d_data->totalAngle );

        updateGeometry();
        update();
    }
}

//! Return the total angle
double QwtKnob::totalAngle() const
{
    return d_data->totalAngle;
}

void QwtKnob::setNumTurns( int numTurns )
{
    numTurns = qMax( numTurns, 1 );

    if ( numTurns == 1 && d_data->totalAngle <= 360.0 )
        return;

    const double angle = numTurns * 360.0;
    if ( angle != d_data->totalAngle )
    {
        d_data->totalAngle = angle;

        scaleDraw()->setAngleRange( -0.5 * d_data->totalAngle,
            0.5 * d_data->totalAngle );

        updateGeometry();
        update();
    }
}

int QwtKnob::numTurns() const
{
    return qCeil( d_data->totalAngle / 360.0 );
}

/*!
   Change the scale draw of the knob

   For changing the labels of the scales, it
   is necessary to derive from QwtRoundScaleDraw and
   overload QwtRoundScaleDraw::label().

   \sa scaleDraw()
*/
void QwtKnob::setScaleDraw( QwtRoundScaleDraw *scaleDraw )
{
    setAbstractScaleDraw( scaleDraw );
    setTotalAngle( d_data->totalAngle );
}

/*!
   \return the scale draw of the knob
   \sa setScaleDraw()
*/
const QwtRoundScaleDraw *QwtKnob::scaleDraw() const
{
    return static_cast<const QwtRoundScaleDraw *>( abstractScaleDraw() );
}

/*!
   \return the scale draw of the knob
   \sa setScaleDraw()
*/
QwtRoundScaleDraw *QwtKnob::scaleDraw()
{
    return static_cast<QwtRoundScaleDraw *>( abstractScaleDraw() );
}

/*!
  \brief Set the scrolling mode and direction

  Called by QwtAbstractSlider
  \param pos Point in question
  \return Scrolling mode
*/
bool QwtKnob::isScrollPosition( const QPoint &pos ) const
{
    d_data->mouseOffset = 
        angleAt( pos ) - qwtAngleOfValue( this, value() );

    return true;
}

double QwtKnob::angleAt( const QPoint &pos ) const
{
    const double dx = rect().center().x() - pos.x();
    const double dy = rect().center().y() - pos.y();

    double angle = qAtan2( -dx, dy ) * 180.0 / M_PI;

    const double origin = 90.0;
    angle -= origin;

    return angle;
}

double QwtKnob::scrolledTo( const QPoint &pos ) const
{
    const double valueAngle = qwtAngleOfValue( this, value() );

    double angle = angleAt( pos );
    angle -= d_data->mouseOffset;

    if ( scaleMap().pDist() > 360.0 )
    {
        if ( angle < 0.0 )
            angle += 360.0;

        const double min = scaleMap().p1();
        const double v = transform( value() );

        int numTurns = qFloor( ( v - min ) / 360.0 );

        if ( qAbs( valueAngle - angle ) > 180.0 )
            numTurns += ( angle > valueAngle ) ? 1 : -1;

        angle += min + numTurns * 360.0;
    }
    else
    {
        angle = qBound( scaleMap().p1(), angle, scaleMap().p2() );

        const double arc = valueAngle - angle;
        if ( qAbs( arc ) > 180.0 )
        {
            const int numTurns = ( arc < 0 ) ? -1 : 1;
            angle += numTurns * 360.0;
        }
    }

    return invTransform( angle );
}

/*!
  Qt Resize Event
  \param event Resize event
*/
void QwtKnob::resizeEvent( QResizeEvent *event )
{
    Q_UNUSED( event );

    scaleDraw()->setRadius( 0.5 * d_data->knobWidth + d_data->scaleDist );
    scaleDraw()->moveCenter( rect().center() );
}

/*! 
  Handle QEvent::StyleChange and QEvent::FontChange;
  \param event Change event
*/
void QwtKnob::changeEvent( QEvent *event )
{
    switch( event->type() )
    {
        case QEvent::StyleChange:
        case QEvent::FontChange:
        {
            updateGeometry();
            update();
            break;
        }
        default:
            break;
    }
}

/*!
  Repaint the knob
  \param event Paint event
*/
void QwtKnob::paintEvent( QPaintEvent *event )
{
    QRectF knobRect( 0, 0, d_data->knobWidth, d_data->knobWidth );
    knobRect.moveCenter( rect().center() );

    QPainter painter( this );
    painter.setClipRegion( event->region() );

    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    painter.setRenderHint( QPainter::Antialiasing, true );

    if ( !knobRect.contains( event->region().boundingRect() ) )
        scaleDraw()->draw( &painter, palette() );

    drawKnob( &painter, knobRect );

    double angle = 0.0;
    if ( upperBound() != lowerBound() )
    {
        const double min = transform( lowerBound() );
        const double max = transform( upperBound() );
        const double tValue = transform( value() );

        angle = ( tValue - 0.5 * ( min + max ) )
            / ( max - min ) * d_data->totalAngle;

        const double numTurns = qFloor( ( angle + 180.0 ) / 360.0 );
        angle = angle - numTurns * 360.0;
    }

    drawMarker( &painter, knobRect, angle );

    painter.setRenderHint( QPainter::Antialiasing, false );

    if ( hasFocus() )
        QwtPainter::drawFocusRect( &painter, this );
}

/*!
  \brief Draw the knob
  \param painter painter
  \param knobRect Bounding rectangle of the knob (without scale)
*/
void QwtKnob::drawKnob( QPainter *painter, const QRectF &knobRect ) const
{
    double dim = qMin( knobRect.width(), knobRect.height() );
    dim -= d_data->borderWidth * 0.5;

    QRectF aRect( 0, 0, dim, dim );
    aRect.moveCenter( knobRect.center() );

    QPen pen( Qt::NoPen );
    if ( d_data->borderWidth > 0 )
    {
        QColor c1 = palette().color( QPalette::Light );
        QColor c2 = palette().color( QPalette::Dark );

        QLinearGradient gradient( aRect.topLeft(), aRect.bottomRight() );
        gradient.setColorAt( 0.0, c1 );
        gradient.setColorAt( 0.3, c1 );
        gradient.setColorAt( 0.7, c2 );
        gradient.setColorAt( 1.0, c2 );

        pen = QPen( gradient, d_data->borderWidth ); 
    }

    QBrush brush;
    switch( d_data->knobStyle )
    {
        case QwtKnob::Raised:
        {
            double off = 0.3 * knobRect.width();
            QRadialGradient gradient( knobRect.center(),
                knobRect.width(), knobRect.topLeft() + QPointF( off, off ) );
            
            gradient.setColorAt( 0.0, palette().color( QPalette::Midlight ) );
            gradient.setColorAt( 1.0, palette().color( QPalette::Button ) );

            brush = QBrush( gradient );

            break;
        }
        case QwtKnob::Sunken:
        {
            QLinearGradient gradient( 
                knobRect.topLeft(), knobRect.bottomRight() );
            gradient.setColorAt( 0.0, palette().color( QPalette::Mid ) );
            gradient.setColorAt( 0.5, palette().color( QPalette::Button ) );
            gradient.setColorAt( 1.0, palette().color( QPalette::Midlight ) );
            brush = QBrush( gradient );

            break;
        }
        default:
            brush = palette().brush( QPalette::Button );
    }

    painter->setPen( pen );
    painter->setBrush( brush );
    painter->drawEllipse( aRect );
}


/*!
  \brief Draw the marker at the knob's front
  \param painter Painter
  \param rect Bounding rectangle of the knob without scale
  \param angle Angle of the marker in degrees 
               ( clockwise, 0 at the 12 o'clock position )
*/
void QwtKnob::drawMarker( QPainter *painter, 
    const QRectF &rect, double angle ) const
{
    if ( d_data->markerStyle == NoMarker || !isValid() )
        return;

    const double radians = angle * M_PI / 180.0;
    const double sinA = -qFastSin( radians );
    const double cosA = qFastCos( radians );

    const double xm = rect.center().x();
    const double ym = rect.center().y();
    const double margin = 4.0;

    double radius = 0.5 * ( rect.width() - d_data->borderWidth ) - margin;
    if ( radius < 1.0 )
        radius = 1.0;

    switch ( d_data->markerStyle )
    {
        case Notch:
        case Nub:
        {
            const double dotWidth = 
                qMin( double( d_data->markerSize ), radius);

            const double dotCenterDist = radius - 0.5 * dotWidth;
            if ( dotCenterDist > 0.0 )
            {
                const QPointF center( xm - sinA * dotCenterDist, 
                    ym - cosA * dotCenterDist );

                QRectF ellipse( 0.0, 0.0, dotWidth, dotWidth );
                ellipse.moveCenter( center );

                QColor c1 = palette().color( QPalette::Light );
                QColor c2 = palette().color( QPalette::Mid );

                if ( d_data->markerStyle == Notch )
                    qSwap( c1, c2 );

                QLinearGradient gradient( 
                    ellipse.topLeft(), ellipse.bottomRight() );
                gradient.setColorAt( 0.0, c1 );
                gradient.setColorAt( 1.0, c2 );

                painter->setPen( Qt::NoPen );
                painter->setBrush( gradient );

                painter->drawEllipse( ellipse );
            }
            break;
        }
        case Dot:
        {
            const double dotWidth = 
                qMin( double( d_data->markerSize ), radius);

            const double dotCenterDist = radius - 0.5 * dotWidth;
            if ( dotCenterDist > 0.0 )
            {
                const QPointF center( xm - sinA * dotCenterDist, 
                    ym - cosA * dotCenterDist );

                QRectF ellipse( 0.0, 0.0, dotWidth, dotWidth );
                ellipse.moveCenter( center );

                painter->setPen( Qt::NoPen );
                painter->setBrush( palette().color( QPalette::ButtonText ) );
                painter->drawEllipse( ellipse );
            }

            break;
        }
        case Tick:
        {
            const double rb = qMax( radius - d_data->markerSize, 1.0 );
            const double re = radius;

            const QLineF line( xm - sinA * rb, ym - cosA * rb,
                xm - sinA * re, ym - cosA * re );

            QPen pen( palette().color( QPalette::ButtonText ), 0 );
            pen.setCapStyle( Qt::FlatCap );
            painter->setPen( pen );
            painter->drawLine ( line );

            break;
        }
        case Triangle:
        {
            const double rb = qMax( radius - d_data->markerSize, 1.0 );
            const double re = radius;

            painter->translate( rect.center() );
            painter->rotate( angle - 90.0 );
            
            QPolygonF polygon;
            polygon += QPointF( re, 0.0 );
            polygon += QPointF( rb, 0.5 * ( re - rb ) );
            polygon += QPointF( rb, -0.5 * ( re - rb ) );

            painter->setPen( Qt::NoPen );
            painter->setBrush( palette().color( QPalette::Text ) );
            painter->drawPolygon( polygon );

            painter->resetTransform();

            break;
        }
        default:
            break;
    }
}

/*!
  \brief Change the knob's width.

  The specified width must be >= 5, or it will be clipped.
  \param width New width
*/
void QwtKnob::setKnobWidth( int width )
{
    d_data->knobWidth = qMax( width, 5 );

    scaleDraw()->setRadius( 0.5 * d_data->knobWidth + d_data->scaleDist );
    scaleDraw()->moveCenter( rect().center() );
    
    updateGeometry();
    update();
}

//! Return the width of the knob
int QwtKnob::knobWidth() const
{
    return d_data->knobWidth;
}

/*!
  \brief Set the knob's border width
  \param borderWidth new border width
*/
void QwtKnob::setBorderWidth( int borderWidth )
{
    d_data->borderWidth = qMax( borderWidth, 0 );

    updateGeometry();
    update();

}

//! Return the border width
int QwtKnob::borderWidth() const
{
    return d_data->borderWidth;
}

/*!
  \brief Set the size of the marker
  \sa markerSize(), markerStyle()
*/
void QwtKnob::setMarkerSize( int size )
{
    if ( d_data->markerSize != size )
    {
        d_data->markerSize = size;
        update();
    }
}

//! Return the marker size
int QwtKnob::markerSize() const
{
    return d_data->markerSize;
}

/*!
    Recalculates the layout
*/
void QwtKnob::scaleChange()
{
#if 1
    // why here ???
    scaleDraw()->setRadius( 0.5 * d_data->knobWidth + d_data->scaleDist );
    scaleDraw()->moveCenter( rect().center() );
#endif

    QwtAbstractSlider::scaleChange();
}

/*!
  \return minimumSizeHint()
*/
QSize QwtKnob::sizeHint() const
{
    const QSize hint = minimumSizeHint();
    return hint.expandedTo( QApplication::globalStrut() );
}

/*!
  \brief Return a minimum size hint
  \warning The return value of QwtKnob::minimumSizeHint() depends on the
           font and the scale.
*/
QSize QwtKnob::minimumSizeHint() const
{
    // Add the scale radial thickness to the knobWidth
    const int extent = qCeil( scaleDraw()->extent( font() ) );
    const int d = 2 * ( extent + d_data->scaleDist ) + d_data->knobWidth;

    return QSize( d, d );
}
