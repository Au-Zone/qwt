#include "qwt_vector_graphic.h"
#include "qwt_painter_command.h"
#include <qvector.h>
#include <qpainter.h>
#include <qpaintengine.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpainterpath.h>
#include <qmath.h>

class QwtVectorGraphic::PrivateData
{
public:
    PrivateData():
        boundingRect( 0.0, 0.0, -1.0, -1.0 ),
        pointRect( 0.0, 0.0, -1.0, -1.0 )
    {
    }

    QVector<QwtPainterCommand> commands;
    QRectF boundingRect;
    QRectF pointRect;
};

QwtVectorGraphic::QwtVectorGraphic()
{
    setMode( QwtNullPaintDevice::PathMode );
    d_data = new PrivateData;
}

QwtVectorGraphic::QwtVectorGraphic( const QwtVectorGraphic &other )
{
    setMode( other.mode() );
    d_data = new PrivateData( *other.d_data );
}

QwtVectorGraphic::~QwtVectorGraphic()
{
    delete d_data;
}

QwtVectorGraphic& QwtVectorGraphic::operator=(const QwtVectorGraphic &other)
{
    setMode( other.mode() );
    *d_data = *other.d_data;

    return *this;
}

void QwtVectorGraphic::reset() 
{
    d_data->commands.clear();
    d_data->boundingRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
    d_data->pointRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
}

bool QwtVectorGraphic::isNull() const
{
    return d_data->commands.isEmpty();
}

QRectF QwtVectorGraphic::boundingRect() const
{
    if ( d_data->boundingRect.width() < 0 )
        return QRectF();

    return d_data->boundingRect;
}

QRectF QwtVectorGraphic::pointRect() const
{
    if ( d_data->pointRect.width() < 0 )
        return QRectF();

    return d_data->pointRect;
}

QSize QwtVectorGraphic::sizeMetrics() const
{
    const QSizeF sz = boundingRect().size();
    return QSize( qCeil( sz.width() ), qCeil( sz.height() ) );
}

void QwtVectorGraphic::render( QPainter *painter ) const
{
    const int numCommands = d_data->commands.size();
    if ( numCommands <= 0 )
        return;

    const QwtPainterCommand *commands = d_data->commands.constData();

    painter->save();

    for ( int i = 0; i < numCommands; i++ )
        commands[i].render( painter );

    painter->restore();
}

void QwtVectorGraphic::drawPolygon(
    const QPointF *points, int pointCount, 
    QPaintEngine::PolygonDrawMode mode)
{
    if ( pointCount <= 0 )
        return;

    QPolygonF polygon;
    polygon.reserve( pointCount );

    double minX = points[0].x();
    double maxX = points[0].x();
    double minY = points[0].y();
    double maxY = points[0].y();

    for ( int i = 0; i < pointCount; i++ )
    {
        polygon += points[i];

        minX = qMin( points[i].x(), minX );
        minY = qMin( points[i].y(), minY );
        maxX = qMax( points[i].x(), maxX );
        maxY = qMin( points[i].y(), maxY );
    }

    d_data->commands += QwtPainterCommand( polygon, mode );

    const QRectF br( minX, minY, maxX - minX, maxY - minY );
    updateRects( br, true );
}

void QwtVectorGraphic::drawPolygon(
    const QPoint *points, int pointCount, 
    QPaintEngine::PolygonDrawMode mode)
{
    if ( pointCount <= 0 )
        return;

    QPolygon polygon;
    polygon.reserve( pointCount );

    int minX = points[0].x();
    int maxX = points[0].x();
    int minY = points[0].y();
    int maxY = points[0].y();

    for ( int i = 0; i < pointCount; i++ )
    {
        polygon += points[i];

        minX = qMin( points[i].x(), minX );
        minY = qMin( points[i].y(), minY );
        maxX = qMax( points[i].x(), maxX );
        maxY = qMin( points[i].y(), maxY );
    }

    d_data->commands += QwtPainterCommand( polygon, mode );

    const QRectF br( minX, minY, maxX - minX, maxY - minY );
    updateRects( br, true );
}

void QwtVectorGraphic::drawPath( const QPainterPath &path )
{
    d_data->commands += QwtPainterCommand( path );
    updateRects( path.boundingRect(), true );
}

void QwtVectorGraphic::drawPixmap(
    const QRectF &rect, const QPixmap &pixmap, 
    const QRectF &subRect )
{
    d_data->commands += QwtPainterCommand( rect, pixmap, subRect );
    updateRects( rect, false );
}

void QwtVectorGraphic::drawImage(
    const QRectF &rect, const QImage &image,
    const QRectF &subRect, Qt::ImageConversionFlags flags)
{
    d_data->commands += QwtPainterCommand( rect, image, subRect, flags );
    updateRects( rect, false );
}

void QwtVectorGraphic::updateState(
    const QPaintEngineState &state)
{
    d_data->commands += QwtPainterCommand( state );
}

void QwtVectorGraphic::updateRects( const QRectF &rect, bool addPen )
{
    if ( ( rect.width() <= 0.0 ) && ( rect.height() <= 0.0 ) )
        return;

    QRectF br = rect;
    QRectF pr = rect;

    const QPainter *p = paintEngine()->painter();
    if ( p )
    {
        double w2 = 0.0;
        if ( addPen && p->pen().style() != Qt::NoPen )
            w2 = 0.5 * p->pen().widthF();

        // what about cosmetic pens ?
        br.adjust( -w2, -w2, w2, w2 );

        if ( p->hasClipping() )
        {
            const QRectF cr = p->clipRegion().boundingRect();

            br &= cr;
            pr &= cr;
        }

        br = p->transform().mapRect( br );
        pr = p->transform().mapRect( pr );
    }

    if ( d_data->boundingRect.width() < 0 )
        d_data->boundingRect = br;
    else
        d_data->boundingRect |= br;

    if ( d_data->pointRect.width() < 0 )
        d_data->pointRect = pr;
    else
        d_data->pointRect |= pr;
}

const QVector< QwtPainterCommand > &QwtVectorGraphic::commands() const
{
    return d_data->commands;
}

void QwtVectorGraphic::setCommands( QVector< QwtPainterCommand > &commands )
{
    reset();

    const int numCommands = commands.size();
    if ( numCommands <= 0 )
        return;

    // to calculate a proper bounding rectangle we don't simply copy 
    // the commands. 

    const QwtPainterCommand *cmds = commands.constData();

    QPainter painter( this );
    for ( int i = 0; i < numCommands; i++ )
        cmds[i].render( &painter );

    painter.end();
}

