#include <qapplication.h>
#include "plot.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    Plot plot;
#if QT_VERSION < 0x040000
    a.setMainWidget(&plot);
#endif
    plot.resize(600,400);
    plot.show();
    return a.exec(); 
}
