#include <qapplication.h>
#include <qlayout.h>
#include "wheelbox.h"

class MainWindow: public QWidget
{
public:
    MainWindow()
    {
        const QColor c( "Wheat" );

        const int numBoxes = 4;
    
        QGridLayout *layout1 = new QGridLayout();
        for ( int i = 0; i < numBoxes; i++ )
        {   
            WheelBox *box = new WheelBox( Qt::Vertical, i );
            box->setPalette( QPalette( c ) );
            layout1->addWidget( box, i / 2, i % 2 );
        }
    
        QGridLayout *layout2 = new QGridLayout();
        for ( int i = 0; i < numBoxes; i++ )
        {   
            WheelBox *box = new WheelBox( Qt::Horizontal, i + numBoxes );
            box->setPalette( QPalette( c ) );
            layout2->addWidget( box, i / 2, i % 2 );
        }
        layout2->setRowStretch( layout2->rowCount(), 10 );
     
        QHBoxLayout *layout = new QHBoxLayout( this );
        layout->addLayout( layout1 );
        layout->addLayout( layout2, 10 );
    }
protected:
    virtual void resizeEvent( QResizeEvent * )
    {
        // Qt 4.7.1: QGradient::StretchToDeviceMode is buggy on X11

        QPalette pal = palette();
        const QColor buttonColor = pal.color( QPalette::Button );

        QLinearGradient gradient( rect().topLeft(), rect().bottomRight() );
        gradient.setColorAt( 0.0, buttonColor.lighter( 120 ) );
        gradient.setColorAt( 0.8, buttonColor.darker( 120 ) );

        pal.setBrush( QPalette::Window, gradient );

        setPalette( pal );
    }
};

int main ( int argc, char **argv )
{
    QApplication a( argc, argv );
    a.setPalette( QColor( "BurlyWood" ) );

    MainWindow w;
    w.show();

    return a.exec();
}
