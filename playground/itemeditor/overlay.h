#ifndef _OVERLAY_H_
#define _OVERLAY_H_

#include <qwidget.h>

class QPainter;

class Overlay: public QWidget
{
public:
    enum MaskMode
    {
        NoMask,

        ImageMask,
        BitmapMask
    };

    Overlay( QWidget* parent );
    virtual ~Overlay();

    void updateMask();

protected:
    virtual void paintEvent( QPaintEvent* event );
    virtual void resizeEvent( QResizeEvent* event );

    virtual void drawOverlay( QPainter * ) const = 0;
    virtual QRegion maskHint() const;

private:
    void draw( QPainter * ) const;
    uchar* rgbaBuffer() const;

private:
    uchar* m_rgbaBuffer;
};

#endif
