#include <qapplication.h>
#include "mainwindow.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    a.setFont( QFont( "Helvetica", 12 ) );

    MainWindow window;
    window.resize( 800, 600 );
    window.show();

    return a.exec();
}
