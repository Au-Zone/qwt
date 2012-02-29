#include <qapplication.h>
#include <qdebug.h>
#include <qdatetime.h>
#include "plot.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    Plot plot;
    plot.resize( 800, 600 );
    plot.show();

    return a.exec();
}
