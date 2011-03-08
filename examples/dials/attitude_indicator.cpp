#include "attitude_indicator.h"
#include <qwt_math.h>
#include <qevent.h>
#include <qpainter.h>
#include <qpolygon.h>

AttitudeIndicatorNeedle::AttitudeIndicatorNeedle(const QColor &c)
{
    QPalette palette;
    for ( int i = 0; i < QPalette::NColorGroups; i++ )
    {
        palette.setColor((QPalette::ColorGroup)i,
            QPalette::Text, c);
    }
    setPalette(palette);
}

void AttitudeIndicatorNeedle::draw(QPainter *painter, const QPointF &center,
    double length, double direction, QPalette::ColorGroup cg) const
{
    direction *= M_PI / 180.0;
    double triangleSize = length * 0.1;

    painter->save();

    const QPointF p1 = qwtPolar2Pos( center,
        length - 2 * triangleSize - 2, direction);

    QPolygonF pa;
    pa += qwtPolar2Pos( p1, 2 * triangleSize, direction );
    pa += qwtPolar2Pos( p1, triangleSize, direction + M_PI_2 );
    pa += qwtPolar2Pos( p1, triangleSize, direction - M_PI_2 );

    const QColor color = palette().color(cg, QPalette::Text);
    painter->setBrush(color);
    painter->drawPolygon(pa);

    painter->setPen( QPen(color, 3) );
    painter->drawLine( qwtPolar2Pos(center, length - 2, direction + M_PI_2),
        qwtPolar2Pos(center, length - 2, direction - M_PI_2));

    painter->restore();
}

AttitudeIndicator::AttitudeIndicator(
        QWidget *parent):
    QwtDial(parent),
    d_gradient(0.0)
{
    setMode(RotateScale);
    setWrapping(true);

    setOrigin(270.0);
    setScaleOptions(ScaleTicks);
    setScale(0, 0, 30.0);

    const QColor color = palette().color(QPalette::Text);
    setNeedle(new AttitudeIndicatorNeedle(color));
}

void AttitudeIndicator::setGradient(double gradient)
{
    if ( gradient < -1.0 )
         gradient = -1.0;
    else if ( gradient > 1.0 )
        gradient = 1.0;

    if ( d_gradient != gradient )
    {
        d_gradient = gradient;
        update();
    }
}

void AttitudeIndicator::drawScale(QPainter *painter, const QPointF &center,
    double radius, double origin, double minArc, double maxArc) const
{
    // counter clockwise, radian

    const double dir = (360.0 - origin) * M_PI / 180.0; 
    const double offset = 4.0;
    
    const QPointF p0 = qwtPolar2Pos( center, offset, dir + M_PI );

    const double w = innerRect().width();

    QPainterPath path;
    path.moveTo( qwtPolar2Pos( p0, w, dir - M_PI_2 ) );
    path.lineTo( qwtPolar2Pos( path.currentPosition(), 2 * w, dir + M_PI_2 ) );
    path.lineTo( qwtPolar2Pos( path.currentPosition(), w, dir ) );
    path.lineTo( qwtPolar2Pos( path.currentPosition(), w, dir - M_PI_2 ) );

    painter->save();
    painter->setClipPath( path ); // swallow 180 - 360 degrees

    QwtDial::drawScale(painter, 
        center, radius, origin, minArc, maxArc);

    painter->restore();
}

void AttitudeIndicator::drawScaleContents(QPainter *painter,
    const QPointF &, double) const
{
    int dir = 360 - qRound(origin() - value()); // counter clockwise
    int arc = 90 + qRound(gradient() * 90);

    const QColor skyColor(38, 151, 221);

    painter->save();
    painter->setBrush(skyColor);
    painter->drawChord(scaleInnerRect(), 
        (dir - arc) * 16, 2 * arc * 16 ); 
    painter->restore();
}

void AttitudeIndicator::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
        case Qt::Key_Plus:
            setGradient(gradient() + 0.05);
            break;
            
        case Qt::Key_Minus:
            setGradient(gradient() - 0.05);
            break;

        default:
            QwtDial::keyPressEvent(e);
    }
}
