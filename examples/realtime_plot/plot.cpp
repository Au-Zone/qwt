#include "plot.h"
#include <qpainter.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_painter.h>
#include <qwt_plot_item.h>

class EllipseItem: public QwtPlotItem
{
public:
    EllipseItem()
    {
    }

    void setPen(const QPen &pen)
    {
        if ( pen != d_pen )
        {
            d_pen = pen;
            itemChanged();
        }
    }

    void setBrush(const QBrush &brush)
    {
        if ( brush != d_brush )
        {
            d_brush = brush;
            itemChanged();
        }
    }
    void setRect(const QwtDoubleRect &rect)
    {
        if ( d_rect != rect )
        {
            d_rect = rect;
            itemChanged();
        }
    }

    virtual QwtDoubleRect boundingRect() const
    {
        return d_rect;
    }

    virtual void draw(QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRect &) const
    {
        if ( d_rect.isValid() )
        {
            const QRect rect = transform(xMap, yMap, d_rect);
            painter->setPen(d_pen);
            painter->setBrush(d_brush);
            QwtPainter::drawEllipse(painter, rect);
        }
    }
private:
    QPen d_pen;
    QBrush d_brush;
    QwtDoubleRect d_rect;
};

Plot::Plot(QWidget *parent):
    QwtPlot(parent)
{
    const int dim = 1000;
    for ( int axis = 0; axis < QwtPlot::axisCnt; axis ++ )
        setAxisScale(axis, 0.0, dim);

    setCanvasBackground(QColor(Qt::darkBlue));
    plotLayout()->setAlignCanvasToScales(true);

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajPen(QPen(Qt::white, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(this);

    const int numEllipses = 10;

    for ( int i = 0; i < numEllipses; i++ )
    {
        const int x = rand() % dim;
        const int y = rand() % dim;
        const int r = rand() % dim / 6;

        const QwtDoubleRect area(x - r, y - r , 2 * r, 2 * r);

        EllipseItem *item = new EllipseItem();
        item->setRect(area);
        item->setPen(QPen(Qt::yellow));
        item->attach(this);
    }

#if 0
    for ( int axis = 0; axis < QwtPlot::axisCnt; axis ++ )
        enableAxis(axis, false);
#endif
}

void Plot::updateLayout()
{
    QwtPlot::updateLayout();

    const QwtScaleMap xMap = canvasMap(QwtPlot::xBottom);
    const QwtScaleMap yMap = canvasMap(QwtPlot::yLeft);

    const QRect cr = canvas()->contentsRect();
    const double x1 = xMap.invTransform(cr.left());
    const double x2 = xMap.invTransform(cr.right());
    const double y1 = yMap.invTransform(cr.bottom());
    const double y2 = yMap.invTransform(cr.top());

    const double xRatio = (x2 - x1) / cr.width();
    const double yRatio = (y2 - y1) / cr.height();

    emit resized(xRatio, yRatio);
}
