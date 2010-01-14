#include <qapplication.h>
#include <qcombobox.h>
#include <qlayout.h>
#include "plot.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

	QWidget w;
	
	QComboBox *typeBox = new QComboBox(&w);
	typeBox->addItem("Bars");
	typeBox->addItem("Tube");
	typeBox->setCurrentIndex(1);

	typeBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    Plot *plot = new Plot(&w);
	plot->setMode(typeBox->currentIndex());

	QVBoxLayout *layout = new QVBoxLayout(&w);
	layout->addWidget(typeBox);
	layout->addWidget(plot);

    w.resize(600,400);
    w.show();

	QObject::connect(typeBox, SIGNAL(currentIndexChanged(int)),
		plot, SLOT(setMode(int)));

    return a.exec(); 
}
