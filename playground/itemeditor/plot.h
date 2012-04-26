#ifndef _PLOT_H
#define _PLOT_H

#include <qwt_plot.h>

class QColor;
class QRectF;
class QPolygonF;
class QPainterPath;

class Plot: public QwtPlot
{
public:
    Plot( QWidget *parent = NULL );

private:
    void populate();

    void addEllipse( const QRectF &, const QColor & );
    void addRect( const QRectF &, const QColor & );
    void addPolygon( const QPolygonF &, const QColor & );
    void addShape( const QPainterPath &, const QColor & );
};

#endif
