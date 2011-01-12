#ifndef _STYLED_PLOT_H_

#include <qwt_plot.h>

class StyledPlot: public QwtPlot
{
    Q_OBJECT

public:
    StyledPlot(QWidget * = NULL);

    virtual void drawCanvas( QPainter * );
    virtual bool eventFilter( QObject* object, QEvent* event );

    virtual bool event( QEvent * );

private:
    void initStyleSheets();
    void updateCanvasClip();

private:
    QRegion d_canvasClip;

};

#endif

