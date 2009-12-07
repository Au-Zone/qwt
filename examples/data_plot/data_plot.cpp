#include <stdlib.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_math.h>
#include "data_plot.h"
#if 1
#include <qdebug.h>
#include <QBasicTimer>
#endif

const double numSeconds = 10.0; // seconds

DataPlot::DataPlot(QWidget *parent):
    QwtPlot(parent)
{
    // Assign a title
    setTitle("A Test for High Refresh Rates");

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
    
    d_started.start();

    for ( int i = 0; i < PLOT_SIZE; i++ )
        d_x[i] = i * (numSeconds / PLOT_SIZE);

    updateValues();

    // Insert new curves
    QwtPlotCurve *cRight = new QwtPlotCurve("Data Moving Right");
    cRight->setPen(QPen(Qt::red));

    QwtPlotCurve *cLeft = new QwtPlotCurve("Data Moving Left");
    cLeft->setPen(QPen(Qt::blue));

    // Attach (don't copy) data. Both curves use the same x array.
    cRight->setRawSamples(d_x, d_y, PLOT_SIZE);
    cLeft->setRawSamples(d_x, d_z, PLOT_SIZE);

    cRight->attach(this);
    cLeft->attach(this);

#if 0
    //  Insert zero line at y = 0
    QwtPlotMarker *mY = new QwtPlotMarker();
    mY->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    mY->setLineStyle(QwtPlotMarker::HLine);
    mY->setYValue(0.0);
    mY->attach(this);
#endif

    // Axis 
    setAxisTitle(QwtPlot::xBottom, "Time/seconds");
    setAxisScale(QwtPlot::xBottom, 0.0, numSeconds);

    setAxisTitle(QwtPlot::yLeft, "Values");
    setAxisScale(QwtPlot::yLeft, -1.5, 1.5);
    
    setTimerInterval(0.0); 
}

//
//  Set a plain canvas frame and align the scales to it
//
void DataPlot::alignScales()
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
}

void DataPlot::setTimerInterval(int ms)
{
    d_timer.start(ms, this);
}

//  Generate new values 
void DataPlot::timerEvent(QTimerEvent *)
{
    updateValues();

    // the axes are unchanged. So all we need to do
    // is to erase and repaint the content of the canvas 
    // ( without te frame )

    canvas()->update(canvas()->contentsRect());
    //canvas()->repaint(canvas()->contentsRect());
}

void DataPlot::updateValues()
{
    const double elapsed = d_started.elapsed() / 1000.0;
    const double interval = numSeconds / PLOT_SIZE;

    for ( int i = 0; i < PLOT_SIZE; i++ )
    {
        const double v = elapsed - i * interval;

        d_y[i] = ::cos(v) * fmod(v, 1.0);
        d_z[PLOT_SIZE - i - 1] = ::sin(v) * fmod(v, 1.0);
    }
}
