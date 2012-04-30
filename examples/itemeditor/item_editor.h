#ifndef _ITEM_EDITOR_H_
#define _ITEM_EDITOR_H_

#include "canvas_editor.h"
#include <QPointF>

class QwtPlotShapeItem;

class ItemEditor: public CanvasEditor
{
    Q_OBJECT

public:
    ItemEditor( QwtPlot * );
    virtual ~ItemEditor();

    const QwtPlot *plot() const;
    QwtPlot *plot(); 

protected:
    virtual void drawOverlay( QPainter* painter ) const;
    virtual QRegion maskHint() const;

private:
    virtual bool pressed( const QPoint& );
    virtual bool moved( const QPoint& );
    virtual void released( const QPoint& );

    QwtPlotShapeItem* itemAt( const QPoint& ) const;
    void raiseItem( QwtPlotShapeItem * );

private:
    // Mouse positions
    QPointF m_currentPos;
    QwtPlotShapeItem* m_editedItem;
};

#endif
