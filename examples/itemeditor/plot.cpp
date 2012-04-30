#include "plot.h"
#include "editor.h"
#include <qwt_plot_shapeitem.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_canvas.h>
#include <qwt_legend.h>

class Legend: public QwtLegend
{
protected:
    virtual QWidget *createWidget( const QwtLegendData &data ) const
    {
        QWidget *w = QwtLegend::createWidget( data );
        if ( w )
        {
            w->setStyleSheet(
                "border-radius: 5px;"
                "padding: 2px;"
                "background: LemonChiffon;"
            );
        }

        return w;
    }
};

Plot::Plot( QWidget *parent ):
    QwtPlot( parent )
{
    setAutoReplot( false );

    setTitle( "Movable Items" );
    setPalette( QColor( "DimGray" ) );

    QwtPlotCanvas *canvas = new QwtPlotCanvas();
    canvas->setPalette( QColor( "LemonChiffon" ) );
    canvas->setLineWidth( 2 );
    canvas->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    canvas->setBorderRadius( 15 );

    setCanvas( canvas );

    insertLegend( new Legend(), QwtPlot::RightLegend );

    populate();

    updateAxes();
    for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
        setAxisAutoScale( axis, false );

    ( void ) new Editor( this );
    ( void ) new QwtPlotMagnifier( canvas );

}

void Plot::populate()
{
    addShape( "Rectangle", ShapeFactory::Rect, "RoyalBlue", 
        QPointF( 30.0, 50.0 ), QSizeF( 40.0, 50.0 ) );
    addShape( "Ellipse", ShapeFactory::Ellipse, "IndianRed", 
        QPointF( 80.0, 130.0 ), QSizeF( 50.0, 40.0 ) );
    addShape( "Ring", ShapeFactory::Ring, "DarkOliveGreen", 
        QPointF( 30.0, 165.0 ), QSizeF( 40.0, 40.0 ) );
    addShape( "Triangle", ShapeFactory::Triangle, "SandyBrown", 
        QPointF( 165.0, 165.0 ), QSizeF( 60.0, 40.0 ) );
    addShape( "Star", ShapeFactory::Star, "DarkViolet", 
        QPointF( 165.0, 50.0 ), QSizeF( 40.0, 50.0 ) );
    addShape( "Hexagon", ShapeFactory::Hexagon, "DarkSlateGray", 
        QPointF( 120.0, 70.0 ), QSizeF( 50.0, 50.0 ) );

}

void Plot::addShape( const QString &title,
    ShapeFactory::Shape shape, const QColor &color, 
    const QPointF &pos, const QSizeF &size )
{
    QwtPlotShapeItem *item = new QwtPlotShapeItem( title );
    item->setItemAttribute( QwtPlotItem::Legend, true );
    item->setLegendMode( QwtPlotShapeItem::LegendShape );
    item->setLegendIconSize( QSize( 20, 20 ) );
    item->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    item->setShape( ShapeFactory::path( shape, pos, size ) );

    QColor fillColor = color;
    fillColor.setAlpha( 200 );

    QPen pen( color, 3 );
    pen.setJoinStyle( Qt::MiterJoin );
    item->setPen( pen );
    item->setBrush( fillColor );

    item->attach( this );
}
