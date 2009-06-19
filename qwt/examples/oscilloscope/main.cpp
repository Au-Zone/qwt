#include <qapplication.h>
#include "mainwindow.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    MainWindow window;
#if QT_VERSION < 0x040000
    a.setMainWidget(&window);
#endif

    window.resize(800,400);
    window.show();

    return a.exec(); 
}
