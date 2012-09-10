#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include "sliderbox.h"

class SliderDemo: public QWidget
{
public:
    SliderDemo( QWidget *parent = NULL ):
        QWidget( parent )
    {
        int i;

        QBoxLayout *hSliderLayout = createLayout( Qt::Vertical );
        for ( i = 0; i < 4; i++ )
            hSliderLayout->addWidget( new SliderBox( i ) );
        hSliderLayout->addStretch();

        QBoxLayout *vSliderLayout = createLayout( Qt::Horizontal );
        for ( ; i < 7; i++ )
            vSliderLayout->addWidget( new SliderBox( i ) );

        QLabel *vTitle = new QLabel( "Vertical Sliders", this );
        vTitle->setFont( QFont( "Helvetica", 14, QFont::Bold ) );
        vTitle->setAlignment( Qt::AlignHCenter );

        QBoxLayout *layout1 = createLayout( Qt::Vertical );
        layout1->addWidget( vTitle, 0 );
        layout1->addLayout( vSliderLayout, 10 );

        QLabel *hTitle = new QLabel( "Horizontal Sliders", this );
        hTitle->setFont( vTitle->font() );
        hTitle->setAlignment( Qt::AlignHCenter );

        QBoxLayout *layout2 = createLayout( Qt::Vertical );
        layout2->addWidget( hTitle, 0 );
        layout2->addLayout( hSliderLayout, 10 );

        QBoxLayout *mainLayout = createLayout( Qt::Horizontal, this );
        mainLayout->addLayout( layout1 );
        mainLayout->addLayout( layout2, 10 );
    }
private:
    QBoxLayout *createLayout( Qt::Orientation orientation,
        QWidget *widget = NULL )
    {
        QBoxLayout *layout = 
            new QBoxLayout( QBoxLayout::LeftToRight, widget );

        if ( orientation == Qt::Vertical )
            layout->setDirection( QBoxLayout::TopToBottom );

        layout->setSpacing( 20 );
        layout->setMargin( 0 );

        return layout;
    }
};

int main ( int argc, char **argv )
{
    QApplication a( argc, argv );
    //a.setFont( QFont( "Helvetica", 10 ) );

    SliderDemo w;
    w.show();

    return a.exec();
}
