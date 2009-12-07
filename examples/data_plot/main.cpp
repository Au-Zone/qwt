#include <qapplication.h>
#include <qmainwindow.h>
#include <qspinbox.h>
#include <qwt_plot_canvas.h>
#include <qtoolbar.h>
#include <qstatusbar.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qdatetime.h>
#include "data_plot.h"

class MainWindow: public QMainWindow
{
public:
    MainWindow(QWidget *parent = NULL);
    virtual bool eventFilter(QObject *, QEvent *);

private:
    DataPlot *d_plot;
    QLabel *d_frameCount;
};

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent)
{
    QToolBar *toolBar = new QToolBar(this);

#if QT_VERSION < 0x040000
    setDockEnabled(TornOff, true);
    setRightJustification(true);
#else
    toolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
#endif
    QWidget *hBox = new QWidget(toolBar);
    QLabel *label = new QLabel("Timer Interval", hBox);

    QSpinBox *timerInterval = new QSpinBox(hBox);
    timerInterval->setRange(0.0, 100.0);
    timerInterval->setSingleStep(1);

    QHBoxLayout *layout = new QHBoxLayout(hBox);
    layout->addWidget(label);
    layout->addWidget(timerInterval);
    layout->addWidget(new QWidget(hBox), 10); // spacer);

#if QT_VERSION >= 0x040000
    toolBar->addWidget(hBox);
#endif
    addToolBar(toolBar);

    d_plot = new DataPlot(this);
    setCentralWidget(d_plot);
    d_plot->canvas()->installEventFilter(this);

    d_frameCount = new QLabel();
    statusBar()->addWidget(d_frameCount, 10);

    connect(timerInterval, SIGNAL(valueChanged(int)),
        d_plot, SLOT(setTimerInterval(int)) );

    timerInterval->setValue(20);
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if ( object == d_plot->canvas() && event->type() == QEvent::Paint )
    {
        static int counter;
        static QTime timeStamp;

        if ( !timeStamp.isValid() )
        {
            timeStamp.start();
            counter = 0;
        }
        else
        {
            counter++;

            const double elapsed = timeStamp.elapsed() / 1000.0;
            if ( elapsed >= 1 )
            {
                QString fps;
                fps.setNum(qRound(counter / elapsed));
                fps += " Fps";

                d_frameCount->setText(fps);

                counter = 0;
                timeStamp.start();
            }
        }
    }

    return QMainWindow::eventFilter(object, event);
};

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    MainWindow mainWindow;
#if QT_VERSION < 0x040000
    a.setMainWidget(&mainWindow);
#endif

    mainWindow.resize(600,400);
    mainWindow.show();

    return a.exec(); 
}
