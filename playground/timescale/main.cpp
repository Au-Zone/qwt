#include <qapplication.h>
#include "mainwindow.h"
#include <QDateTime>
#include <QDebug>
#include "qwt_date.h"

QString toString( QDateTime& dt )
{
    return dt.toString( Qt::SystemLocaleLongDate );
}

void test()
{
    static const QwtDate::JulianDay d1 =
        QDateTime( QDate(1970, 1, 1) ).toUTC().date().toJulianDay();
    static const QwtDate::JulianDay d2 = 
        QDateTime::fromMSecsSinceEpoch( 0 ).date().toJulianDay();

    qDebug() << d1 << d2;
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    a.setFont( QFont( "Helvetica", 12 ) );

#if 0
test();
exit( 0 );
#endif
    MainWindow window;
    window.resize( 800, 600 );
    window.show();

    return a.exec();
}
