#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwt_slider.h>
#include <qwt_scale_engine.h>
#include "sliders.h"

class Layout: public QBoxLayout
{
public:
    Layout( Qt::Orientation o, QWidget *parent = NULL ):
        QBoxLayout( QBoxLayout::LeftToRight, parent )
    {
        if ( o == Qt::Vertical )
            setDirection( QBoxLayout::TopToBottom );

        setSpacing( 20 );
        setMargin( 0 );
    }
};

Slider::Slider( QWidget *parent, int sliderType ):
    QWidget( parent )
{
    d_slider = createSlider( this, sliderType );

    QFlags<Qt::AlignmentFlag> alignment;

	if ( d_slider->orientation() == Qt::Horizontal )
	{
		if ( d_slider->scalePosition() == QwtSlider::TrailingScale )
			alignment = Qt::AlignBottom;
		else
			alignment = Qt::AlignTop;

		alignment |= Qt::AlignHCenter;
	}
	else
	{
		if ( d_slider->scalePosition() == QwtSlider::TrailingScale )
			alignment = Qt::AlignRight;
		else
			alignment = Qt::AlignLeft;

		alignment |= Qt::AlignVCenter;
	}

    d_label = new QLabel( "0", this );
    d_label->setAlignment( alignment );
    d_label->setFixedWidth( d_label->fontMetrics().width( "10000.9" ) );

    connect( d_slider, SIGNAL( scaleValueChanged( double ) ), SLOT( setNum( double ) ) );

    QBoxLayout *layout;
    if ( d_slider->orientation() == Qt::Horizontal )
        layout = new QHBoxLayout( this );
    else
        layout = new QVBoxLayout( this );

    layout->addWidget( d_slider );
    layout->addWidget( d_label );
}

QwtSlider *Slider::createSlider( QWidget *parent, int sliderType ) const
{
    QwtSlider *slider = NULL;

    switch( sliderType )
    {
        case 0:
        {
            slider = new QwtSlider( parent );
			slider->setOrientation( Qt::Horizontal );
			slider->setScalePosition( QwtSlider::TrailingScale );
			slider->setBackgroundStyle( QwtSlider::Trough );
            slider->setHandleSize( QSize( 30, 16 ) );
            slider->setRange( 10.0, -10.0 ); 
			slider->setSingleStep( 1.0 ); 
			slider->setPageStepCount( 0 ); // paging disabled
            break;
        }
        case 1:
        {
            slider = new QwtSlider( parent );
			slider->setOrientation( Qt::Horizontal );
			slider->setScalePosition( QwtSlider::NoScale );
			slider->setBackgroundStyle( QwtSlider::Trough | QwtSlider::Groove );
            slider->setRange( 0.0, 1.0 );
			slider->setSingleStep( 0.01 );
			slider->setPageStepCount( 5 );
            break;
        }
        case 2:
        {
            slider = new QwtSlider( parent );
			slider->setOrientation( Qt::Horizontal );
			slider->setScalePosition( QwtSlider::LeadingScale );
			slider->setBackgroundStyle( QwtSlider::Groove );
            slider->setHandleSize( QSize( 12, 25 ) );
            slider->setRange( 1000.0, 3000.0 );
			slider->setSingleStep( 10.0 );
			slider->setPageStepCount( 10 );
            break;
        }
        case 3:
        {
            slider = new QwtSlider( parent );
			slider->setOrientation( Qt::Vertical );
			slider->setScalePosition( QwtSlider::TrailingScale );
			slider->setBackgroundStyle( QwtSlider::Groove );
            slider->setRange( 0.0, 100.0 );
            slider->setSingleStep( 1.0 );
			slider->setPageStepCount( 5 );
            slider->setScaleMaxMinor( 5 );
            break;
        }
        case 4:
        {
            slider = new QwtSlider( parent );
			slider->setOrientation( Qt::Vertical );
			slider->setScalePosition( QwtSlider::NoScale );
			slider->setBackgroundStyle( QwtSlider::Trough );
            slider->setRange( 0.0, 100.0 );
			slider->setSingleStep( 1.0 );
			slider->setPageStepCount( 10 );
            break;
        }
        case 5:
        {
            slider = new QwtSlider( parent );
			slider->setOrientation( Qt::Vertical );
			slider->setScalePosition( QwtSlider::LeadingScale );
			slider->setBackgroundStyle( QwtSlider::Trough | QwtSlider::Groove );
            slider->setScaleEngine( new QwtLogScaleEngine );
            slider->setHandleSize( QSize( 20, 32 ) );
            slider->setBorderWidth( 1 );
            slider->setRange( 0.0, 4.0 );
			slider->setSingleStep( 0.01 );
            slider->setScale( 1.0, 1.0e4 );
            slider->setScaleMaxMinor( 9 );
            break;
        }
    }

    if ( slider )
    {
        QString name( "Slider %1" );
        slider->setObjectName( name.arg( sliderType ) );
    }

    return slider;
}

void Slider::setNum( double v )
{
    QString text;
    text.setNum( v, 'f', 1 );

    d_label->setText( text );
}

SliderDemo::SliderDemo( QWidget *p ):
    QWidget( p )
{
    int i;

    Layout *hSliderLayout = new Layout( Qt::Vertical );
    for ( i = 0; i < 3; i++ )
        hSliderLayout->addWidget( new Slider( this, i ) );
    hSliderLayout->addStretch();

    Layout *vSliderLayout = new Layout( Qt::Horizontal );
    for ( ; i < 6; i++ )
        vSliderLayout->addWidget( new Slider( this, i ) );

    QLabel *vTitle = new QLabel( "Vertical Sliders", this );
    vTitle->setFont( QFont( "Helvetica", 14, QFont::Bold ) );
    vTitle->setAlignment( Qt::AlignHCenter );

    Layout *layout1 = new Layout( Qt::Vertical );
    layout1->addWidget( vTitle, 0 );
    layout1->addLayout( vSliderLayout, 10 );

    QLabel *hTitle = new QLabel( "Horizontal Sliders", this );
    hTitle->setFont( vTitle->font() );
    hTitle->setAlignment( Qt::AlignHCenter );

    Layout *layout2 = new Layout( Qt::Vertical );
    layout2->addWidget( hTitle, 0 );
    layout2->addLayout( hSliderLayout, 10 );

    Layout *mainLayout = new Layout( Qt::Horizontal, this );
    mainLayout->addLayout( layout1 );
    mainLayout->addLayout( layout2, 10 );
}

int main ( int argc, char **argv )
{
    QApplication a( argc, argv );

    QApplication::setFont( QFont( "Helvetica", 10 ) );

    SliderDemo w;
    w.show();

    return a.exec();
}
