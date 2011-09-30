#include "barchart.h"
#include <qwt_plot_renderer.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_barchart.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_draw.h>
#include <qfiledialog.h>
#include <qimagewriter.h>
#include <qprintdialog.h>
#include <qfileinfo.h>

BarChart::BarChart( QWidget *parent ):
    QwtPlot( parent )
{
    setTitle( "Bar Chart" );

    canvas()->setPalette( Qt::gray );

    setAxisTitle( QwtPlot::yLeft, "Whatever" );
    setAxisTitle( QwtPlot::xBottom, "Whatever" );

    d_barChartItem = new QwtPlotBarChart( "Bar Chart " );
#if 0
    d_barChartItem->setBaseline( 2 );
#endif
    d_barChartItem->attach( this );

    populate();

    setAutoReplot( true );
}

void BarChart::populate()
{
    QVector< QVector<double> > series;

    for ( int i = 0; i < 10; i++ )
    {
        double sign = 1.0;
#if 0
        if ( i % 3 == 0 )
            sign = -1.0;
#endif

        QVector<double> values;
        for ( int j = 0; j < 4; j++ )
            values += sign * ( qrand() % 10 );

        series += values;
#if 0
        qDebug() << i << values;
#endif
    }

    d_barChartItem->setSamples( series );
    setOrientation( 0 );
}


void BarChart::setMode( int mode )
{
    if ( mode == 0 )
    {
        d_barChartItem->setStyle( QwtPlotBarChart::Stacked );
        d_barChartItem->setBarWidth( 0.8 );
    }
    else
    {
        d_barChartItem->setStyle( QwtPlotBarChart::Grouped );
        d_barChartItem->setBarWidth( 0.15 );
    }
}

void BarChart::setOrientation( int orientation )
{
    const int margin = 30;

    QwtPlot::Axis axis1, axis2;

    if ( orientation == 0 )
    {
        axis1 = QwtPlot::xBottom;
        axis2 = QwtPlot::yLeft;

        plotLayout()->setCanvasMargin( margin, QwtPlot::yLeft );
        plotLayout()->setCanvasMargin( margin, QwtPlot::yRight );
        plotLayout()->setCanvasMargin( 0, QwtPlot::xBottom );
        plotLayout()->setCanvasMargin( 0, QwtPlot::xTop );

        d_barChartItem->setOrientation( Qt::Vertical );
    }
    else
    {
        axis1 = QwtPlot::yLeft;
        axis2 = QwtPlot::xBottom;

        plotLayout()->setCanvasMargin( 0, QwtPlot::yLeft );
        plotLayout()->setCanvasMargin( 0, QwtPlot::yRight );
        plotLayout()->setCanvasMargin( margin, QwtPlot::xBottom );
        plotLayout()->setCanvasMargin( margin, QwtPlot::xTop );

        d_barChartItem->setOrientation( Qt::Horizontal );
    }

    setAxisScale( axis1, 0, d_barChartItem->dataSize() - 1, 1.0 );
#if 1
    setAxisAutoScale( axis2 );
#else
    setAxisScale( axis2, -15, 15 );
#endif

    QwtScaleDraw *scaleDraw1 = axisScaleDraw( axis1 );
    scaleDraw1->setTickLength( QwtScaleDiv::MinorTick, 0 );
    scaleDraw1->setTickLength( QwtScaleDiv::MediumTick, 0 );
    scaleDraw1->setTickLength( QwtScaleDiv::MajorTick, 8 );

    QwtScaleDraw *scaleDraw2 = axisScaleDraw( axis2 );
    scaleDraw2->setTickLength( QwtScaleDiv::MinorTick, 4 );
    scaleDraw2->setTickLength( QwtScaleDiv::MediumTick, 6 );
    scaleDraw2->setTickLength( QwtScaleDiv::MajorTick, 8 );

    replot();
}

void BarChart::exportChart()
{
#ifndef QT_NO_PRINTER
    QString fileName = "barchart.pdf";
#else
    QString fileName = "barchart.png";
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
        renderer.renderDocument( this, fileName, QSizeF( 300, 200 ), 85 );
    }
}
