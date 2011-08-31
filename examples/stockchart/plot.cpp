#include "plot.h"
#include "quotefactory.h"
#include <qwt_legend.h>
#include <qwt_plot_tradingcurve.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_legend_item.h>
#include <qfiledialog.h>
#include <qimagewriter.h>
#include <qprintdialog.h>
#include <qfileinfo.h>

Plot::Plot( QWidget *parent ):
    QwtPlot( parent )
{
    setTitle( "Financial Chart 2010" );

    setAxisTitle( QwtPlot::xBottom, "2010" );
    setAxisTitle( QwtPlot::yLeft, QString( "Price [EUR]" ) );

    QwtLegend *legend = new QwtLegend;
    legend->setItemMode( QwtLegend::CheckableItem );
    insertLegend( legend, QwtPlot::RightLegend );

    populate();

    // LeftButton for the zooming
    // MidButton for the panning
    // RightButton: zoom out by 1
    // Ctrl+RighButton: zoom out to full size

    QwtPlotZoomer* zoomer = new QwtPlotZoomer( canvas() );
    zoomer->setRubberBandPen( QColor( Qt::black ) );
    zoomer->setTrackerPen( QColor( Qt::black ) );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
        Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
        Qt::RightButton );

    QwtPlotPanner *panner = new QwtPlotPanner( canvas() );
    panner->setMouseButton( Qt::MidButton );

    connect( this, SIGNAL( legendChecked( QwtPlotItem *, bool ) ),
        SLOT( showCurve( QwtPlotItem *, bool ) ) );
}

void Plot::populate()
{
    const Qt::GlobalColor colors[] =
    {
        Qt::red,
        Qt::blue,
        Qt::darkCyan,
        Qt::darkMagenta,
        Qt::darkYellow
    };

    const int numColors = sizeof( colors ) / sizeof( colors[0] );

    for ( int i = 0; i < QuoteFactory::NumStocks; i++ )
    {
        QuoteFactory::Stock stock = static_cast<QuoteFactory::Stock>( i );

        QwtPlotTradingCurve *curve = new QwtPlotTradingCurve();
        curve->setTitle( QuoteFactory::title( stock ) );
        curve->setSamples( QuoteFactory::samples2010( stock ) );

		const Qt::GlobalColor color = colors[ i % numColors ];

        curve->setSymbolPen( QPen( color ) );
        curve->setSymbolBrush( QwtPlotTradingCurve::Decreasing, color );
        curve->setSymbolBrush( QwtPlotTradingCurve::Increasing, Qt::white );
        curve->attach( this );

        showCurve( curve, true );
    }
}

void Plot::setMode( int style )
{
    QwtPlotTradingCurve::SymbolStyle symbolStyle =
        static_cast<QwtPlotTradingCurve::SymbolStyle>( style );

    QwtPlotItemList curves = itemList( QwtPlotItem::Rtti_PlotTradingCurve );
    for ( int i = 0; i < curves.size(); i++ )
    {
        QwtPlotTradingCurve *curve =
            static_cast<QwtPlotTradingCurve *>( curves[i] );
        curve->setSymbolStyle( symbolStyle );
    }

    replot();
}

void Plot::showCurve( QwtPlotItem *item, bool on )
{
    item->setVisible( on );

    QwtLegendItem *legendItem =
        qobject_cast<QwtLegendItem *>( legend()->find( item ) );

    if ( legendItem )
        legendItem->setChecked( on );

    replot();
}

void Plot::exportPlot()
{
#ifndef QT_NO_PRINTER
    QString fileName = "stockchart.pdf";
#else
    QString fileName = "stockchart.png";
#endif

#ifndef QT_NO_FILEDIALOG
    const QList<QByteArray> imageFormats =
        QImageWriter::supportedImageFormats();

    QStringList filter;
    filter += "PDF Documents (*.pdf)";
#ifndef QWT_NO_SVG
    filter += "SVG Documents (*.svg)";
#endif
    filter += "Postscript Documents (*.ps)";

    if ( imageFormats.size() > 0 )
    {
        QString imageFilter( "Images (" );
        for ( int i = 0; i < imageFormats.size(); i++ )
        {
            if ( i > 0 )
                imageFilter += " ";
            imageFilter += "*.";
            imageFilter += imageFormats[i];
        }
        imageFilter += ")";

        filter += imageFilter;
    }

    fileName = QFileDialog::getSaveFileName(
        this, "Export File Name", fileName,
        filter.join( ";;" ), NULL, QFileDialog::DontConfirmOverwrite );
#endif
    if ( !fileName.isEmpty() )
    {
        QwtPlotRenderer renderer;
        renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, false );

        renderer.renderDocument( this, fileName, QSizeF( 300, 200 ), 85 );
    }
}
