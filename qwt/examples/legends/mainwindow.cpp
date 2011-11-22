#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qdebug.h>
#include "plot.h"
#include "mainwindow.h"

MainWindow::MainWindow( QWidget *parent ):
    QMainWindow( parent )
{
    Plot *plot = new Plot( this );
    setCentralWidget( plot );

    QToolBar *toolBar = new QToolBar( this );

    QToolButton *btnLegend = new QToolButton( toolBar );
    btnLegend->setText( "Legend" );
    toolBar->addWidget( btnLegend );

    QToolButton *btnCurve = new QToolButton( toolBar );
    btnCurve->setText( "Curve" );
    toolBar->addWidget( btnCurve );

    addToolBar( toolBar );

    connect( btnLegend, SIGNAL( clicked() ), plot, SLOT( insertLegend() ) );
    //connect( btnLegend, SIGNAL( clicked() ), this, SLOT( debugChain() ) );
    connect( btnCurve, SIGNAL( clicked() ), plot, SLOT( insertCurve() ) );
}

void MainWindow::debugChain()
{
    int i = 0;
    for ( QWidget *w = nextInFocusChain();
        w != this; w = w->nextInFocusChain() )
    {
        qDebug() << i++ << ": " << w;
    }
}
