#include <qapplication.h>
#include <qwidget.h>
#include <qlayout.h>
#include "knobbox.h"

int main ( int argc, char **argv )
{
    QApplication a( argc, argv );
    a.setPalette( Qt::darkGray );

	QWidget w;
    QGridLayout *layout = new QGridLayout( &w );

    const int numRows = 3;
    for ( int i = 0; i < 2 * numRows; i++ )
    {
        KnobBox *knobBox = new KnobBox( &w, i );
        layout->addWidget( knobBox, i / numRows, i % numRows );
    }

    w.show();

    return a.exec();
}
