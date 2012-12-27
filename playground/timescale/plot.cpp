#include "plot.h"
#include "settings.h"
#include <qwt_date.h>
#include <qwt_timescale_draw.h>
#include <qwt_timescale_engine.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_grid.h>

Plot::Plot( QWidget *parent ):
    QwtPlot( parent )
{
    setCanvasBackground( Qt::white );

    const int axis = QwtPlot::yLeft;

    QwtDateTimeScaleDraw *scaleDraw = new QwtDateTimeScaleDraw();
    setAxisScaleDraw( axis, scaleDraw );

    QwtDateTimeScaleEngine *scaleEngine = new QwtDateTimeScaleEngine();
#if 0
    scaleEngine->setTimeSpec( Qt::UTC );
#endif
    setAxisScaleEngine( axis, scaleEngine );

    QwtPlotPanner *panner = new QwtPlotPanner( canvas() );
    QwtPlotMagnifier *magnifier = new QwtPlotMagnifier( canvas() );

    for ( int i = 0; i < QwtPlot::axisCnt; i++ )
    {
        const bool on = ( i == axis );
        enableAxis( i, on );
        panner->setAxisEnabled( i, on );
        magnifier->setAxisEnabled( i, on );
    }

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setMajorPen( Qt::black, 0, Qt::SolidLine );
    grid->setMinorPen( Qt::gray, 0 , Qt::SolidLine );
    grid->enableX( false );
    grid->enableXMin( false );
    grid->enableY( true );
    grid->enableYMin( true );

    grid->attach( this );
}

void Plot::applySettings( const Settings &settings )
{
    const int axis = QwtPlot::yLeft;

    QwtDateTimeScaleEngine *scaleEngine = 
        static_cast<QwtDateTimeScaleEngine *>( axisScaleEngine( axis ) );

    scaleEngine->setMaxWeeks( settings.maxWeeks );
    setAxisMaxMinor( axis, settings.maxMinorSteps );
    setAxisMaxMajor( axis, settings.maxMajorSteps );
    setAxisScale( axis, QwtDate::toDouble( settings.startDateTime ), 
        QwtDate::toDouble( settings.endDateTime ) );

    replot();
}
