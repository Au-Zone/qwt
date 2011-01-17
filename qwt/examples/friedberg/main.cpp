#include <qapplication.h>
#include <qcombobox.h>
#include <qlayout.h>
#include "plot.h"

static const QString styleSheet (
#if 0
    "QWidget#MainWindow"
    "{"
        "border-radius: 10px;"
        "background-color: qlineargradient( x1: 0, y1: 0, x2: 0, y2: 1, "
        "stop: 0 CornflowerBlue, stop: 0.5 Gainsboro, stop: 1 CornflowerBlue );"
    "}"
#endif
#if 1
    "QwtPlot"
    "{"
        "border: 1px solid white;"
        "border-radius: 10px;"
        "padding: 10px;"
        "background-color: qlineargradient( x1: 0, y1: 0, x2: 0, y2: 1, "
        "stop: 0 Brown, stop: 0.5 Chocolate, stop: 1 Brown );"
    "}"
#endif
    "QwtPlotCanvas"
    "{"
        "border: 1px solid White;"
        "border-radius: 10px;"
        "background-color: Tan;"
    "}"
    "QwtTextLabel#QwtPlotTitle"
    "{"
        "color: palette(light);"
    "}"
    "QwtLegendItem"
    "{"
        "color: palette(light);"
    "}"
    "QwtLegend"
    "{"
        "border: 1px solid white;"
        "border-radius: 10px;"
        "padding: 10px;"
        "background: brown;"
    "}"
    "QwtScaleWidget"
    "{"
        "color: palette(light);"
    "}"
);

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    a.setStyleSheet( styleSheet );

    QWidget w;
    w.setObjectName("MainWindow");
    
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
