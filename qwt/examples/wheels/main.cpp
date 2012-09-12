#include <qapplication.h>
#include <qlayout.h>
#include "wheelbox.h"

class MainWindow: public QWidget
{
public:
    MainWindow()
    {
        const int numBoxes = 4;
    
        QGridLayout *layout1 = new QGridLayout();
        for ( int i = 0; i < numBoxes; i++ )
        {   
            WheelBox *box = new WheelBox( Qt::Vertical, i );
            layout1->addWidget( box, i / 2, i % 2 );
        }
    
        QGridLayout *layout2 = new QGridLayout();
        for ( int i = 0; i < numBoxes; i++ )
        {   
            WheelBox *box = new WheelBox( Qt::Horizontal, i + numBoxes );
            layout2->addWidget( box, i / 2, i % 2 );
        }
        layout2->setRowStretch( layout2->rowCount(), 10 );
     
        QHBoxLayout *layout = new QHBoxLayout( this );
        layout->addLayout( layout1 );
        layout->addLayout( layout2, 10 );
    }
};

int main ( int argc, char **argv )
{
    QApplication a( argc, argv );
    a.setPalette( QColor( "Silver" ) );

    MainWindow w;
    w.show();

    return a.exec();
}
