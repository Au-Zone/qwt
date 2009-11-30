#include <qapplication.h>
#include "tvplot.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    TVPlot plot;
    
#if QT_VERSION < 0x040000
    a.setMainWidget(&plot);
#endif

    plot.resize(600,400);
    plot.show();

    return a.exec(); 
}
