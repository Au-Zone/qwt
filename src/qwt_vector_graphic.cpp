#include "qwt_vector_graphic.h"
#include "qwt_painter_command.h"
#include <qvector.h>
#include <qpainter.h>
#include <qpaintengine.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpainterpath.h>
#include <qmath.h>

static bool qwtHasScalablePen( const QPainter *painter )
{
    const QPen pen = painter->pen();

    bool scalablePen = false;

    if ( pen.style() != Qt::NoPen && pen.brush().style() != Qt::NoBrush )
    {
        scalablePen = !pen.isCosmetic();
        if ( !scalablePen && pen.widthF() == 0.0 )
        {
            const QPainter::RenderHints hints = painter->renderHints();
            if ( hints.testFlag( QPainter::NonCosmeticDefaultPen ) )
                scalablePen = true;
        }
    }

    return scalablePen;
}


static QRectF qwtStrokedPathRect( 
    const QPainter *painter, const QPainterPath &path )
{
    QPainterPathStroker stroker;
    stroker.setWidth( painter->pen().widthF() );

    QRectF rect;
    if ( qwtHasScalablePen( painter ) )
    {
        QPainterPath stroke = stroker.createStroke(path);
        rect = painter->transform().map(stroke).boundingRect();
    }
    else
    {
        QPainterPath mappedPath = painter->transform().map(path);
        rect = stroker.createStroke( mappedPath ).boundingRect();
    }

    return rect;
}

static inline void qwtExecCommand( 
    QPainter *painter, const QwtPainterCommand &cmd, 
    QwtVectorGraphic::RenderHints renderHints,
    const QTransform &transform )
{
    switch( cmd.type() )
    {
        case QwtPainterCommand::Path:
        {
            bool doMap = false;

            if ( renderHints.testFlag( QwtVectorGraphic::RenderPensUnscaled )
                && painter->transform().isScaling() )
            {
                bool isCosmetic = painter->pen().isCosmetic();
                if ( isCosmetic && painter->pen().widthF() == 0.0 )
                {
                    QPainter::RenderHints hints = painter->renderHints();
                    if ( hints.testFlag( QPainter::NonCosmeticDefaultPen ) )
                        isCosmetic = false;
                }

                doMap = isCosmetic;
            }

            if ( doMap )
            {
                const QTransform transform = painter->transform();

                QPainterPath clipPath;
                if ( painter->hasClipping() )
                {
                    clipPath = painter->clipPath();
                    painter->setClipPath( transform.map( clipPath ) );
                }

                painter->resetTransform();
                painter->drawPath( transform.map( *cmd.path() ) );

                // restore painter
                painter->setTransform( transform );
                if ( painter->hasClipping() ) 
                    painter->setClipPath( clipPath );
            }
            else
            {
                painter->drawPath( *cmd.path() );
            }
            break;
        }
        case QwtPainterCommand::Pixmap:
        {
            const QwtPainterCommand::PixmapData *data = cmd.pixmapData();
            painter->drawPixmap( data->rect, data->pixmap, data->subRect );
            break;
        }
        case QwtPainterCommand::Image:
        {
            const QwtPainterCommand::ImageData *data = cmd.imageData();
            painter->drawImage( data->rect, data->image, 
                data->subRect, data->flags );
            break;
        }
        case QwtPainterCommand::State:
        {
            const QwtPainterCommand::StateData *data = cmd.stateData();

            if ( data->flags & QPaintEngine::DirtyPen ) 
                painter->setPen( data->pen );

            if ( data->flags & QPaintEngine::DirtyBrush ) 
                painter->setBrush( data->brush );

            if ( data->flags & QPaintEngine::DirtyBrushOrigin ) 
                painter->setBrushOrigin( data->brushOrigin );

            if ( data->flags & QPaintEngine::DirtyFont ) 
                painter->setFont( data->font );

            if ( data->flags & QPaintEngine::DirtyBackground ) 
            {
                painter->setBackgroundMode( data->backgroundMode );
                painter->setBackground( data->backgroundBrush );
            }

            if ( data->flags & QPaintEngine::DirtyTransform ) 
            {
                painter->setTransform( data->transform * transform );
            }

            if ( data->flags & QPaintEngine::DirtyClipEnabled ) 
                painter->setClipping( data->isClipEnabled );

            if ( data->flags & QPaintEngine::DirtyClipRegion) 
            {
                painter->setClipRegion( data->clipRegion, 
                    data->clipOperation );
            }

            if ( data->flags & QPaintEngine::DirtyClipPath ) 
            {
                painter->setClipPath( data->clipPath, data->clipOperation );
            }

            if ( data->flags & QPaintEngine::DirtyHints) 
            {
                const QPainter::RenderHints hints = data->renderHints;

                painter->setRenderHint( QPainter::Antialiasing,
                    hints.testFlag( QPainter::Antialiasing ) );

                painter->setRenderHint( QPainter::TextAntialiasing,
                    hints.testFlag( QPainter::TextAntialiasing ) );

                painter->setRenderHint( QPainter::SmoothPixmapTransform,
                    hints.testFlag( QPainter::SmoothPixmapTransform ) );

                painter->setRenderHint( QPainter::HighQualityAntialiasing,
                    hints.testFlag( QPainter::HighQualityAntialiasing ) );

                painter->setRenderHint( QPainter::NonCosmeticDefaultPen,
                    hints.testFlag( QPainter::NonCosmeticDefaultPen ) );
            }

            if ( data->flags & QPaintEngine::DirtyCompositionMode) 
                painter->setCompositionMode( data->compositionMode );

            if ( data->flags & QPaintEngine::DirtyOpacity) 
                painter->setOpacity( data->opacity );

            break;
        }
        default:
            break;
    }

}

class QwtVectorGraphic::PathInfo
{
public:
	PathInfo():
		d_scalablePen( false )
	{
		// QVector needs a default constructor
	}

	PathInfo( const QRectF &pointRect, 
			const QRectF &boundingRect, bool scalablePen ):
		d_pointRect( pointRect ),
		d_boundingRect( boundingRect ),
		d_scalablePen( scalablePen )
	{
	}

    inline QPointF scaleFactor( const QRectF& pathRect, 
        const QRectF &targetRect ) const
    {
        double sx = 0.0;
        double sy = 0.0;

        const QPointF p0 = d_pointRect.center();

        if ( pathRect.width() > 0.0 )
        {
            const double l = qAbs( pathRect.left() - p0.x() );
            const double r = qAbs( pathRect.right() - p0.x() );

            const double w = 2.0 * qMin( l, r ) 
                * targetRect.width() / pathRect.width();

            if ( d_scalablePen )
            {
                sx = w / d_boundingRect.width();
            }
            else
            {
                const double pw = qMax( 
					qAbs( d_boundingRect.left() - d_pointRect.left() ),
                    qAbs( d_boundingRect.right() - d_pointRect.right() ) );

                sx = ( w - pw ) / d_pointRect.width();
            }
        }

        if ( pathRect.height() > 0.0 )
        {
            const double t = qAbs( pathRect.top() - p0.y() );
            const double b = qAbs( pathRect.bottom() - p0.y() );

            const double h = 2.0 * qMin( t, b ) 
                * targetRect.height() / pathRect.height();

            if ( d_scalablePen )
            {
                sy = h / d_boundingRect.height();
            }
            else
            {
                const double pw = 
					qMax( qAbs( d_boundingRect.top() - d_pointRect.top() ),
                    qAbs( d_boundingRect.bottom() - d_pointRect.bottom() ) );

                sy = ( h - pw ) / d_pointRect.height();
            }
        }

        return QPointF( sx, sy );
    }

private:
    QRectF d_pointRect;
    QRectF d_boundingRect;
    bool d_scalablePen;
};

class QwtVectorGraphic::PrivateData
{
public:
    PrivateData():
        boundingRect( 0.0, 0.0, -1.0, -1.0 ),
        pointRect( 0.0, 0.0, -1.0, -1.0 )
    {
    }

    QSizeF defaultSize;
    QVector<QwtPainterCommand> commands;
    QVector<QwtVectorGraphic::PathInfo> pathInfos;

    QRectF boundingRect;
    QRectF pointRect;

    QwtVectorGraphic::RenderHints renderHints;
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
    d_data->pathInfos.clear();

    d_data->boundingRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
    d_data->pointRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
    d_data->defaultSize = QSizeF();

}

bool QwtVectorGraphic::isNull() const
{
    return d_data->commands.isEmpty();
}

bool QwtVectorGraphic::isEmpty() const
{
    return d_data->boundingRect.isEmpty();
}

/*!
   Toggle an render hint

   \param hint Render hint
   \param on true/false

   \sa testRenderHint(), RenderHint
*/
void QwtVectorGraphic::setRenderHint( RenderHint hint, bool on )
{
    if ( d_data->renderHints.testFlag( hint ) != on )
    {
        if ( on )
            d_data->renderHints |= hint;
        else
            d_data->renderHints &= ~hint;
    }
}

/*!
   Test a render hint

   \param hint Render hint
   \return true/false
   \sa setRenderHint(), RenderHint
*/
bool QwtVectorGraphic::testRenderHint( RenderHint hint ) const
{
    return d_data->renderHints.testFlag( hint );
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
    const QSizeF sz = defaultSize();
    return QSize( qCeil( sz.width() ), qCeil( sz.height() ) );
}

void QwtVectorGraphic::setDefaultSize( const QSizeF &size )
{
    const double w = qMax( 0.0, size.width() );
    const double h = qMax( 0.0, size.height() );

    d_data->defaultSize = QSizeF( w, h );
}

QSizeF QwtVectorGraphic::defaultSize() const
{
    if ( !d_data->defaultSize.isEmpty() )
        return d_data->defaultSize;

    return boundingRect().size();
}

void QwtVectorGraphic::render( QPainter *painter ) const
{
    if ( isNull() )
        return;

    const int numCommands = d_data->commands.size();
    const QwtPainterCommand *commands = d_data->commands.constData();

    const QTransform transform = painter->transform();

    painter->save();

    for ( int i = 0; i < numCommands; i++ )
    {
        qwtExecCommand( painter, commands[i], 
            d_data->renderHints, transform );
    }

    painter->restore();
}

void QwtVectorGraphic::render( QPainter *painter, const QSizeF &size, 
    Qt::AspectRatioMode aspectRatioMode ) const
{
    const QRectF r( 0.0, 0.0, size.width(), size.height() );
    render( painter, r, aspectRatioMode );
}

void QwtVectorGraphic::render( QPainter *painter, const QRectF &rect, 
    Qt::AspectRatioMode aspectRatioMode ) const
{
    if ( isEmpty() || rect.isEmpty() )
        return;

    double sx = 1.0; 
    double sy = 1.0;

    if ( d_data->pointRect.width() > 0.0 )
        sx = rect.width() / d_data->pointRect.width();

    if ( d_data->pointRect.height() > 0.0 )
        sy = rect.height() / d_data->pointRect.height();

    for ( int i = 0; i < d_data->pathInfos.size(); i++ )
    {
        const PathInfo info = d_data->pathInfos[i];

        const QPointF scaleFactor = 
            d_data->pathInfos[i].scaleFactor( d_data->pointRect, rect );

        if ( scaleFactor.x() > 0.0 )
            sx = qMin( sx, scaleFactor.x() );

        if ( scaleFactor.y() > 0.0 )
            sy = qMin( sy, scaleFactor.y() );
    }

    if ( aspectRatioMode == Qt::KeepAspectRatio )
    {
        const double s = qMin( sx, sy );
        sx = s;
        sy = s;
    }
    else if ( aspectRatioMode == Qt::KeepAspectRatioByExpanding )
    {
        const double s = qMax( sx, sy );
        sx = s;
        sy = s;
    }

#if 1
    QTransform tr0;
    tr0.scale( sx, sy );

    const QRectF scaledRect = tr0.mapRect( d_data->pointRect );

    const double dx = rect.x() - scaledRect.x();
    const double dy = rect.y() - scaledRect.y();

    QTransform tr;
    tr.translate( dx, dy );
    tr.scale( sx, sy );
#endif

    const QTransform transform = painter->transform();

    painter->setTransform( tr, true );

    render( painter );
    
    painter->setTransform( transform );
}

void QwtVectorGraphic::render( QPainter *painter, 
    const QPointF &pos, Qt::Alignment alignment ) const
{
    QRectF r( pos, defaultSize() );

    if ( alignment & Qt::AlignLeft )
    {
        r.moveLeft( pos.x() );
    }
    else if ( alignment & Qt::AlignHCenter )
    {
        r.moveCenter( QPointF( pos.x(), r.center().y() ) );
    }
    else if ( alignment & Qt::AlignRight )
    {
        r.moveRight( pos.x() );
    }

    if ( alignment & Qt::AlignTop )
    {
        r.moveTop( pos.y() );
    }
    else if ( alignment & Qt::AlignVCenter )
    {
        r.moveCenter( QPointF( r.center().x(), pos.y() ) );
    }
    else if ( alignment & Qt::AlignBottom )
    {
        r.moveBottom( pos.y() );
    }

    render( painter, r );
}

QPixmap QwtVectorGraphic::toPixmap() const
{
    if ( isNull() )
        return QPixmap();

    const QSizeF sz = defaultSize();

    const int w = qCeil( sz.width() );
    const int h = qCeil( sz.height() );

    QPixmap pixmap( w, h );
    pixmap.fill( Qt::transparent );

    QPainter painter( &pixmap );
    render( &painter, QRectF( 0.0, 0.0, sz.width(), sz.height() ) );
    painter.end();

    return pixmap;
}

QPixmap QwtVectorGraphic::toPixmap( const QSize &size,
    Qt::AspectRatioMode aspectRatioMode ) const
{
    QPixmap pixmap( size );
    pixmap.fill( Qt::transparent );

    const QRect r( 0, 0, size.width(), size.height() );

    QPainter painter( &pixmap );
    render( &painter, r, aspectRatioMode );
    painter.end();

    return pixmap;
}

QImage QwtVectorGraphic::toImage( const QSize &size,
    Qt::AspectRatioMode aspectRatioMode  ) const
{
    QImage image( size, QImage::Format_ARGB32 );
    image.fill( 0 );

    const QRect r( 0, 0, size.width(), size.height() );

    QPainter painter( &image );
    render( &painter, r, aspectRatioMode );
    painter.end();

    return image;
}

QImage QwtVectorGraphic::toImage() const
{
    if ( isNull() )
        return QImage();

    const QSizeF sz = defaultSize();

    const int w = qCeil( sz.width() );
    const int h = qCeil( sz.height() );

    QImage image( w, h, QImage::Format_ARGB32 );
    image.fill( 0 );

    QPainter painter( &image );
    render( &painter, QRectF( 0.0, 0.0, sz.width(), sz.height() ) );
    painter.end();

    return image;
}

void QwtVectorGraphic::drawPath( const QPainterPath &path )
{
    const QPainter *painter = paintEngine()->painter();
    if ( painter == NULL )
        return;

    d_data->commands += QwtPainterCommand( path );

    if ( !path.isEmpty() )
    {
        const QPainterPath scaledPath = painter->transform().map( path );

        QRectF pointRect = scaledPath.boundingRect();
        QRectF boundingRect = pointRect;

        if ( painter->pen().style() != Qt::NoPen 
            && painter->pen().brush().style() != Qt::NoBrush )
        {
            boundingRect = qwtStrokedPathRect( painter, path );
        }

        updatePointRect( pointRect );
        updateBoundingRect( boundingRect );

        d_data->pathInfos += PathInfo( pointRect, 
			boundingRect, qwtHasScalablePen( painter ) );
    }
}

void QwtVectorGraphic::drawPixmap(
    const QRectF &rect, const QPixmap &pixmap, 
    const QRectF &subRect )
{
    const QPainter *painter = paintEngine()->painter();
    if ( painter == NULL )
        return;

    d_data->commands += QwtPainterCommand( rect, pixmap, subRect );

    const QRectF r = painter->transform().mapRect( rect );
    updatePointRect( r );
    updateBoundingRect( r );
}

void QwtVectorGraphic::drawImage(
    const QRectF &rect, const QImage &image,
    const QRectF &subRect, Qt::ImageConversionFlags flags)
{
    const QPainter *painter = paintEngine()->painter();
    if ( painter == NULL )
        return;

    d_data->commands += QwtPainterCommand( rect, image, subRect, flags );

    const QRectF r = painter->transform().mapRect( rect );

    updatePointRect( r );
    updateBoundingRect( r );
}

void QwtVectorGraphic::updateState( const QPaintEngineState &state)
{
    d_data->commands += QwtPainterCommand( state );
}

void QwtVectorGraphic::updateBoundingRect( const QRectF &rect )
{
    QRectF br = rect;

    const QPainter *painter = paintEngine()->painter();
    if ( painter && painter->hasClipping() )
    {
        QRectF cr = painter->clipRegion().boundingRect();
        cr = painter->transform().mapRect( br );

        br &= cr;
    }

    if ( d_data->boundingRect.width() < 0 )
        d_data->boundingRect = br;
    else
        d_data->boundingRect |= br;
}

void QwtVectorGraphic::updatePointRect( const QRectF &rect )
{
    if ( d_data->pointRect.width() < 0.0 )
        d_data->pointRect = rect;
    else
        d_data->pointRect |= rect;
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
        qwtExecCommand( &painter, cmds[i], RenderHints(), QTransform() );

    painter.end();
}


