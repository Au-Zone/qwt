#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include "plot.h"
#include "canvaspicker.h"
#include "scalepicker.h"

class ToolButton: public QToolButton
{
public:
    ToolButton( const char *text, QToolBar *toolBar ):
        QToolButton( toolBar )
    {
        setText( text );
        setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    }
};

int main ( int argc, char **argv )
{
    QApplication a( argc, argv );

    QMainWindow mainWindow;

    Plot *plot = new Plot( &mainWindow );

    QToolBar *toolBar = new QToolBar( &mainWindow );

#ifndef QT_NO_PRINTER
    ToolButton *btnPrint = new ToolButton( "Print", toolBar );
    toolBar->addWidget( btnPrint );
    QObject::connect( btnPrint, SIGNAL( clicked() ),
        plot, SLOT( printPlot() ) );
#endif

    ToolButton *btnOverlay = new ToolButton( "Overlay", toolBar );
    btnOverlay->setCheckable( true );
    toolBar->addWidget( btnOverlay );
    QObject::connect( btnOverlay, SIGNAL( toggled( bool ) ),
        plot, SLOT( setOverlaying( bool ) ) );

    QComboBox *parameterBox = new QComboBox( toolBar );
    parameterBox->addItem( "None" );
    parameterBox->addItem( "Uniform" );
    parameterBox->addItem( "Centripetral" );
    parameterBox->addItem( "Chordal" );
    parameterBox->addItem( "Manhattan" );
    toolBar->addWidget( parameterBox );
    QObject::connect( parameterBox, SIGNAL( activated( const QString & ) ),
         plot, SLOT( setParametric( const QString & ) ) );

    mainWindow.addToolBar( toolBar );

    // The canvas picker handles all mouse and key
    // events on the plot canvas

    ( void ) new CanvasPicker( plot );

    // The scale picker translates mouse clicks
    // int o clicked() signals

    ScalePicker *scalePicker = new ScalePicker( plot );
    a.connect( scalePicker, SIGNAL( clicked( int, double ) ),
        plot, SLOT( updateMarker( int, double ) ) );

    mainWindow.setCentralWidget( plot );

    const QSize sz = 0.6 * QApplication::desktop()->size();
    mainWindow.resize( sz.boundedTo( QSize( 800, 600 ) ) );
    mainWindow.show();

    return a.exec();
}
