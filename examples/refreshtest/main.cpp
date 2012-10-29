#include "mainwindow.h"
#include <qapplication.h>
#ifndef QWT_NO_OPENGL
#include <qgl.h>
#endif

int main( int argc, char **argv )
{
#ifndef QWT_NO_OPENGL
    // on my box QPaintEngine::OpenGL2 has serious problems, f.e:
    // the lines of a simple drawRect are wrong.

    QGL::setPreferredPaintEngine( QPaintEngine::OpenGL );
#endif

    QApplication a( argc, argv );

    MainWindow mainWindow;
    mainWindow.resize( 600, 400 );
    mainWindow.show();

    return a.exec();
}
