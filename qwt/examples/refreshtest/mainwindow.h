#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <qmainwindow.h>

class Plot;
class QSpinBox;
class QLabel;

class MainWindow: public QMainWindow
{
public:
    MainWindow(QWidget *parent = NULL);
    virtual bool eventFilter(QObject *, QEvent *);

private:
    void initToolBar();
    void initStatusBar();

    Plot *d_plot;

    QSpinBox *d_timerInterval;
    QSpinBox *d_numPoints;
    QLabel *d_frameCount;
};

#endif
