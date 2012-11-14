#include "mainwindow.h"
#include <qapplication.h>

#ifndef QWT_NO_OPENGL
#if QT_VERSION > 0x040600 && QT_VERSION < 0x050000
#include <qgl.h>
#endif
#endif

int main( int argc, char **argv )
{
#ifndef QWT_NO_OPENGL
#if QT_VERSION > 0x040600 && QT_VERSION < 0x050000
    // on my box QPaintEngine::OpenGL2 has serious problems, f.e:
    // the lines of a simple drawRect are wrong.

    QGL::setPreferredPaintEngine( QPaintEngine::OpenGL );
#endif
#endif

    QApplication a( argc, argv );

    MainWindow mainWindow;
    mainWindow.resize( 600, 400 );
    mainWindow.show();

    return a.exec();
}
