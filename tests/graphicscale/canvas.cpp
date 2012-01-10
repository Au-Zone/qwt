#include "canvas.h"
#include <qwt_vector_graphic.h>
#include <qsvgrenderer.h>
#include <qpicture.h>
#include <qdebug.h>

Canvas::Canvas( Mode mode, QWidget *parent ):
    QWidget( parent ),
    d_mode( mode ),
    d_type( Invalid )
{
    const int m = 10;
    setContentsMargins( m, m, m, m );
}

Canvas::~Canvas()
{
    reset();
}

void Canvas::setSvg( const QByteArray &bytearray )
{
    reset();

    d_type = Svg;
    d_renderer = new QSvgRenderer();
    d_renderer->load( bytearray );

    if ( d_mode == VectorGraphic )
        convertToGraphic();

    update();
}

void Canvas::setPicture( const QPicture &picture )
{
    reset();

    d_type = Picture;
    d_picture = new QPicture( picture );

    if ( d_mode == VectorGraphic )
        convertToGraphic();

    update();
}

void Canvas::convertToGraphic()
{
    QwtVectorGraphic *graphic = new QwtVectorGraphic();

    QPainter painter( graphic );
    render( &painter, QRectF( 0.0, 0.0, 100.0, 100.0 ) );
    painter.end();

    reset();

    d_type = Graphic;
    d_graphic = graphic;
}

void Canvas::reset()
{
    switch( d_type )
    {
        case Svg:
        {
            delete d_renderer;
            break;
        }
        case Picture:
        {
            delete d_picture;
            break;
        }
        case Graphic:
        {
            delete d_graphic;
            break;
        }
        default:
            break;
    }

    d_type = Invalid;
    update();
}

void Canvas::paintEvent( QPaintEvent * )
{
    QPainter painter( this );

    painter.save();

    painter.setPen( Qt::black );
    painter.setBrush( Qt::white );
    painter.drawRect( contentsRect().adjusted( 0, 0, -1, -1 ) );

    painter.restore();

    render( &painter, contentsRect() );
}

void Canvas::render( QPainter *painter, const QRectF &rect ) const
{
    switch( d_type )
    {
        case Svg:
        {
            d_renderer->render( painter, rect );
            break;
        }
        case Picture:
        {
            renderPicture( painter, rect );
            break;
        }
        case Graphic:
        {
            d_graphic->render( painter, rect );
            break;
        }
        default:
            break;
    }
}

void Canvas::renderPicture( QPainter *painter, const QRectF &rect ) const
{
    const QRectF br = d_picture->boundingRect();

    double sx = 1.0;
    double sy = 1.0;

    if ( br.width() > 0.0 )
        sx = rect.width() / br.width();

    if ( br.height() > 0.0 )
        sy = rect.height() / br.height();

    const double dx = sx * br.center().x();
    const double dy = sy * br.center().y();

    QTransform transform = painter->transform();
    transform.translate( rect.center().x() - dx,
        rect.center().y() - dy );
    transform.scale( sx, sy );

    painter->save();

    painter->setTransform( transform );
    d_picture->play( painter );

    painter->restore();
}
