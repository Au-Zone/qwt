#include "overlay.h"
#include <qpainter.h>
#include <qpaintengine.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qevent.h>

#define USE_IMAGE 1
static const QImage::Format imageFormat = QImage::Format_ARGB32_Premultiplied;

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

Overlay::Overlay( QWidget* parent ):
    QWidget( parent ),
    m_imageBuffer( 0 )
{
    setObjectName( "Overlay" );

    setAttribute( Qt::WA_TransparentForMouseEvents );
    setAttribute( Qt::WA_NoSystemBackground );
    setFocusPolicy( Qt::NoFocus );
}

Overlay::~Overlay()
{
    if ( m_imageBuffer )
        ::free( m_imageBuffer );
}

void Overlay::updateMask()
{
    if ( m_imageBuffer )
        ::free( m_imageBuffer );

#if USE_IMAGE
    m_imageBuffer = overlayImage();

    const QImage image( m_imageBuffer, width(), height(), imageFormat );
    const QBitmap mask = QBitmap::fromImage( qwtAlphaMask( image ) );
#else
    QBitmap mask( width(), height() );
    mask.fill( Qt::color0 );

    QPainter painter( &mask );
    drawOverlay( &painter );
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
        if ( m_imageBuffer == 0 )
            m_imageBuffer = overlayImage();

        const QImage image( m_imageBuffer, width(), height(), imageFormat );
        painter.drawImage( 0, 0, image );
    }
    else
    {
        drawOverlay( &painter );
    }
}

void Overlay::resizeEvent( QResizeEvent* )
{
    if ( m_imageBuffer )
    {
        ::free( m_imageBuffer );
        m_imageBuffer = 0;
    }
}

uchar* Overlay::overlayImage() const
{
    // A fresh buffer from calloc() is usually faster
    // then reinitializing an existing one with
    // QImage::fill( 0 ) or memset()

    uchar* buf = ( uchar* )::calloc( width() * height(), 4 );
    QImage image( buf, width(), height(), imageFormat );

    QPainter painter( &image );

    if ( parentWidget() )
    {
        painter.setClipRect( parentWidget()->contentsRect() );

        QPainterPath clipPath;

        ( void )QMetaObject::invokeMethod(
            const_cast< QWidget *>( parentWidget() ), "borderPath", Qt::DirectConnection,
            Q_RETURN_ARG( QPainterPath, clipPath ), Q_ARG( QRect, rect() ) );

        if (!clipPath.isEmpty())
        {
            painter.setClipPath( clipPath, Qt::IntersectClip );
        }
    }

    drawOverlay( &painter );

    return buf;
}
