#include <qlabel.h>
#include <qlayout.h>
#include <qwt_wheel.h>
#include <qwt_thermo.h>
#include <qwt_scale_engine.h>
#include <qwt_transform.h>
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

    d_thermo = new QwtThermo();
    if ( orientation == Qt::Horizontal )
        d_thermo->setOrientation( orientation, QwtThermo::TopScale );
    else
        d_thermo->setOrientation( orientation, QwtThermo::LeftScale );

    switch( type )
    {
        case 0:
        {
            break;
        }
        case 1:
        {
            break;
        }
        case 2:
        {
            break;
        }
        case 3:
        {
            break;
        }
    }

    d_thermo->setRange( d_wheel->minimum(), d_wheel->maximum() );
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
