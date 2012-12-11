#include <qframe.h>

class QPainter;
class Curve;

class MainWindow: public QFrame
{
public:
    MainWindow( QWidget * = NULL);
    virtual ~MainWindow();

protected:
    virtual void timerEvent( QTimerEvent * );
    virtual void paintEvent( QPaintEvent * );

private:
    void updateCurves();
    void drawCurves( QPainter *, const QRect & );

    enum { CurveCount = 4 };
    Curve *d_curves[CurveCount];

    double d_phase;
};
