#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <qmainwindow.h>

class FormulaView;

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private Q_SLOTS:
    void load();

private:
    void loadFormula( const QString & );

private:
    FormulaView *d_view;
};

#endif
