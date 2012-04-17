#include "barchart.h"
#include <qwt_plot_renderer.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_barchart.h>
#include <qwt_column_symbol.h>
#include <qwt_plot_layout.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>

class DistroScaleDraw: public QwtScaleDraw
{
public:
    DistroScaleDraw( const QStringList &labels ):
        d_labels( labels )
    {
        setTickLength( QwtScaleDiv::MinorTick, 0 );
        setTickLength( QwtScaleDiv::MediumTick, 0 );

        setLabelRotation( -60.0 );
        setLabelAlignment( Qt::AlignLeft | Qt::AlignBottom );
    }

    virtual QwtText label( double value ) const
    {
        const int index = qRound( value );
        if ( index >= 0 && index <= d_labels.size() )
            return d_labels[ index ];

        return QwtText();
    }

private:
    const QStringList d_labels;
};

BarChart::BarChart( QWidget *parent ):
    QwtPlot( parent )
{
    const struct 
    {
        const char *distro;
        const int hits;
        QColor color;

    } pageHits[] =
    {
        { "Arch", 1116, QColor( Qt::blue ) },
        { "Debian", 1388, QColor( Qt::red ) },
        { "Fedora", 1483, QColor( Qt::darkBlue ) },
        { "Mageia", 1311, QColor( Qt::darkCyan ) },
        { "Mint", 3857, QColor( "MintCream" ) },
        { "openSuSE", 1604, QColor( Qt::darkGreen ) },
        { "Puppy", 1065, QColor( Qt::darkYellow ) }
    };

    QVector< double > samples;

    for ( uint i = 0; i < sizeof( pageHits ) / sizeof( pageHits[ 0 ] ); i++ )
    {
        d_distros += pageHits[ i ].distro;
        samples += pageHits[ i ].hits;
    }

    setAutoFillBackground( true );

    setPalette( Qt::white );
    canvas()->setPalette( QColor( "LemonChiffon" ) );

    setTitle( "DistroWatch Page Hit Ranking, April 2012" );

    setAxisTitle( QwtPlot::yLeft, "Hits per day ( HPD )" );
    setAxisTitle( QwtPlot::xBottom, "Distros" );

    d_barChartItem = new QwtPlotBarChart();
    d_barChartItem->setSamples( samples );

    QwtColumnSymbol *symbol = new QwtColumnSymbol( QwtColumnSymbol::Box );
    symbol->setLineWidth( 2 );
    symbol->setFrameStyle( QwtColumnSymbol::Raised );
    d_barChartItem->setSymbol( symbol );
    
    d_barChartItem->attach( this );

    insertLegend( new QwtLegend() );

    setOrientation( 0 );
    setAutoReplot( false );
}

void BarChart::setOrientation( int orientation )
{
    if ( orientation == 0 )
    {
        d_barChartItem->setOrientation( Qt::Vertical );

        setAxisScaleDraw( QwtPlot::xBottom, new DistroScaleDraw( d_distros ) );
        setAxisScaleDraw( QwtPlot::yLeft, new QwtScaleDraw() );

    }
    else
    {
        d_barChartItem->setOrientation( Qt::Horizontal );

        setAxisScaleDraw( QwtPlot::xBottom, new QwtScaleDraw() );
        setAxisScaleDraw( QwtPlot::yLeft, new DistroScaleDraw( d_distros ) );
    }

    replot();
}

void BarChart::exportChart()
{
    QwtPlotRenderer renderer;
    renderer.exportTo( this, "distrowatch.pdf" );
}
