#include <qframe.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_map.h>

class MainWin : public QFrame
{
public:
    MainWin();

protected:
    virtual void timerEvent( QTimerEvent * );
    virtual void paintEvent( QPaintEvent * );
    virtual void drawContents( QPainter * );

private:
    void newValues();

    enum { curveCount = 4 };

    QwtScaleMap xMap[curveCount];
    QwtScaleMap yMap[curveCount];
    QwtPlotCurve curve[curveCount];
};
