#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <qobject.h>
#include <qregion.h>
#include <qpointer.h>
#include <qwt_plot_overlay.h>

class QwtPlot;
class QwtPlotShapeItem;
class QPainter;
class QPoint;

class Editor: public QObject
{
    Q_OBJECT

public:
    Editor( QwtPlot * );
    virtual ~Editor();

    const QwtPlot *plot() const;
    QwtPlot *plot();

    virtual void setEnabled( bool on );
    bool isEnabled() const;

    void drawOverlay( QPainter * ) const;
    QRegion maskHint() const;

    virtual bool eventFilter( QObject *, QEvent *);

private:
    bool pressed( const QPoint & );
    bool moved( const QPoint & );
    void released( const QPoint & );

    QwtPlotShapeItem* itemAt( const QPoint& ) const;
    void raiseItem( QwtPlotShapeItem * );

    bool m_isEnabled;
    QPointer<QwtPlotOverlay> m_overlay;

    // Mouse positions
    QPointF m_currentPos;
    QwtPlotShapeItem* m_editedItem;
};

#endif
