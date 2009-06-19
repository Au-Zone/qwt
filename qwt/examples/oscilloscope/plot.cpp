#include "plot.h"
#include "curvedata.h"
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_curve_fitter.h>
#include <qwt_painter.h>
#include <qevent.h>
#include <QDebug>

Plot::Plot(QWidget *parent):
    QwtPlot(parent),
    d_paintedPoints(0),
    d_interval(0.0, 10.0),
    d_timerId(-1)
{
    d_clock.start();

    setAutoReplot(false);

    // Disable polygon clipping
    QwtPainter::setDeviceClipping(false);

    // We don't need the cache here
    canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
    canvas()->setPaintAttribute(QwtPlotCanvas::PaintPacked, false);

#if 0
#if QT_VERSION >= 0x040000 && defined(Q_WS_X11)
    // Even if not recommended by TrollTech, Qt::WA_PaintOutsidePaintEvent
    // works on X11. This has an tremendous effect on the performance..
    
    canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
    canvas()->setAttribute(Qt::WA_PaintOnScreen, true);
#endif
#endif

    plotLayout()->setCanvasMargin(0);

    for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
        enableAxis(axis, false);

    setAxisScale(QwtPlot::xBottom, d_interval.minValue(), d_interval.maxValue()); 
    setAxisScale(QwtPlot::yLeft, -200.0, 200.0);

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    grid->enableX(true);
    grid->enableXMin(true);
    grid->enableY(true);
    grid->enableYMin(false);
    grid->attach(this);

    QwtPlotMarker *origin = new QwtPlotMarker();
    origin->setLineStyle(QwtPlotMarker::Cross);
    origin->setValue(5.0, 0.0);
    origin->setLinePen(QPen(Qt::gray, 0.0, Qt::DashLine));
    origin->attach(this);

    d_curve = new QwtPlotCurve();
    d_curve->setPaintAttribute(QwtPlotCurve::PaintFiltered);
    d_curve->setStyle(QwtPlotCurve::Lines);
    d_curve->setPen(QPen(Qt::green));
#if 1
    d_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
#endif
    d_curve->setData(CurveData());
    d_curve->attach(this);

    d_timerId = startTimer(10);
}

void Plot::setIntervalLength(double interval)
{
    if ( interval > 0.0 && interval != d_interval.width() )
    {
        d_interval.setMaxValue(d_interval.minValue() + interval);
        setAxisScale(QwtPlot::xBottom, 
            d_interval.minValue(), d_interval.maxValue());

        replot();
    }
}

void Plot::append(double elapsed, double value)
{
    if ( elapsed > d_interval.minValue() )
    {
        CurveData &data = (CurveData &)d_curve->data();
        data.append(QwtDoublePoint(elapsed, value));
    }
}

void Plot::updateCurve()
{
    const int numPoints = d_curve->data().size();
    if ( numPoints > d_paintedPoints )
    {
        d_curve->draw(d_paintedPoints - 1, numPoints - 1);
        d_paintedPoints = numPoints;
    }
}

void Plot::incrementInterval()
{
    d_interval = QwtDoubleInterval(d_interval.maxValue(),
        d_interval.maxValue() + d_interval.width());

    CurveData &data = (CurveData &)d_curve->data();
#if 1
	qDebug() << "Number of Points" << data.size();
#endif
    data.reset(d_interval.minValue());

    // To avoid, that the grid is jumping, we disable 
    // the autocalculation of the ticks and shift them
    // manually instead.

    QwtScaleDiv scaleDiv = *axisScaleDiv(QwtPlot::xBottom);
    scaleDiv.setInterval(d_interval);

    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
    {
        QwtValueList ticks = scaleDiv.ticks(i);
        for ( int j = 0; j < ticks.size(); j++ )
            ticks[j] += d_interval.width();
        scaleDiv.setTicks(i, ticks);
    }
    setAxisScaleDiv(QwtPlot::xBottom, scaleDiv);

    replot();

    d_paintedPoints = data.size();
}

void Plot::timerEvent(QTimerEvent *event)
{
    if ( event->timerId() == d_timerId )
    {
        const double elapsed = d_clock.elapsed() / 1000.0;
        if ( elapsed > d_interval.maxValue() )
            incrementInterval();
        else
            updateCurve();
        return;
    }

    QwtPlot::timerEvent(event);
}

void Plot::resizeEvent(QResizeEvent *event)
{
    d_curve->resetPainter();
    QwtPlot::resizeEvent(event);

    const QColor color(46, 74, 95);
    const QRect cr = canvas()->contentsRect();
    QLinearGradient gradient(cr.topLeft(), cr.topRight());
    gradient.setColorAt(0.0, color.dark(130));
    gradient.setColorAt(0.2, color.dark(110));
    gradient.setColorAt(0.7, color);
    gradient.setColorAt(1.0, color.dark(150));

    QPalette pal = canvas()->palette();
    pal.setBrush(QPalette::Window, QBrush(gradient));
    canvas()->setPalette(pal);
}
