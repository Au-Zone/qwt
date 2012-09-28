#include <qlabel.h>
#include <qlayout.h>
#include <qwt_wheel.h>
#include <qwt_thermo.h>
#include <qwt_scale_engine.h>
#include <qwt_transform.h>
#include <qwt_color_map.h>
#include "wheelbox.h"

WheelBox::WheelBox( Qt::Orientation orientation,
        int type, QWidget *parent ):
    QWidget( parent )
{
    QWidget *box = createBox( orientation, type );
    d_label = new QLabel( this );
    d_label->setAlignment( Qt::AlignCenter );

    QBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( box );
    layout->addWidget( d_label );

    setNum( d_wheel->value() );

    connect( d_wheel, SIGNAL( valueChanged( double ) ), 
        this, SLOT( setNum( double ) ) );
}

QWidget *WheelBox::createBox( 
    Qt::Orientation orientation, int type ) 
{
    d_wheel = new QwtWheel();
    d_wheel->setOrientation( orientation );
    d_wheel->setValue( 80 );
    d_wheel->setMass( 1.0 );

    d_thermo = new QwtThermo();
    if ( orientation == Qt::Horizontal )
        d_thermo->setOrientation( orientation, QwtThermo::TopScale );
    else
        d_thermo->setOrientation( orientation, QwtThermo::LeftScale );

    switch( type )
    {
        case 0:
        {
			QwtLinearColorMap *colorMap = new QwtLinearColorMap(); 
			colorMap->setColorInterval( Qt::blue, Qt::red );
			d_thermo->setColorMap( colorMap );

            break;
        }
        case 1:
        {
			QwtLinearColorMap *colorMap = new QwtLinearColorMap();
			colorMap->setMode( QwtLinearColorMap::FixedColors );

			int idx = 4;

			colorMap->setColorInterval( Qt::GlobalColor( idx ),
				Qt::GlobalColor( idx + 10 ) );
			for ( int i = 1; i < 10; i++ )
			{
				colorMap->addColorStop( i / 10.0, 
					Qt::GlobalColor( idx + i ) );
			}

			d_thermo->setColorMap( colorMap );
            break;
        }
        case 2:
        {
			d_wheel->setRange( 10, 1000 );
			d_wheel->setSingleStep( 1.0 );

			d_thermo->setScaleEngine( new QwtLogScaleEngine );
			d_thermo->setScaleMaxMinor( 10 );

			d_thermo->setFillBrush( Qt::darkCyan );
			d_thermo->setAlarmBrush( Qt::magenta );
			d_thermo->setAlarmLevel( 500.0 );

    		d_wheel->setValue( 800 );

            break;
        }
        case 3:
        {
            break;
        }
    }

    d_thermo->setScale( d_wheel->minimum(), d_wheel->maximum() );
    d_thermo->setValue( d_wheel->value() );

    connect( d_wheel, SIGNAL( valueChanged( double ) ), 
        d_thermo, SLOT( setValue( double ) ) );

    QWidget *box = new QWidget();

    QBoxLayout *layout;

    if ( orientation == Qt::Horizontal )
        layout = new QVBoxLayout( box );
    else
        layout = new QHBoxLayout( box );

    layout->addStretch( 10 );
    layout->addWidget( d_thermo );
    layout->addWidget( d_wheel );
    layout->addStretch( 10 );

    return box;
}

void WheelBox::setNum( double v )
{
    QString text;
    text.setNum( v, 'f', 2 );

    d_label->setText( text );
}
