#include "plot.h"
#include "timescaleengine.h"
#include "timescaledraw.h"
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_scale_widget.h>
#include <qdatetime.h>

Plot::Plot( QWidget *parent ):
	QwtPlot( parent )
{
#if 0
	qDebug() << "MIN: " << qwtToDateTime( DATE_MIN );
	qDebug() << "ABOVE: " << qwtToDateTime( DATE_MIN - 1 );
	qDebug() << "BELOW: " << qwtToDateTime( DATE_MIN + 1 );
	exit( 0 );
#endif
	const int axis = QwtPlot::yLeft;

    TimeScaleDraw *scaleDraw = new TimeScaleDraw();
	setAxisScaleDraw( axis, scaleDraw );
	setAxisScaleEngine( axis, new TimeScaleEngine() );

#if 0
	const QDateTime from( QDate( 2005, 1, 1 ) );
	const QDateTime to( QDate( 2012, 12, 31 ) );
#else
	const QDateTime from( QDate( 1, 1, 1 ) );
	const QDateTime to( QDate( 1000000, 12, 31 ) );
#endif

	setAxisScale( axis, qwtFromDateTime( from ), qwtFromDateTime( to ) );

#if 0
	axisWidget( axis )->setFont( QFont( "Verdana", 16 ) );
#endif

    QwtPlotPanner *panner = new QwtPlotPanner( canvas() );
    QwtPlotMagnifier *magnifier = new QwtPlotMagnifier( canvas() );

	for ( int i = 0; i < QwtPlot::axisCnt; i++ )
	{
		panner->setAxisEnabled( i, i == axis );
		magnifier->setAxisEnabled( i, i == axis );
	}
}
