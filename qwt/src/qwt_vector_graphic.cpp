#include "qwt_vector_graphic.h"
#include "qwt_null_paintdevice.h"
#include <qpainter.h>

class QwtPicPaintDevice: public QwtNullPaintDevice
{
public:
    QwtPicPaintDevice( int dpiX, int dpiY ):
        QwtNullPaintDevice( QPaintEngine::AllFeatures ),
        boundingRect( 0.0, 0.0, -1.0, -1.0 ),
        d_dpi( dpiX, dpiY )
    {
        setMode( QwtNullPaintDevice::PathMode );
    }

    virtual void drawPath(const QPainterPath &path)
    {
        updateRect( path.boundingRect() );
    }

    virtual void drawPixmap(const QRectF &rect,
        const QPixmap &, const QRectF &)
    {
        updateRect( rect );
    }

    virtual void drawImage(const QRectF &rect,
        const QImage &, const QRectF &, Qt::ImageConversionFlags )
    {
        updateRect( rect );
    }

    virtual int metric( PaintDeviceMetric metric ) const
    {
        if ( metric == PdmDpiX )
            return d_dpi.width();

        if ( metric == PdmDpiY )
            return d_dpi.height();

        return QwtNullPaintDevice::metric( metric );
    }

    QRectF boundingRect;

private:
    void updateRect( const QRectF &rect )
    {
        const QPainter *painter = paintEngine()->painter();

        QRectF r = rect;
        if ( painter->hasClipping() ) 
            r &= painter->clipRegion().boundingRect();

        r = painter->transform().mapRect( r );

        if ( boundingRect.width() < 0 )
            boundingRect = r;
        else
            boundingRect |= r;
    }

    const QSize d_dpi;
};

class QwtVectorGraphic::PrivateData
{
public:
    PrivateData()
    {
    }
};

QwtVectorGraphic::QwtVectorGraphic()
{
    d_data = new PrivateData;
}

QwtVectorGraphic::~QwtVectorGraphic()
{
    delete d_data;
}

QwtVectorGraphic& QwtVectorGraphic::operator=(const QwtVectorGraphic &other)
{
    QPicture::operator=( other );
    *d_data = *other.d_data;

    return *this;
}

QRectF QwtVectorGraphic::boundingRectF() const
{
    QwtPicPaintDevice nullDevice( logicalDpiX(), logicalDpiY() );

    QPainter painter( &nullDevice );
    painter.drawPicture( QPointF(), *this );

    return nullDevice.boundingRect;
}

bool QwtVectorGraphic::operator==(const QwtVectorGraphic &other) const
{
    return ( &other == this );
}

bool QwtVectorGraphic::operator!=(const QwtVectorGraphic &other) const
{
    return !( operator==( other ) );
}
