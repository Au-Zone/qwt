#include "overlay.h"
#include <qpainter.h>
#include <qpaintengine.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qevent.h>

#define USE_IMAGE 1
static const QImage::Format imageFormat = QImage::Format_ARGB32_Premultiplied;

#if 0
static QImage qwtAlphaMask( const QImage &image )
{
    QImage mask(image.width(), image.height(), QImage::Format_MonoLSB);
    memset( mask.bits(), 0, mask.byteCount() );

    QVector<QRgb> colorTable;
    colorTable.append(0xffffffff);
    colorTable.append(0xff000000);

    mask.setColorTable( colorTable );

    const int w = image.width();
    const int h = image.height();

    for ( int y = 0; y < h; y++)
    {
        const uint *from = reinterpret_cast<const uint *> ( image.scanLine( y ) );
        const uint *end = from + w;

        uchar *to = mask.scanLine( y );

        int bit = 0;
        while ( from < end )
        {
            if ( *from++ )
                *to |= 1 << bit;

            if ( ++bit == 8 )
            {
                to++;
                bit = 0;
            }
        }
    }

    return mask;
}
#endif

static QRegion qwtToRegion( const QImage& image )
{
    const int w = image.width();
    const int h = image.height();

    QRegion region;
    QRect xr;

    for ( int y = 0; y < h; ++y ) 
    {
        const uint *line = reinterpret_cast<const uint *> ( image.scanLine( y ) );
        bool inRect = false;
        int x0 = -1;

        for ( int x = 0; x < w; x++ ) 
        {
            const bool on = ( line[x] != 0 );
            if ( on != inRect ) 
            {
                if ( inRect  ) 
                {
                    xr.setCoords( x0, y, x - 1, y );
                    region = region.united( xr );
                } 
                else 
                {
                    x0 = x;
                }

                inRect = !inRect;
            } 
        }

        if ( inRect ) 
        {
            xr.setCoords( x0, h - 1, w - 1, y );
            region = region.united( xr );
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

#if 0
    const QImage monoImage = qwtAlphaMask( image );
    const QBitmap bitmap = QBitmap::fromImage( monoImage );
    const QRegion mask( bitmap );
#else
    QRegion mask = qwtToRegion( image );
#endif

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
    QPainter painter( this );
    painter.setClipRegion( event->region() );

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
        painter.drawImage( 0, 0, image );
    }
    else
    {
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
