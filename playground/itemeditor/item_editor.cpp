#include "item_editor.h"
#include <qwt_plot_shapeitem.h>
#include <qwt_scale_map.h>
#include <qwt_plot.h>

ItemEditor::ItemEditor( QwtPlot* plot ):
    CanvasEditor( plot ),
    m_editedItem( NULL )
{
    setEnabled( false );
}

ItemEditor::~ItemEditor()
{
}

QwtPlot *ItemEditor::plot()
{
    return qobject_cast<QwtPlot *>( parent() );
}

const QwtPlot *ItemEditor::plot() const
{
    return qobject_cast<const QwtPlot *>( parent() );
}

bool ItemEditor::pressed( const QPoint& pos )
{
    m_editedItem = itemAt( pos );
    if ( m_editedItem )
    {
        m_currentPos = pos;
        m_editedItem->setVisible( false );
        
        if ( plot() )
            plot()->replot();

        return true;
    }

    return false; // don't accept the position
}

bool ItemEditor::moved( const QPoint& pos )
{
    if ( plot() == NULL )
        return false;

    const QwtScaleMap xMap = plot()->canvasMap( m_editedItem->xAxis() );
    const QwtScaleMap yMap = plot()->canvasMap( m_editedItem->yAxis() );

    const QPointF p1 = QwtScaleMap::invTransform( xMap, yMap, m_currentPos );
    const QPointF p2 = QwtScaleMap::invTransform( xMap, yMap, pos );

    m_editedItem->setShape( m_editedItem->shape().translated( p2 - p1 ) );
    m_currentPos = pos;

    return true;
}


void ItemEditor::released( const QPoint& pos )
{
    Q_UNUSED( pos );

    if ( m_editedItem  )
    {
        raiseItem( m_editedItem );
        m_editedItem->setVisible( true );
        m_editedItem = NULL;

        if ( plot() )
            plot()->replot();
    }
}

QwtPlotShapeItem* ItemEditor::itemAt( const QPoint& pos ) const
{
    const QwtPlot *plot = this->plot();
    if ( plot == NULL )
        return NULL;

    // translate pos into the plot coordinates
    double coords[ QwtPlot::axisCnt ];
    coords[ QwtPlot::xBottom ] =
        plot->canvasMap( QwtPlot::xBottom ).invTransform( pos.x() );
    coords[ QwtPlot::xTop ] =
        plot->canvasMap( QwtPlot::xTop ).invTransform( pos.x() );
    coords[ QwtPlot::yLeft ] =
        plot->canvasMap( QwtPlot::yLeft ).invTransform( pos.y() );
    coords[ QwtPlot::yRight ] =
        plot->canvasMap( QwtPlot::yRight ).invTransform( pos.y() );

    QwtPlotItemList items = plot->itemList();
    for ( int i = items.size() - 1; i >= 0; i-- )
    {
        QwtPlotItem *item = items[ i ];
        if ( item->isVisible() && 
            item->rtti() == QwtPlotItem::Rtti_PlotShape )
        {
            QwtPlotShapeItem *shapeItem = static_cast<QwtPlotShapeItem *>( item );
            const QPointF p( coords[ item->xAxis() ], coords[ item->yAxis() ] );
            
            if ( shapeItem->boundingRect().contains( p )
                && shapeItem->shape().contains( p ) )
            {
                return shapeItem;
            }
        }
    }

    return NULL;
}

void ItemEditor::drawOverlay( QPainter* painter ) const
{
    const QwtPlot *plot = this->plot();
    if ( plot == NULL || m_editedItem == NULL )
        return;

    const QwtScaleMap xMap = plot->canvasMap( QwtPlot::xBottom );
    const QwtScaleMap yMap = plot->canvasMap( QwtPlot::yLeft );

    painter->setRenderHint( QPainter::Antialiasing,
        m_editedItem->testRenderHint( QwtPlotItem::RenderAntialiased ) );
    m_editedItem->draw( painter, xMap, yMap,
        plot->canvas()->contentsRect() );
}

void ItemEditor::raiseItem( QwtPlotShapeItem *shapeItem )
{
    const QwtPlot *plot = this->plot();
    if ( plot == NULL || shapeItem == NULL )
        return;

    const QwtPlotItemList items = plot->itemList();
    for ( int i = items.size() - 1; i >= 0; i-- )
    {
        QwtPlotItem *item = items[ i ];
        if ( shapeItem == item )
            return;
        
        if ( item->isVisible() &&
            item->rtti() == QwtPlotItem::Rtti_PlotShape )
        {
            shapeItem->setZ( item->z() + 1 );
            return;
        }
    }
}
