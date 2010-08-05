#include <qapplication.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include "plot.h"

class MainWindow: public QMainWindow
{
public:
    MainWindow(QWidget * = NULL);

private:
    Plot *d_plot;
};

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent)
{
    d_plot = new Plot(this);

    setCentralWidget(d_plot);

    QToolBar *toolBar = new QToolBar(this);

    QToolButton *btnSpectrogram = new QToolButton(toolBar);
    QToolButton *btnContour = new QToolButton(toolBar);
    QToolButton *btnPrint = new QToolButton(toolBar);

    btnSpectrogram->setText("Spectrogram");
    btnSpectrogram->setCheckable(true);
    btnSpectrogram->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnSpectrogram);

    btnContour->setText("Contour");
    btnContour->setCheckable(true);
    btnContour->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnContour);

    btnPrint->setText("Print");
    btnPrint->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnPrint);

    addToolBar(toolBar);

    connect(btnSpectrogram, SIGNAL(toggled(bool)), 
        d_plot, SLOT(showSpectrogram(bool)));
    connect(btnContour, SIGNAL(toggled(bool)), 
        d_plot, SLOT(showContour(bool)));
    connect(btnPrint, SIGNAL(clicked()), 
        d_plot, SLOT(printPlot()) );

    btnSpectrogram->setChecked(true);
    btnContour->setChecked(false);
}

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    MainWindow mainWindow;
    mainWindow.resize(600,400);
    mainWindow.show();

    return a.exec(); 
}
