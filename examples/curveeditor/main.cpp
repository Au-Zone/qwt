#include <qapplication.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include "plot.h"
#include "canvaspicker.h"
#include "scalepicker.h"

int main ( int argc, char **argv )
{
    QApplication a( argc, argv );

    QMainWindow mainWindow;

    Plot *plot = new Plot( &mainWindow );

#ifndef QT_NO_PRINTER
    QToolBar *toolBar = new QToolBar( &mainWindow );

    QToolButton *btnPrint = new QToolButton( toolBar );
    btnPrint->setText( "Print" );
    btnPrint->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    toolBar->addWidget( btnPrint );
    QObject::connect( btnPrint, SIGNAL( clicked() ),
        plot, SLOT( printPlot() ) );

    mainWindow.addToolBar( toolBar );
#endif

    // The canvas picker handles all mouse and key
    // events on the plot canvas

    ( void ) new CanvasPicker( plot );

    // The scale picker translates mouse clicks
    // int o clicked() signals

    ScalePicker *scalePicker = new ScalePicker( plot );
    a.connect( scalePicker, SIGNAL( clicked( int, double ) ),
        plot, SLOT( updateMarker( int, double ) ) );

    mainWindow.setCentralWidget( plot );
    mainWindow.resize( 540, 400 );
    mainWindow.show();

    return a.exec();
}
