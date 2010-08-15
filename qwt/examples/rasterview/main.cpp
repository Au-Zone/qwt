#include <qapplication.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qwindowsstyle.h>
#include "plot.h"

class MainWindow: public QMainWindow
{
public:
    MainWindow(QWidget * = NULL);
};

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent)
{
    Plot *plot = new Plot(this);
    setCentralWidget(plot);

    QToolBar *toolBar = new QToolBar(this);

    QComboBox *comboBox = new QComboBox(toolBar);
    comboBox->setStyle(new QWindowsStyle() );
    comboBox->addItem("Nearest Neighbour");
    comboBox->addItem("Bilinear Interpolation");
    connect(comboBox, SIGNAL(activated(int)), 
        plot, SLOT(setResampleMode(int)));

    toolBar->addWidget( new QLabel("Resampling: ") );
    toolBar->addWidget(comboBox);

    addToolBar(toolBar);
}

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    MainWindow mainWindow;
    mainWindow.resize(600,400);
    mainWindow.show();

    return a.exec(); 
}
