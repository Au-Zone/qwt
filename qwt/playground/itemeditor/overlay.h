#ifndef _OVERLAY_H_
#define _OVERLAY_H_

#include <qwidget.h>

class QPainter;

class Overlay: public QWidget
{
public:
    Overlay( QWidget* parent );
    virtual ~Overlay();

    void updateMask();

protected:
    virtual void paintEvent( QPaintEvent* event );
    virtual void resizeEvent( QResizeEvent* event );

    virtual void drawOverlay( QPainter * ) const = 0;

private:
    uchar* overlayImage() const;

private:
    uchar* m_imageBuffer;
};

#endif
