#include <qwidget.h>

class QByteArray;
class QSvgRenderer;
class QPicture;
class QwtVectorGraphic;

class Canvas: public QWidget
{
public:
    enum Mode
    {
        Native,
        VectorGraphic
    };

    Canvas( Mode, QWidget *parent = NULL );
    virtual ~Canvas();

    void setMode( Mode );

    void setSvg( const QByteArray & );
    void setPicture( const QPicture & );

    void reset();

protected:
    virtual void paintEvent( QPaintEvent * );

private:
    void render( QPainter *, const QRectF & ) const;
    void renderPicture( QPainter *, const QRectF & ) const;

    void convertToGraphic();

    const Mode d_mode;

    enum Type
    {
        Invalid = -1,

        Svg,
        Picture,
        Graphic
    };

    Type d_type;

    union
    {
        QSvgRenderer *d_renderer;
        QPicture *d_picture;
        QwtVectorGraphic *d_graphic;
    };
};
