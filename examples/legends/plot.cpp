#include "plot.h"
#include "settings.h"
#include <qwt_plot_curve.h>
#include <qwt_plot_legenditem.h>
#include <qwt_legend.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>

class LegendItem: public QwtPlotLegendItem
{
public:
    LegendItem()
    {
#if 1
        setBackgroundMode( LegendBackground );
        setBorderRadius( 10 );
        setRenderHint( QwtPlotItem::RenderAntialiased );
#else
        setBackgroundMode( ItemBackground );
#endif

        setMaxColumns( 1 );
        setAlignment( Qt::AlignRight | Qt::AlignVCenter );

        initColors();
    }

    void initColors()
    {
        QColor color( Qt::white );

        setTextPen( color );
#if 1
        setBorderPen( color );

        QColor c( Qt::gray );
        c.setAlpha( 200 );

        setBackgroundBrush( c );
#endif
    }
};

class Curve: public QwtPlotCurve
{
public:
    Curve()
    {
        setRenderHint( QwtPlotItem::RenderAntialiased );
        initData();
    }

    void initData()
    {
        QVector<QPointF> points;

        double y = qrand() % 1000;

        for ( double x = 0.0; x <= 1000.0; x += 100.0 )
        {
            double off = qrand() % 200 - 100;
            if ( y + off > 980.0 || y + off < 20.0 )
                off = -off;

            y += off;

            points += QPointF( x, y );
        }

        setSamples( points );
    }
};

Plot::Plot( QWidget *parent ):
    QwtPlot( parent ),
    d_legendItem( NULL )
{
    canvas()->setFocusIndicator( QwtPlotCanvas::CanvasFocusIndicator );
    canvas()->setFocusPolicy( Qt::StrongFocus );
    canvas()->setPalette( Qt::black );

    setAutoReplot( false );

    setTitle( "Legend Test" );
    setFooter( "Footer" );

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin( true );
    grid->setMajPen( QPen( Qt::gray, 0, Qt::DotLine ) );
    grid->setMinPen( QPen( Qt::darkGray, 0 , Qt::DotLine ) );
    grid->attach( this );

    // axis
    setAxisScale( QwtPlot::yLeft, 0.0, 1000.0 );
    setAxisScale( QwtPlot::xBottom, 0.0, 1000.0 );
}

void Plot::insertCurve()
{
    static int counter = 1;

    QString title("Curve %1");
    title = title.arg( counter ++ );

    const char *colors[] = 
    { 
        "LightSalmon",
        "HotPink",
        "Yellow",
        "Fuchsia",
        "PaleGreen",
        "PaleTurquoise",
        "SteelBlue",
        "Cornsilk",
        "Peru",
        "Maroon"
    };
    const int numColors = sizeof( colors ) / sizeof( colors[0] );

    QwtPlotCurve *curve = new Curve();
    curve->setTitle( title );
    curve->setPen( QPen( QColor( colors[ counter % numColors ] ), 2 ) );
    curve->attach( this );
}

void Plot::applySettings( const Settings &settings )
{
    if ( settings.legend.isEnabled )
    {
        QwtLegend *legend = new QwtLegend();
        legend->setWindowTitle("Plot Legend");

        insertLegend( legend, 
            QwtPlot::LegendPosition( settings.legend.position ) );

        if ( settings.legend.position == QwtPlot::ExternalLegend )
            legend->show();
    }
    else
    {
        insertLegend( NULL );
    }

    if ( settings.legendItem.isEnabled )
    {
        if ( d_legendItem == NULL )
        {
            d_legendItem = new LegendItem();
            d_legendItem->attach( this );
        }

        d_legendItem->setMaxColumns( settings.legendItem.numColumns );
        d_legendItem->setAlignment( Qt::Alignment( settings.legendItem.alignment ) );
        d_legendItem->setBackgroundMode(
            QwtPlotLegendItem::BackgroundMode( settings.legendItem.backgroundMode ) );
    }
    else
    {
        delete d_legendItem;
        d_legendItem = NULL;
    }

    if ( itemList( QwtPlotItem::Rtti_PlotCurve ).count() != settings.numCurves )
    {
        detachItems( QwtPlotItem::Rtti_PlotCurve, true );
        for ( int i = 0; i < settings.numCurves; i++ )
            insertCurve();
    }

    replot();
}
