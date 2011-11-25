#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qwt_plot_renderer.h>
#include "plot.h"
#include "panel.h"
#include "mainwindow.h"

MainWindow::MainWindow( QWidget *parent ):
    QMainWindow( parent )
{
    Settings settings;
    settings.legend.isEnabled = false;
    settings.legend.position = QwtPlot::RightLegend;

    settings.legendItem.isEnabled = true;
    settings.legendItem.numColumns = 1;
    settings.legendItem.alignment = Qt::AlignRight | Qt::AlignVCenter;
    settings.legendItem.backgroundMode = 0;
    settings.numCurves = 4;
    
    d_plot = new Plot();
    d_panel = new Panel();
    d_panel->setSettings( settings );

    QWidget *box = new QWidget( this );
    QHBoxLayout *layout = new QHBoxLayout( box );
    layout->addWidget( d_plot, 10 );
    layout->addWidget( d_panel );

    setCentralWidget( box );

    QToolBar *toolBar = new QToolBar( this );

    QToolButton *btnExport = new QToolButton( toolBar );
    btnExport->setText( "Export" );
    toolBar->addWidget( btnExport );

    addToolBar( toolBar );

    updatePlot();

    connect( d_panel, SIGNAL( edited() ), SLOT( updatePlot() ) );
    connect( btnExport, SIGNAL( clicked() ), SLOT( exportPlot() ) );
}

void MainWindow::updatePlot()
{
    d_plot->applySettings( d_panel->settings() );
}

void MainWindow::exportPlot()
{
    QwtPlotRenderer renderer;

    // flags to make the document look like the widget
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, false );
    renderer.setLayoutFlag( QwtPlotRenderer::KeepFrames, true );

    renderer.exportTo( d_plot, "legends.pdf" );
}
