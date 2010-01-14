#include <qapplication.h>
#include <qcombobox.h>
#include <qlayout.h>
#include "plot.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

	QWidget w;
	
	QComboBox *typeBox = new QComboBox(&w);
#if QT_VERSION < 0x040000
	typeBox->insertItem("Bars");
	typeBox->insertItem("Tube");
	typeBox->setCurrentItem(1);
#else
	typeBox->addItem("Bars");
	typeBox->addItem("Tube");
	typeBox->setCurrentIndex(1);
#endif

	typeBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    Plot *plot = new Plot(&w);
#if QT_VERSION < 0x040000
	plot->setMode(typeBox->currentItem());
#else
	plot->setMode(typeBox->currentIndex());
#endif

	QVBoxLayout *layout = new QVBoxLayout(&w);
	layout->addWidget(typeBox);
	layout->addWidget(plot);

#if QT_VERSION < 0x040000
    a.setMainWidget(&w);
#endif
    w.resize(600,400);
    w.show();

#if QT_VERSION < 0x040000
	QObject::connect(typeBox, SIGNAL(activated(int)),
		plot, SLOT(setMode(int)));
#else
	QObject::connect(typeBox, SIGNAL(currentIndexChanged(int)),
		plot, SLOT(setMode(int)));
#endif

    return a.exec(); 
}
