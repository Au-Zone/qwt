#ifndef _PLOT_H
#define _PLOT_H

#include <qwt_plot.h>
#include "shapefactory.h"

class QColor;
class QRectF;

class Plot: public QwtPlot
{
public:
    Plot( QWidget *parent = NULL );

private:
    void populate();

    void addShape( const QString &title,
        ShapeFactory::Shape, const QColor &,
        const QPointF &, const QSizeF & );
};

#endif
