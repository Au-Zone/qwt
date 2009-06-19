#include "wheelbox.h"
#include <qwt_wheel.h>
#include <qlcdnumber.h>
#include <qlabel.h>
#include <qlayout.h>

WheelBox::WheelBox(const QString &title,
    	double min, double max, double stepSize, QWidget *parent):
	QWidget(parent)
{
    d_number = new QLCDNumber(this);
    d_number->setSegmentStyle(QLCDNumber::Filled);

    d_wheel = new QwtWheel(this);
    d_wheel->setOrientation(Qt::Vertical);
    d_wheel->setRange(min, max, stepSize);

    QFont font("Helvetica", 10);
	font.setBold(true);

    d_label = new QLabel(title, this);
	d_label->setFont(font);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(d_number, 10);
    hLayout->addWidget(d_wheel);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addLayout(hLayout, 10);
    vLayout->addWidget(d_label, 0, Qt::AlignTop | Qt::AlignHCenter);

    connect(d_wheel, SIGNAL(valueChanged(double)), 
        d_number, SLOT(display(double)));
    connect(d_wheel, SIGNAL(valueChanged(double)), 
        this, SIGNAL(valueChanged(double)));
}

void WheelBox::setValue(double value)
{
    d_wheel->setValue(value);
    d_number->display(value);
}

double WheelBox::value() const
{
    return d_wheel->value();
}
