#ifndef _CANVAS_EDITOR_H_
#define _CANVAS_EDITOR_H_

#include <qobject.h>
#include <qregion.h>
#include <qpointer.h>
#include <qwt_plot_overlay.h>

class QwtPlot;
class QPainter;
class QPoint;

class CanvasEditor: public QObject
{
    Q_OBJECT

public:
    CanvasEditor( QwtPlot * );
    virtual ~CanvasEditor();

    virtual void setEnabled( bool on );
    bool isEnabled() const;

    virtual bool eventFilter( QObject *, QEvent *);

    virtual void drawOverlay( QPainter * ) const = 0;
    virtual QRegion maskHint() const = 0;

protected:
    virtual bool pressed( const QPoint & ) = 0;
    virtual bool moved( const QPoint & ) = 0;
    virtual void released( const QPoint & ) = 0;

private:
    bool m_isEnabled;
    QPointer<QwtPlotOverlay> m_overlay;
};

#endif
