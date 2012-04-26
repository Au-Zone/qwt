#include "overlay.h"
#include <qpainter.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qevent.h>

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

    m_imageBuffer = overlayImage();

    const QImage image( m_imageBuffer, width(), height(),
                        QImage::Format_ARGB32_Premultiplied );

    const QBitmap mask = QBitmap::fromImage( image.createAlphaMask(
                             Qt::MonoOnly | Qt::ThresholdDither | Qt::NoOpaqueDetection ) );

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
    if ( m_imageBuffer == 0 )
       m_imageBuffer = overlayImage();

    const QImage image( m_imageBuffer, width(), height(),
                        QImage::Format_ARGB32_Premultiplied );

    painter.drawImage( 0, 0, image );
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

    QImage image( buf, width(), height(),
        QImage::Format_ARGB32_Premultiplied );

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
