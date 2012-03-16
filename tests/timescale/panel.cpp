#include "panel.h"
#include "settings.h"
#include <qdatetimeedit.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qlabel.h>

Panel::Panel( QWidget *parent ):
    QWidget( parent )
{
    // create widgets

    d_startDateTime = new QDateTimeEdit();
    d_startDateTime->setCalendarPopup( true );

    d_endDateTime = new QDateTimeEdit();
    d_endDateTime->setCalendarPopup( true );
    
    d_maxMajorTicks = new QSpinBox();
    d_maxMajorTicks->setRange( 0, 50 );

    d_maxMinorTicks = new QSpinBox();
    d_maxMinorTicks->setRange( 0, 50 );

    d_maxWeeks = new QSpinBox();
    d_maxWeeks->setRange( -1, 100 );
    d_maxWeeks->setSpecialValueText( "Disabled" );

    // layout

    QGridLayout *layout = new QGridLayout( this );
    layout->setAlignment( Qt::AlignLeft | Qt::AlignTop );

    int row = 0;
    layout->addWidget( new QLabel( "From" ), row, 0 );
    layout->addWidget( d_startDateTime, row, 1 );

    row++;
    layout->addWidget( new QLabel( "To" ), row, 0 );
    layout->addWidget( d_endDateTime, row, 1 );

    row++;
    layout->addWidget( new QLabel( "Max Major" ), row, 0 );
    layout->addWidget( d_maxMajorTicks, row, 1 );

    row++;
    layout->addWidget( new QLabel( "Max Minor" ), row, 0 );
    layout->addWidget( d_maxMinorTicks, row, 1 );

    row++;
    layout->addWidget( new QLabel( "Max Weeks" ), row, 0 );
    layout->addWidget( d_maxWeeks, row, 1 );

    connect( d_startDateTime,
        SIGNAL( dateTimeChanged( const QDateTime & ) ), SIGNAL( edited() ) );
    connect( d_endDateTime,
        SIGNAL( dateTimeChanged( const QDateTime & ) ), SIGNAL( edited() ) );
    connect( d_maxMajorTicks,
        SIGNAL( valueChanged( int ) ), SIGNAL( edited() ) );
    connect( d_maxMinorTicks,
        SIGNAL( valueChanged( int ) ), SIGNAL( edited() ) );
    connect( d_maxWeeks,
        SIGNAL( valueChanged( int ) ), SIGNAL( edited() ) );
}

void Panel::setSettings( const Settings &settings )
{
    blockSignals( true );

    d_startDateTime->setDateTime( settings.startDateTime );
    d_endDateTime->setDateTime( settings.endDateTime );

    d_maxMajorTicks->setValue( settings.maxMajor );
    d_maxMinorTicks->setValue( settings.maxMinor );
    d_maxWeeks->setValue( settings.maxWeeks );
        
    blockSignals( false );
}

Settings Panel::settings() const
{
    Settings settings;

    settings.startDateTime = d_startDateTime->dateTime();
    settings.endDateTime = d_endDateTime->dateTime();
    settings.maxMajor = d_maxMajorTicks->value();
    settings.maxMinor = d_maxMinorTicks->value();
    settings.maxWeeks = d_maxWeeks->value();

    return settings;
}
