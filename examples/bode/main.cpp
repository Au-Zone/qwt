#include <qapplication.h>
#include "mainwindow.h"

int main (int argc, char **argv)
{
    QApplication a(argc, argv);

    MainWindow w;
#if QT_VERSION < 0x040000
    a.setMainWidget(&w);
#endif
    w.resize(540,400);
    w.show();

    int rv = a.exec();
    return rv;
}
