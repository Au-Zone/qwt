#include <qwt_plot_curve.h>
#include <qwt_plot_legenditem.h>
#include <qwt_legend.h>
#include <qwt_plot_canvas.h>
#include "plot.h"

Plot::Plot( QWidget *parent ):
    QwtPlot( parent )
{
    canvas()->setFocusIndicator( QwtPlotCanvas::CanvasFocusIndicator );
    canvas()->setFocusPolicy( Qt::StrongFocus );

    setAutoReplot( false );

    setTitle( "Legend Test" );
    setFooter( "Footer" );

    for ( int i = 0; i < 4; i++ )
        insertCurve();

	QColor c( Qt::white );

	QwtPlotLegendItem *legendItem = new QwtPlotLegendItem();
	legendItem->setSpan( 2 );
	legendItem->setOrientation( Qt::Vertical );
	legendItem->setBorderRadius( 5 );

	c.setAlpha( 100 );
	legendItem->setBorderPen( c );
	c.setAlpha( 20 );
	legendItem->setBackgroundBrush( c );
	
	legendItem->attach( this );

    setAutoReplot( true );
}

void Plot::insertCurve()
{
    static int counter = 1;

    QString title("Curve %1");
    title = title.arg( counter ++ );

    QwtPlotCurve *curve = new QwtPlotCurve( title );
    curve->attach( this );
}

void Plot::insertLegend()
{
    static int counter = 0;

    QwtPlot::LegendPosition pos = 
        ( QwtPlot::LegendPosition ) ( counter++ % ( ExternalLegend + 1 ) );

    QwtLegend *legend = new QwtLegend();
    legend->setDefaultItemMode( QwtLegendData::Clickable );

    QwtPlot::insertLegend( legend, pos );

    connect( legend, SIGNAL( clicked( QwtPlotItem *, int ) ),
        SLOT( removeItem( QwtPlotItem * ) ) );
}

void Plot::removeItem( QwtPlotItem *item )
{
qDebug() << "removeItem: " << item;
    delete item;
}
