#include <cstdlib>
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qstatusbar.h>
#include <qwt_plot.h>
#include <qwt_plot_rescaler.h>
#include "plot.h"
#include "mainwindow.h"

MainWindow::MainWindow()
{
    QFrame *w = new QFrame(this);

    QWidget *panel = createPanel(w);
    QWidget *plot = createPlot(w);

    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setMargin(0);
    layout->addWidget(panel, 0);
    layout->addWidget(plot, 10);

    setCentralWidget(w);

    setRescaleMode(0);
    setMouseMode(0);

    (void)statusBar();
}

QWidget *MainWindow::createPanel(QWidget *parent)
{
    QGroupBox *panel = new QGroupBox("Panel", parent);

    QComboBox *navigationBox = new QComboBox(panel);
    navigationBox->setEditable(false);
    navigationBox->insertItem(Tracking, "Tracking");
    navigationBox->insertItem(Zooming, "Zooming");
    navigationBox->insertItem(Panning, "Panning");
    connect(navigationBox, SIGNAL(activated(int)), SLOT(setMouseMode(int)));

    QComboBox *rescaleBox = new QComboBox(panel);
    rescaleBox->setEditable(false);
    rescaleBox->insertItem(KeepScales, "None");
    rescaleBox->insertItem(Fixed, "Fixed");
    rescaleBox->insertItem(Expanding, "Expanding");
    rescaleBox->insertItem(Fitting, "Fitting");
    connect(rescaleBox, SIGNAL(activated(int)), SLOT(setRescaleMode(int)));

    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->addWidget(navigationBox);
    layout->addWidget(rescaleBox);
    layout->addStretch(10);

    return panel;
}

QWidget *MainWindow::createPlot(QWidget *parent)
{
    Plot *plot = new Plot(parent);
    plot->replot();

    d_rescaler = new QwtPlotRescaler(plot->canvas());
    d_rescaler->setReferenceAxis(QwtPlot::xBottom);
    d_rescaler->setAspectRatio(QwtPlot::yLeft, 1.0);
    d_rescaler->setAspectRatio(QwtPlot::yRight, 0.0);
    d_rescaler->setAspectRatio(QwtPlot::xTop, 0.0);

    connect(plot, SIGNAL(resized(double, double)), 
        SLOT(showRatio(double, double)));
    return plot;
}

void MainWindow::setMouseMode(int)
{
}

void MainWindow::setRescaleMode(int mode)
{
    bool doEnable = true;

    switch(mode)
    {
        case KeepScales:
        {
            doEnable = false;
            break;
        }
        case Fixed:
        {
            d_rescaler->setRescalePolicy(QwtPlotRescaler::Fixed);
            break;
        }
        case Expanding:
        {
            d_rescaler->setRescalePolicy(QwtPlotRescaler::Expanding);
            break;
        }
        case Fitting:
        {
            d_rescaler->setRescalePolicy(QwtPlotRescaler::Fitting);
            break;
        }
    }

    d_rescaler->setEnabled(doEnable);
}

void MainWindow::showRatio(double xRatio, double yRatio)
{
    statusBar()->showMessage(QString("%1, %2").arg(xRatio).arg(yRatio));
}

