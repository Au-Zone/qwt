#include "plot.h"
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_renderer.h>
#include <qwt_symbol.h>
#include <qwt_scale_widget.h>
#include <qwt_wheel.h>
#include <qwt_spline.h>
#include <qwt_curve_fitter.h>
#include <qevent.h>
#include <qprinter.h>
#include <qprintdialog.h>

class CurveFitter1: public QwtCurveFitter
{
public:
    CurveFitter1():
        QwtCurveFitter( QwtCurveFitter::Path )
    {
    }

    virtual QPolygonF fitCurve( const QPolygonF &points ) const
    {
        const QPainterPath path = fitCurvePath( points );

        const QList<QPolygonF> subPaths = fitCurvePath( points ).toSubpathPolygons();
        if ( subPaths.size() == 1 )
            subPaths.first();

        return QPolygonF();
    }

    virtual QPainterPath fitCurvePath( const QPolygonF &points ) const
    {
#if 1
        return QwtSplineAkima::path( points );
#endif
#if 0
        return QwtSplineNatural::path( points );
#endif
#if 0
        return QwtSpline::path( points, 0.01 );
#endif
    }
};

class CurveFitter2: public QwtCurveFitter
{
public:
    CurveFitter2():
        QwtCurveFitter( QwtCurveFitter::Polygon )
    {
    }

    virtual QPolygonF fitCurve( const QPolygonF &polygon ) const
    {
#if 0
        return QwtSpline::polygon( polygon, 1.0, 500 );
#else
        return QwtSplineNatural::polygon( polygon, 500 );
#endif
    }

    virtual QPainterPath fitCurvePath( const QPolygonF &polygon ) const
    {
        const QPolygonF fittedPoints = QwtSplineNatural::polygon( polygon, 500 );

        QPainterPath path;
        path.addPolygon( fittedPoints );
        return path;
    }
};

Plot::Plot( QWidget *parent ):
    QwtPlot( parent )
{
    setTitle( "Movable Spline" );

    setCanvasBackground( Qt::white );
#ifndef QT_NO_CURSOR
    canvas()->setCursor( Qt::PointingHandCursor );
#endif

    d_marker = new QwtPlotMarker( "Marker" );
    d_marker->setLineStyle( QwtPlotMarker::VLine );
    d_marker->setLinePen( QPen( Qt::darkRed, 0, Qt::DotLine ) );

    QwtText text( "Showing a click on the axes" );
    text.setBackgroundBrush( Qt::white );
    text.setColor( Qt::darkRed );

    d_marker->setLabel( text );
    d_marker->setLabelOrientation( Qt::Vertical );
    d_marker->setXValue( 5 );
    d_marker->attach( this );
    // axes

    setAxisScale( QwtPlot::xBottom, 0.0, 100.0 );
    setAxisScale( QwtPlot::yLeft, 0.0, 100.0 );

    // Avoid jumping when label with 3 digits
    // appear/disappear when scrolling vertically

    QwtScaleDraw *sd = axisScaleDraw( QwtPlot::yLeft );
    sd->setMinimumExtent( sd->extent( axisWidget( QwtPlot::yLeft )->font() ) );

    plotLayout()->setAlignCanvasToScales( true );

    // curve 
    QwtPlotCurve *curve = new QwtPlotCurve();
#if 0
    curve->setCurveFitter( new CurveFitter1 );
#endif
    curve->setCurveAttribute( QwtPlotCurve::Fitted, true );
    curve->setRenderHint( QwtPlotItem::RenderAntialiased );

    curve->setPen( Qt::darkBlue );
    curve->setSymbol( new QwtSymbol( QwtSymbol::Ellipse,
        Qt::gray, QPen( Qt::darkBlue ) , QSize( 8, 8 ) ) );

    QPolygonF points;
    points << QPointF( 10, 30 ) << QPointF( 20, 90 ) << QPointF( 25, 60 )
        << QPointF( 35, 38 ) << QPointF( 42, 40 ) << QPointF( 55, 60 )
        << QPointF( 60, 50 ) << QPointF( 65, 80 ) << QPointF( 73, 30 )
        << QPointF( 82, 30 ) << QPointF( 87, 40 ) << QPointF( 95, 70 );

    curve->setSamples( points );
    curve->setZ( 100 ); // on top of the marker
    curve->attach( this );

    // --

    replot();

    // ------------------------------------
    // We add a wheel to the canvas
    // ------------------------------------

    d_wheel = new QwtWheel( canvas() );
    d_wheel->setOrientation( Qt::Vertical );
    d_wheel->setRange( -100, 100 );
    d_wheel->setValue( 0.0 );
    d_wheel->setMass( 0.2 );
    d_wheel->setTotalAngle( 4 * 360.0 );

    connect( d_wheel, SIGNAL( valueChanged( double ) ),
        SLOT( scrollLeftAxis( double ) ) );

    // we need the resize events, to lay out the wheel
    canvas()->installEventFilter( this );

    d_wheel->setWhatsThis(
        "With the wheel you can move the visible area." );
    axisWidget( xBottom )->setWhatsThis(
        "Selecting a value at the scale will insert a new curve." );
}

void Plot::scrollLeftAxis( double value )
{
    setAxisScale( yLeft, value, value + 100.0 );
    replot();
}

bool Plot::eventFilter( QObject *object, QEvent *e )
{
    if ( e->type() == QEvent::Resize )
    {
        if ( object == canvas() )
        {
            const int w = 16;
            const int h = 50;
            const int margin = 2;

            const QRect cr = canvas()->contentsRect();
            d_wheel->setGeometry(
                cr.right() - margin - w, cr.center().y() - h / 2, w, h );
        }
    }

    return QwtPlot::eventFilter( object, e );
}

void Plot::updateMarker( int axis, double value )
{
    if ( axis == yLeft || axis == yRight )
    {
        d_marker->setLineStyle( QwtPlotMarker::HLine );
        d_marker->setLabelOrientation( Qt::Horizontal );
        d_marker->setYValue( value );
    }
    else
    {
        d_marker->setLineStyle( QwtPlotMarker::VLine );
        d_marker->setLabelOrientation( Qt::Vertical );
        d_marker->setXValue( value );
    }

    replot();
}

#ifndef QT_NO_PRINTER

void Plot::printPlot()
{
    QPrinter printer( QPrinter::HighResolution );
    printer.setOrientation( QPrinter::Landscape );
    printer.setOutputFileName( "spline.pdf" );

    QPrintDialog dialog( &printer );
    if ( dialog.exec() )
    {
        QwtPlotRenderer renderer;

        if ( printer.colorMode() == QPrinter::GrayScale )
        {
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground );
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground );
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame );
            renderer.setLayoutFlag( QwtPlotRenderer::FrameWithScales );
        }

        renderer.renderTo( this, printer );
    }
}

#endif

