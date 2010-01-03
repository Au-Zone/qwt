#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include "plot.h"
#include "circularbuffer.h"

static double wave(double x) 
{
    const double period = 1.0;
    const double c = 5.0;

    double v = ::fmod(x, period);

    const double amplitude = qwtAbs(x - qRound(x / c) * c) / ( 0.5 * c );
    v = amplitude * ::sin(v / period * 2 * M_PI);

    return v;
}

static double noise(double)
{
    return 2.0 * ( rand() / ((double)RAND_MAX + 1) ) - 1.0;
}

Plot::Plot(QWidget *parent):
    QwtPlot(parent),
    d_interval(10.0), // seconds
    d_timerId(-1)
{
    // Assign a title
    setTitle("Testing Refresh Rates");

    setCanvasBackground(Qt::white);

    // Disable polygon clipping
    QwtPainter::setDeviceClipping(false);

    // We don't need the cache here
    canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
    canvas()->setPaintAttribute(QwtPlotCanvas::PaintPacked, false);

#if QT_VERSION >= 0x040000
#ifdef Q_WS_X11
    /*
       Qt::WA_PaintOnScreen is only supported for X11, but leads
       to substantial bugs with Qt 4.2.x/Windows
     */
    canvas()->setAttribute(Qt::WA_PaintOnScreen, true);
#endif
#endif

    alignScales();

    // Insert new curves
    d_curve = new QwtPlotCurve("Data Moving Right");
    d_curve->setPen(QPen(Qt::black));
    d_curve->setData(CircularBuffer(d_interval, 10));
    d_curve->attach(this);

    // Axis 
    setAxisTitle(QwtPlot::xBottom, "Seconds");
    setAxisScale(QwtPlot::xBottom, -d_interval, 0.0);

    setAxisTitle(QwtPlot::yLeft, "Values");
    setAxisScale(QwtPlot::yLeft, -1.0, 1.0);
    
    d_clock.start();
    setTimerInterval(0.0); 
}

//
//  Set a plain canvas frame and align the scales to it
//
void Plot::alignScales()
{
    // The code below shows how to align the scales to
    // the canvas frame, but is also a good example demonstrating
    // why the spreaded API needs polishing.

    canvas()->setFrameStyle(QFrame::Box | QFrame::Plain );
    canvas()->setLineWidth(1);

    for ( int i = 0; i < QwtPlot::axisCnt; i++ )
    {
        QwtScaleWidget *scaleWidget = (QwtScaleWidget *)axisWidget(i);
        if ( scaleWidget )
            scaleWidget->setMargin(0);

        QwtScaleDraw *scaleDraw = (QwtScaleDraw *)axisScaleDraw(i);
        if ( scaleDraw )
            scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    }

    plotLayout()->setAlignCanvasToScales(true);
}

void Plot::setTimerInterval(int ms)
{
    if ( d_timerId >= 0 )
        killTimer(d_timerId);

    d_timerId = startTimer(ms);
}

void Plot::setNumPoints(int numPoints)
{
    CircularBuffer &buffer = (CircularBuffer &)d_curve->data();
    buffer.fill(d_interval, numPoints);
}

void Plot::setFunctionType(int functionType)
{
    CircularBuffer &buffer = (CircularBuffer &)d_curve->data();
    switch(functionType)
    {
        case Plot::Wave:
            buffer.setFunction(wave);
            break;
        case Plot::Noise:
            buffer.setFunction(noise);
            break;
        default:
            buffer.setFunction(NULL);
    }

    buffer.fill(d_interval, buffer.size());
}

void Plot::timerEvent(QTimerEvent *)
{
    CircularBuffer &buffer = (CircularBuffer &)d_curve->data();
    buffer.setReferenceTime(d_clock.elapsed() / 1000.0);

    // the axes are unchanged. So all we need to do
    // is to erase and repaint the content of the canvas 
    // ( without te frame )

#if 1
    canvas()->invalidatePaintCache();
    //canvas()->update(canvas()->contentsRect());
    canvas()->repaint(canvas()->contentsRect());
#else
    replot();
#endif
}
