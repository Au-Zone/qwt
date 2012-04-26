#include "plot.h"
#include "item_editor.h"
#include <qwt_plot_shapeitem.h>

Plot::Plot( QWidget *parent ):
    QwtPlot( parent )
{
    setAutoReplot( false );

    populate();

    updateAxes();
    for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
        setAxisAutoScale( axis, false );

    ItemEditor *editor = new ItemEditor( this );
    editor->setEnabled( true );
}

void Plot::populate()
{
    addRect( QRectF( 20.0, 30.0, 40.0, 50.0 ), Qt::blue );
    addEllipse( QRectF( 90.0, 110.0, 50.0, 40.0 ), Qt::red );
    addRect( QRectF( 200.0, 200.0, 30.0, 60.0 ), Qt::darkCyan );
    addEllipse( QRectF( 10.0, 140.0, 30.0, 50.0 ), Qt::darkYellow );
}

void Plot::addRect( const QRectF &rect, const QColor &color )
{
    QPainterPath path;
    path.addRect( rect );

    addShape( path, color );
}

void Plot::addEllipse( const QRectF &rect, const QColor &color )
{
    QPainterPath path;
    path.addEllipse( rect );

    addShape( path, color );
}

void Plot::addPolygon( const QPolygonF &polygon, const QColor &color )
{
    QPainterPath path;
    path.addPolygon( polygon );

    addShape( path, color );
}

void Plot::addShape( const QPainterPath &path, const QColor &color )
{
    QColor fillColor = color;
    fillColor.setAlpha( 128 );

    QwtPlotShapeItem *item = new QwtPlotShapeItem();
    item->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    item->setShape( path );
    item->setPen( QPen( color, 2 ) );
    item->setBrush( fillColor );

    item->attach( this );
}
