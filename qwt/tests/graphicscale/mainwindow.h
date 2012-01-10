#include <qmainwindow.h>

class Canvas;

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    virtual ~MainWindow();

private Q_SLOTS:
    void loadSVG();

private:
    void loadSVG( const QString & );

    Canvas *d_canvas[2];
};
