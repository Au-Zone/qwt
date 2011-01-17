#include "styledplot.h"
#include <qwt_plot_canvas.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qpainter.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qevent.h>

StyledPlot::StyledPlot( QWidget *parent ):
    QwtPlot( parent )
{
    canvas()->installEventFilter( this );
}

void StyledPlot::drawCanvas( QPainter *painter )
{
    if ( !d_canvasClip.isEmpty() )
        painter->setClipRegion( d_canvasClip, Qt::IntersectClip );

    QwtPlot::drawCanvas( painter );
}

void StyledPlot::updateCanvasClip()
{
    // The rounded borders of the styled background need to
    // clipped away, to avoid that we paint on them.

    if ( !canvas()->testAttribute( Qt::WA_StyledBackground ) )
    {
        d_canvasClip = QRegion();
        return;
    }

    // Let the style paint the styled background
    // to an image

    QImage image( size(), QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );

    QStyleOption opt;
    opt.initFrom( canvas() );
    style()->drawPrimitive( QStyle::PE_Widget, &opt, &painter, canvas() );
    painter.end();

    // Assuming we have no border in the center and solid background
    // color inside the frame we can calculate the mask from the
    // pixel in the center.

    const QRgb bg = image.pixel( image.width() / 2, image.height() / 2 );

    QImage mask = image.createMaskFromColor( bg, Qt::MaskOutColor );
    d_canvasClip = QBitmap::fromImage( mask );
}

bool StyledPlot::eventFilter( QObject* object, QEvent* event )
{
    if ( object == canvas() )
    {
        if ( event->type() == QEvent::Resize ||
                event->type() == QEvent::StyleChange )
        {
            // Calculating the canvas clip is an expensive
            // operation. So we better do this in advance and not
            // for every paint event.

            updateCanvasClip();
        }
    }

    return QwtPlot::eventFilter( object, event );
}

