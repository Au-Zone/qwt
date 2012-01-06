#include "qwt_painter_command.h"

QwtPainterCommand::QwtPainterCommand():
    d_type( Invalid )
{
}

QwtPainterCommand::QwtPainterCommand( const QPainterPath &path ):
    d_type( Path )
{
    d_path = new QPainterPath( path );
}

QwtPainterCommand::QwtPainterCommand( const QPolygon &polygon, 
        QPaintEngine::PolygonDrawMode mode ):
    d_type( Polygon )
{
    d_polygonData = new PolygonData();
    d_polygonData->polygon = polygon;
    d_polygonData->mode = mode;
}

QwtPainterCommand::QwtPainterCommand( const QPolygonF &polygon, 
        QPaintEngine::PolygonDrawMode mode ):
    d_type( PolygonF )
{
    d_polygonFData = new PolygonFData();
    d_polygonFData->polygonF = polygon;
    d_polygonFData->mode = mode;
}

QwtPainterCommand::QwtPainterCommand( const QRectF &rect,
        const QPixmap &pixmap, const QRectF& subRect ):
    d_type( Pixmap )
{
    d_pixmapData = new PixmapData();
    d_pixmapData->rect = rect;
    d_pixmapData->pixmap = pixmap;
    d_pixmapData->subRect = subRect;
}

QwtPainterCommand::QwtPainterCommand( const QRectF &rect,
        const QImage &image, const QRectF& subRect,
        Qt::ImageConversionFlags flags ):
    d_type( Image )
{
    d_imageData = new ImageData();
    d_imageData->rect = rect;
    d_imageData->image = image;
    d_imageData->subRect = subRect;
    d_imageData->flags = flags;
}

QwtPainterCommand::QwtPainterCommand( const QPaintEngineState &state ):
    d_type( State )
{
    d_stateData = new StateData();

    d_stateData->flags = state.state();

    if ( d_stateData->flags & QPaintEngine::DirtyPen) 
        d_stateData->pen = state.pen();

    if ( d_stateData->flags & QPaintEngine::DirtyBrush) 
        d_stateData->brush = state.brush();

    if ( d_stateData->flags & QPaintEngine::DirtyBrushOrigin) 
        d_stateData->brushOrigin = state.brushOrigin();

    if ( d_stateData->flags & QPaintEngine::DirtyFont) 
        d_stateData->font = state.font();

    if ( d_stateData->flags & QPaintEngine::DirtyBackground) 
    {
        d_stateData->backgroundMode = state.backgroundMode();
        d_stateData->backgroundBrush = state.backgroundBrush();
    }

    if ( d_stateData->flags & QPaintEngine::DirtyTransform) 
        d_stateData->transform = state.transform();

    if ( d_stateData->flags & QPaintEngine::DirtyClipEnabled) 
        d_stateData->isClipEnabled = state.isClipEnabled();

    if ( d_stateData->flags & QPaintEngine::DirtyClipRegion) 
    {
        d_stateData->clipRegion = state.clipRegion();
        d_stateData->clipOperation = state.clipOperation();
    }

    if ( d_stateData->flags & QPaintEngine::DirtyClipPath) 
    {
        d_stateData->clipPath = state.clipPath();
        d_stateData->clipOperation = state.clipOperation();
    }

    if ( d_stateData->flags & QPaintEngine::DirtyHints) 
        d_stateData->renderHints = state.renderHints();

    if ( d_stateData->flags & QPaintEngine::DirtyCompositionMode) 
        d_stateData->compositionMode = state.compositionMode();

    if ( d_stateData->flags & QPaintEngine::DirtyOpacity) 
        d_stateData->opacity = state.opacity();
}

QwtPainterCommand::QwtPainterCommand(const QwtPainterCommand &other)
{
    copy( other );
}

QwtPainterCommand::~QwtPainterCommand()
{
    reset();
}

QwtPainterCommand &QwtPainterCommand::operator=(const QwtPainterCommand &other)
{
    reset();
    copy( other );

    return *this;
}

void QwtPainterCommand::render( QPainter *painter ) const
{
    switch( d_type )
    {
        case Path:
        {
            painter->drawPath( *d_path );
            break;
        }
        case Polygon:
        {
            switch( d_polygonData->mode )
            {
                case QPaintEngine::PolylineMode:
                {
                    painter->drawPolyline( d_polygonData->polygon );
                    break;
                }
                case QPaintEngine::OddEvenMode:
                {
                    painter->drawPolygon( 
                        d_polygonData->polygon, Qt::OddEvenFill );
                    break;
                }
                case QPaintEngine::WindingMode:
                {
                    painter->drawPolygon( 
                        d_polygonData->polygon, Qt::WindingFill );
                    break;
                }
                case QPaintEngine::ConvexMode:
                {
                    painter->drawConvexPolygon( d_polygonData->polygon );
                    break;
                }
            }
            break;
        }
        case PolygonF:
        {
            switch( d_polygonFData->mode )
            {
                case QPaintEngine::PolylineMode:
                {
                    painter->drawPolyline( d_polygonFData->polygonF );
                    break;
                }
                case QPaintEngine::OddEvenMode:
                {
                    painter->drawPolygon( 
                        d_polygonFData->polygonF, Qt::OddEvenFill );
                    break;
                }
                case QPaintEngine::WindingMode:
                {
                    painter->drawPolygon( 
                        d_polygonFData->polygonF, Qt::WindingFill );
                    break;
                }
                case QPaintEngine::ConvexMode:
                {
                    painter->drawConvexPolygon( d_polygonFData->polygonF );
                    break;
                }
            }
            break;
        }
        case Pixmap:
        {
            painter->drawPixmap( d_pixmapData->rect, 
                d_pixmapData->pixmap, d_pixmapData->subRect );
            break;
        }
        case Image:
        {
            painter->drawImage( d_imageData->rect, 
                d_imageData->image, d_imageData->subRect, d_imageData->flags );
            break;
        }
        case State:
        {
            if ( d_stateData->flags & QPaintEngine::DirtyPen ) 
                painter->setPen( d_stateData->pen );

            if ( d_stateData->flags & QPaintEngine::DirtyBrush ) 
                painter->setBrush( d_stateData->brush );

            if ( d_stateData->flags & QPaintEngine::DirtyBrushOrigin ) 
                painter->setBrushOrigin( d_stateData->brushOrigin );

            if ( d_stateData->flags & QPaintEngine::DirtyFont ) 
                painter->setFont( d_stateData->font );

            if ( d_stateData->flags & QPaintEngine::DirtyBackground ) 
            {
                painter->setBackgroundMode( d_stateData->backgroundMode );
                painter->setBackground( d_stateData->backgroundBrush );
            }

            if ( d_stateData->flags & QPaintEngine::DirtyTransform ) 
                painter->setTransform( d_stateData->transform );

            if ( d_stateData->flags & QPaintEngine::DirtyClipEnabled ) 
                painter->setClipping( d_stateData->isClipEnabled );

            if ( d_stateData->flags & QPaintEngine::DirtyClipRegion) 
            {
                painter->setClipRegion( d_stateData->clipRegion, 
                    d_stateData->clipOperation );
            }

            if ( d_stateData->flags & QPaintEngine::DirtyClipPath ) 
            {
                painter->setClipPath( d_stateData->clipPath, 
                    d_stateData->clipOperation );
            }

            if ( d_stateData->flags & QPaintEngine::DirtyHints) 
            {
                const QPainter::RenderHints hints = d_stateData->renderHints;

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

            if ( d_stateData->flags & QPaintEngine::DirtyCompositionMode) 
                painter->setCompositionMode( d_stateData->compositionMode );

            if ( d_stateData->flags & QPaintEngine::DirtyOpacity) 
                painter->setOpacity( d_stateData->opacity );

            break;
        }
        default:
            break;
    }
}

void QwtPainterCommand::copy( const QwtPainterCommand &other )
{
    d_type = other.d_type;

    switch( other.d_type )
    {
        case Path:
        {
            d_path = new QPainterPath( *other.d_path );
            break;
        }
        case Polygon:
        {
            d_polygonData = new PolygonData( *other.d_polygonData );
            break;
        }
        case PolygonF:
        {
            d_polygonFData = new PolygonFData( *other.d_polygonFData );
            break;
        }
        case Pixmap:
        {
            d_pixmapData = new PixmapData( *other.d_pixmapData );
            break;
        }
        case Image:
        {
            d_imageData = new ImageData( *other.d_imageData );
            break;
        }
        case State:
        {
            d_stateData = new StateData( *other.d_stateData );
            break;
        }
        default:
            break;
    }
}

void QwtPainterCommand::reset()
{
    switch( d_type )
    {
        case Path:
        {
            delete d_path;
            break;
        }
        case Polygon:
        {
            delete d_polygonData;
            break;
        }
        case PolygonF:
        {
            delete d_polygonFData;
            break;
        }
        case Pixmap:
        {
            delete d_pixmapData;
            break;
        }
        case Image:
        {
            delete d_imageData;
            break;
        }
        case State:
        {
            delete d_stateData;
            break;
        }
        default:
            break;
    }

    d_type = Invalid;
}
