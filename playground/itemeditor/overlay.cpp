#include "overlay.h"
#include <qpainter.h>
#include <qpaintengine.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qevent.h>
#include <qelapsedtimer.h>
#include <qdebug.h>

#define USE_IMAGE 1
static const QImage::Format imageFormat = QImage::Format_ARGB32_Premultiplied;


static QRegion qwtAlphaMask( 
    const QImage& image, const QVector<QRect> rects )
{
    const int w = image.width();
    const int h = image.height();

    QRegion region;
    QRect rect;

    for ( int i = 0; i < rects.size(); i++ )
    {
        int x1, x2, y1, y2;
        rects[i].getCoords( &x1, &y1, &x2, &y2 );

        x1 = qMax( x1, 0 );
        x2 = qMin( x2, w - 1 );
        y1 = qMax( y1, 0 );
        y2 = qMin( y2, h - 1 );

        for ( int y = y1; y <= y2; ++y ) 
        {
            bool inRect = false;
            int rx0 = -1;

            const uint *line = 
                reinterpret_cast<const uint *> ( image.scanLine( y ) ) + x1;
            for ( int x = x1; x <= x2; x++ ) 
            {
                const bool on = ( *line++ != 0 );
                if ( on != inRect ) 
                {
                    if ( inRect  ) 
                    {
                        rect.setCoords( rx0, y, x - 1, y );
                        region += rect;
                    } 
                    else 
                    {
                        rx0 = x;
                    }

                    inRect = on;
                } 
            }

            if ( inRect ) 
            {
                rect.setCoords( rx0, y, x2, y );
                region = region.united( rect );
            }
        }
    }

    return region;
}

Overlay::Overlay( QWidget* parent ):
    QWidget( parent ),
    m_rgbaBuffer( NULL )
{
    setObjectName( "Overlay" );

    setAttribute( Qt::WA_TransparentForMouseEvents );
    setAttribute( Qt::WA_NoSystemBackground );
    setFocusPolicy( Qt::NoFocus );
}

Overlay::~Overlay()
{
    if ( m_rgbaBuffer )
        ::free( m_rgbaBuffer );
}

void Overlay::updateMask()
{
#if USE_IMAGE

    if ( m_rgbaBuffer )
        ::free( m_rgbaBuffer );

    m_rgbaBuffer = rgbaBuffer();

    const QImage image( m_rgbaBuffer, width(), height(), imageFormat );

    QRegion hint = maskHint();
    if ( hint.isEmpty() )
        hint += QRect( 0, 0, width(), height() );

    QRegion mask = qwtAlphaMask( image, hint.rects() );

#else
    QBitmap bitmap( width(), height() );
    bitmap.fill( Qt::color0 );

    QPainter painter( &bitmap );
    draw( &painter );

    const QRegion mask( bitmap );
#endif

    // A bug in Qt initiates a full repaint of the plot canvas
    // when we change the mask, while we are visible !

    setVisible( false );
    setMask( mask );
    setVisible( true );
}


void Overlay::paintEvent( QPaintEvent* event )
{
    const QRegion clipRegion = event->region();

    QPainter painter( this );

    bool useImage = false;
#if USE_IMAGE
    if ( painter.paintEngine()->type() == QPaintEngine::Raster )
        useImage = true;
#endif

    if ( useImage )
    {
        if ( m_rgbaBuffer == 0 )
            m_rgbaBuffer = rgbaBuffer();

        const QImage image( m_rgbaBuffer, width(), height(), imageFormat );

        QVector<QRect> rects;
        if ( clipRegion.rects().size() > 20 )
        {
            painter.setClipRegion( clipRegion );
            rects += clipRegion.boundingRect();
        }
        else
        {
            rects = clipRegion.rects();
        }

        for ( int i = 0; i < rects.size(); i++ )
        {
            const QRect r = rects[i];
            painter.drawImage( r.topLeft(), image, r );
        }
    }
    else
    {
        painter.setClipRegion( clipRegion );
        draw( &painter );
    }
}

void Overlay::resizeEvent( QResizeEvent* )
{
    if ( m_rgbaBuffer )
    {
        ::free( m_rgbaBuffer );
        m_rgbaBuffer = 0;
    }
}

uchar* Overlay::rgbaBuffer() const
{
    // A fresh buffer from calloc() is usually faster
    // then reinitializing an existing one with
    // QImage::fill( 0 ) or memset()

    uchar* buf = ( uchar* )::calloc( width() * height(), 4 );
    QImage image( buf, width(), height(), imageFormat );

    QPainter painter( &image );
    draw( &painter );

    return buf;
}

void Overlay::draw( QPainter *painter ) const
{
    if ( parentWidget() )
    {
        painter->setClipRect( parentWidget()->contentsRect() );

        QPainterPath clipPath;

        ( void )QMetaObject::invokeMethod(
            const_cast< QWidget *>( parentWidget() ), "borderPath", Qt::DirectConnection,
            Q_RETURN_ARG( QPainterPath, clipPath ), Q_ARG( QRect, rect() ) );

        if (!clipPath.isEmpty())
        {
            painter->setClipPath( clipPath, Qt::IntersectClip );
        }
    }

    drawOverlay( painter );
}

QRegion Overlay::maskHint() const
{
    return QRegion();
}
