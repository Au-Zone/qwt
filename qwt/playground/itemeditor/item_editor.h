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

private:
    virtual bool pressed( const QPoint& );
    virtual bool moved( const QPoint& );
    virtual void released( const QPoint& );

    virtual void drawOverlay( QPainter* painter ) const;
    QwtPlotShapeItem* itemAt( const QPoint& ) const;

private:
    // Mouse positions
    QPointF m_currentPos;
    QwtPlotShapeItem* m_editedItem;
};

#endif
