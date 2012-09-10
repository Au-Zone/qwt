#include <qlabel.h>
#include <qlayout.h>
#include <qwt_knob.h>
#include <qwt_scale_engine.h>
#include <qwt_transform.h>
#include "knobbox.h"

KnobBox::KnobBox( QWidget *parent, int knobType ):
    QWidget( parent )
{
    d_knob = createKnob( knobType );
#if 1
	d_knob->setKnobWidth( 100 );
#endif

    d_label = new QLabel( this );
	d_label->setAlignment( Qt::AlignCenter );

    QVBoxLayout *layout = new QVBoxLayout( this );;
    layout->addWidget( d_knob, 10 );
    layout->addWidget( d_label );

    connect( d_knob, SIGNAL( valueChanged( double ) ), 
		this, SLOT( setNum( double ) ) );

	setNum( d_knob->value() );
}

QwtKnob *KnobBox::createKnob( int knobType ) const
{
    QwtKnob *knob = new QwtKnob();
   	knob->setTracking( true );

    switch( knobType )
    {
        case 0:
        {
			knob->setKnobStyle( QwtKnob::Sunken );
    		knob->setMarkerStyle( QwtKnob::Nub );
    		knob->setWrapping( true );
    		knob->setNumTurns( 2 );
    		knob->setScale( 0, 100, 5.0 );
            break;
        }
        case 1:
		{
			knob->setKnobStyle( QwtKnob::Sunken );
    		knob->setMarkerStyle( QwtKnob::Dot );
			break;
		}
        case 2:
		{
			knob->setKnobStyle( QwtKnob::Sunken );
    		knob->setMarkerStyle( QwtKnob::Tick );
			break;
		}
        case 3:
		{
			knob->setKnobStyle( QwtKnob::Raised );
    		knob->setMarkerStyle( QwtKnob::Notch );
    		knob->setScaleEngine( new QwtLogScaleEngine() );
    		knob->setScale( 0.1, 1000.0, 1.0 );
    		knob->setScaleMaxMinor( 10 );
			break;
		}
        case 4:
		{
			knob->setKnobStyle( QwtKnob::Raised );
    		knob->setMarkerStyle( QwtKnob::Dot );
			break;
		}
        case 5:
		{
			knob->setKnobStyle( QwtKnob::Raised );
    		knob->setMarkerStyle( QwtKnob::Tick );
			break;
		}
    }

    return knob;
}

void KnobBox::setNum( double v )
{
    QString text;
    text.setNum( v, 'f', 2 );

    d_label->setText( text );
}
