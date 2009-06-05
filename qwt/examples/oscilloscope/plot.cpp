#include "plot.h"
#include "curvedata.h"
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_painter.h>
#include <qevent.h>
#include <QDebug>

Plot::Plot(QWidget *parent):
    QwtPlot(parent),
	d_numPoints(0),
	d_timerId(-1)
{
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

	const double interval = 10.0; // 10 seconds
	setAxisScale(QwtPlot::xBottom, 0.0, interval); 
	setAxisScale(QwtPlot::yLeft, -10.0, 10.0);

	QwtPlotGrid *grid = new QwtPlotGrid();
	grid->setPen(QPen(Qt::gray, 0.0, Qt::DotLine));
	grid->enableX(true);
	grid->enableXMin(true);
	grid->enableY(true);
	grid->enableYMin(true);
	grid->attach(this);

	QwtPlotMarker *origin = new QwtPlotMarker();
	origin->setLineStyle(QwtPlotMarker::Cross);
	origin->setValue(5.0, 0.0);
	origin->setLinePen(QPen(Qt::gray, 0.0, Qt::DashLine));
	origin->attach(this);

	d_curve = new QwtPlotCurve();
	d_curve->setStyle(QwtPlotCurve::Lines);
	d_curve->setPen(QPen(Qt::green));
	d_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	d_curve->setData(CurveData(interval));
	d_curve->attach(this);

	d_timerId = startTimer(10);
	reset();
}

void Plot::reset()
{
	CurveData &data = (CurveData &)d_curve->data();
	data.reset();
	d_numPoints = 0;
}

void Plot::append(double value)
{
	CurveData &data = (CurveData &)d_curve->data();
	data.append(value);
}

void Plot::timerEvent(QTimerEvent *event)
{
	if ( event->timerId() == d_timerId )
	{
		const int numPoints = d_curve->dataSize();
		if ( d_numPoints > numPoints )
		{
			d_curve->resetPainter();
			canvas()->repaint();
		}
		else
		{
			if ( d_numPoints > 0 )
			{
				int from = qwtMax(d_numPoints - 4, 0);
				d_curve->draw(from, -1);
			}
		}

		d_numPoints = numPoints;

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

void Plot::updateStatistics()
{
	static int counter;
	static QTime timeStamp;

	if ( !timeStamp.isValid() )
	{
		timeStamp.start();
		counter = 0;
	}
	else
	{
		counter++;

		const double elapsed = timeStamp.elapsed() / 1000.0;
		if ( elapsed >= 5 )
		{
			QString fps;
			fps.setNum(qRound(counter / elapsed));
			fps += " Fps";

			qDebug() << fps;

			counter = 0;
			timeStamp.start();
		}
	}
}
