#include "plot.h"
#include "scalepicker.h"
#include "canvaspicker.h"
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_renderer.h>
#include <qwt_symbol.h>
#include <qwt_scale_widget.h>
#include <qwt_wheel.h>
#include <qwt_spline_local.h>
#include <qwt_spline_cubic.h>
#include <qwt_spline_cardinal.h>
#include <qwt_curve_fitter.h>
#include <qwt_spline_curve_fitter.h>
#include <qwt_legend.h>
#include <qwt_legend_label.h>
#include <qevent.h>
#include <qprinter.h>
#include <qprintdialog.h>

class Symbol: public QwtSymbol
{
public:
    Symbol():
        QwtSymbol( QwtSymbol::Ellipse )
    {
        QColor c( Qt::gray );
        c.setAlpha( 100 );
        setBrush( c );

        setPen( Qt::black );
        setSize( 8 );
    }

};

class SplineFitter: public QwtCurveFitter
{
public:
    enum Mode
    {
        HarmonicSpline,
        AkimaSpline,
        CubicSpline,
        CardinalSpline,
        PleasingSpline
    };

    SplineFitter( Mode mode ):
        QwtCurveFitter( QwtCurveFitter::Path )
    {
        switch( mode )
        {   
            case PleasingSpline:
            {   
                d_spline = new QwtSplinePleasing();
                break;
            }
            case HarmonicSpline:
            {   
                d_spline = new QwtSplineLocal( QwtSplineLocal::HarmonicMean );
                break;
            }
            case AkimaSpline:
            {   
                d_spline = new QwtSplineLocal( QwtSplineLocal::Akima );
                break;
            }
            case CubicSpline:
            {   
                QwtSplineCubic *cubicSpline = new QwtSplineCubic();
                cubicSpline->setBoundaryConditions( QwtSplineCubic::Natural );

                d_spline = cubicSpline;
                break;
            }
            case CardinalSpline:
            {   
                d_spline = new QwtSplineLocal( QwtSplineLocal::Cardinal );
            }
        }
    }

    ~SplineFitter()
    {
        delete d_spline;
    }

    void setClosing( bool on )
    {
        d_spline->setClosing( on );
    }

    void setParametric( const QString &parameterType )
    {
        QwtSplineParameter::Type type = QwtSplineParameter::ParameterX;

        if ( parameterType == "Uniform" )
            type = QwtSplineParameter::ParameterUniform;
        else if ( parameterType == "Centripetral" )
            type = QwtSplineParameter::ParameterCentripetral;
        else if ( parameterType == "Chordal" )
            type = QwtSplineParameter::ParameterChordal;
        else if ( parameterType == "Manhattan" )
            type = QwtSplineParameter::ParameterManhattan;

        d_spline->setParametrization( type );
    }

    virtual QPolygonF fitCurve( const QPolygonF &points ) const
    {
        const QPainterPath path = d_spline->pathP( points );

        const QList<QPolygonF> subPaths = path.toSubpathPolygons();
        if ( subPaths.size() == 1 )
            subPaths.first();

        return QPolygonF();
    }

    virtual QPainterPath fitCurvePath( const QPolygonF &points ) const
    {
        return d_spline->pathP( points );
    }

private:
    QwtSpline *d_spline;
};

class Curve: public QwtPlotCurve
{
public:
    Curve( const QString &title, const QColor &color ):
        QwtPlotCurve( title )
    {
        setCurveAttribute( QwtPlotCurve::Fitted, true );
        setRenderHint( QwtPlotItem::RenderAntialiased );

        setPen( color );
        setZ( 100 ); // on top of the marker
    }
};

Plot::Plot( bool parametric, QWidget *parent ):
    QwtPlot( parent )
{
    setTitle( "Points can be dragged using the mouse" );

    setCanvasBackground( Qt::white );
#ifndef QT_NO_CURSOR
    canvas()->setCursor( Qt::PointingHandCursor );
#endif

    QwtLegend *legend = new QwtLegend;
    legend->setDefaultItemMode( QwtLegendData::Checkable );
    insertLegend( legend, QwtPlot::RightLegend );

    connect( legend, SIGNAL( checked( const QVariant &, bool, int ) ),
        SLOT( legendChecked( const QVariant &, bool ) ) );

    d_marker = new QwtPlotMarker( "Marker" );
    d_marker->setLineStyle( QwtPlotMarker::VLine );
    d_marker->setLinePen( QPen( Qt::darkRed, 0, Qt::DotLine ) );

    QwtText text( "click on the axes" );
    text.setBackgroundBrush( Qt::white );
    text.setColor( Qt::darkRed );

    d_marker->setLabel( text );
    d_marker->setLabelOrientation( Qt::Vertical );
    d_marker->setXValue( 5 );
    d_marker->attach( this );
    // axes

    setAxisScale( QwtPlot::xBottom, 0.0, 100.0 );
    setAxisScale( QwtPlot::yLeft, -20.0, 120.0 );

    // Avoid jumping when label with 3 digits
    // appear/disappear when scrolling vertically

    QwtScaleDraw *sd = axisScaleDraw( QwtPlot::yLeft );
    sd->setMinimumExtent( sd->extent( axisWidget( QwtPlot::yLeft )->font() ) );

    // curves 
    d_curve = new Curve( "Lines", QColor() );
    d_curve->setStyle( QwtPlotCurve::NoCurve );
    d_curve->setSymbol( new Symbol() );
    d_curve->setItemAttribute( QwtPlotItem::Legend, false );
    d_curve->setZ( 1000 ); 

    QPolygonF points;
#if 1
    points << QPointF( 10, 30 ) << QPointF( 20, 90 ) << QPointF( 25, 60 )
        << QPointF( 35, 38 ) << QPointF( 42, 40 ) << QPointF( 55, 60 )
        << QPointF( 60, 50 ) << QPointF( 65, 80 ) << QPointF( 73, 30 )
        << QPointF( 82, 30 ) << QPointF( 87, 40 ) << QPointF( 95, 70 );
#else
    points << QPointF( 10, 30 ) << QPointF( 60, 50 ) << QPointF( 95, 70 );
#endif

    d_curve->setSamples( points );
    d_curve->attach( this );

    // 

    Curve *curve0 = new Curve( "Cardinal", Qt::black);
    curve0->setCurveFitter( new SplineFitter( SplineFitter::CardinalSpline ) );
    curve0->attach( this );

    Curve *curve1 = new Curve( "Pleasing", Qt::darkBlue);
    curve1->setCurveFitter( new SplineFitter( SplineFitter::PleasingSpline ) );
    curve1->attach( this );

    Curve *curve2 = new Curve( "Cubic", Qt::darkRed);
    curve2->setCurveFitter( new SplineFitter( SplineFitter::CubicSpline ) );
    curve2->attach( this );

    Curve *curve3 = new Curve( "Akima", Qt::darkCyan);
    curve3->setCurveFitter( new SplineFitter( SplineFitter::AkimaSpline ) );
    curve3->attach( this );

    Curve *curve4 = new Curve( "Harmonic", Qt::darkYellow);
    curve4->setCurveFitter( new SplineFitter( SplineFitter::HarmonicSpline ) );
    curve4->attach( this );

    QwtPlotItemList curves = itemList( QwtPlotItem::Rtti_PlotCurve );
#if 1
    for ( int i = 0; i < curves.size(); i++ )
        showCurve( curves[i], true );
#else
    for ( int i = 0; i < curves.size(); i++ )
        showCurve( curves[i], false );

    showCurve( curve2, true );
    showCurve( curve3, true );
#endif

    setOverlaying( false );

    // ------------------------------------
    // The scale picker translates mouse clicks
    // on the bottom axis into clicked() signals
    // ------------------------------------

    ScalePicker *scalePicker = new ScalePicker( this );
    connect( scalePicker, SIGNAL( clicked( int, double ) ),
        this, SLOT( updateMarker( int, double ) ) );

    // ------------------------------------
    // The canvas picker handles all mouse and key
    // events on the plot canvas
    // ------------------------------------

    ( void ) new CanvasPicker( !parametric, this );

    // ------------------------------------
    // We add a wheel to the canvas
    // ------------------------------------

    d_wheel = new QwtWheel( canvas() );
    d_wheel->setOrientation( Qt::Vertical );
    d_wheel->setRange( -100, 100 );
    d_wheel->setValue( 0.0 );
    d_wheel->setMass( 0.2 );
    d_wheel->setTotalAngle( 4 * 360.0 );
    d_wheel->resize( 16, 60 );

    plotLayout()->setAlignCanvasToScale( QwtPlot::xTop, true );
    plotLayout()->setAlignCanvasToScale( QwtPlot::xBottom, true );
    plotLayout()->setAlignCanvasToScale( QwtPlot::yLeft, true );
    plotLayout()->setCanvasMargin( d_wheel->width() + 4, QwtPlot::yRight );

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
    const double range = axisScaleDiv( QwtPlot::yLeft ).range();
    setAxisScale( QwtPlot::yLeft, value, value + range );
    replot();
}

bool Plot::eventFilter( QObject *object, QEvent *e )
{
    if ( e->type() == QEvent::Resize )
    {
        if ( object == canvas() )
        {
            const int margin = 2;

            const QRect cr = canvas()->contentsRect();
            d_wheel->move( cr.right() - margin - d_wheel->width(), 
                cr.center().y() - ( d_wheel->height() ) / 2 );
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


void Plot::legendChecked( const QVariant &itemInfo, bool on )
{
    QwtPlotItem *plotItem = infoToItem( itemInfo );
    if ( plotItem )
        showCurve( plotItem, on );
}

void Plot::showCurve( QwtPlotItem *item, bool on )
{
    item->setVisible( on );

    QwtLegend *lgd = qobject_cast<QwtLegend *>( legend() );

    QList<QWidget *> legendWidgets =
        lgd->legendWidgets( itemToInfo( item ) );

    if ( legendWidgets.size() == 1 )
    {
        QwtLegendLabel *legendLabel =
            qobject_cast<QwtLegendLabel *>( legendWidgets[0] );

        if ( legendLabel )
            legendLabel->setChecked( on );
    }

    replot();
}

void Plot::setClosed( bool on )
{
    QwtPlotItemList curves = itemList( QwtPlotItem::Rtti_PlotCurve );
    for ( int i = 0; i < curves.size(); i++ )
    {
        QwtPlotCurve *curve = dynamic_cast<QwtPlotCurve *>( curves[i] );

        SplineFitter *fitter = dynamic_cast<SplineFitter*>( curve->curveFitter() );
        if ( fitter )
            fitter->setClosing( on );
    }

    replot();
}

void Plot::setParametric( const QString &parameterType )
{
    QwtPlotItemList curves = itemList( QwtPlotItem::Rtti_PlotCurve );
    for ( int i = 0; i < curves.size(); i++ )
    {
        QwtPlotCurve *curve = ( QwtPlotCurve *)curves[i];

        SplineFitter *fitter = dynamic_cast<SplineFitter*>( curve->curveFitter() );
        if ( fitter )
            fitter->setParametric( parameterType );
    }

    replot();
}

void Plot::setOverlaying( bool on )
{
    QPolygonF points;
    for ( size_t i = 0; i < d_curve->dataSize(); i++ )
        points += d_curve->sample( i );

    QwtPlotItemList curves = itemList( QwtPlotItem::Rtti_PlotCurve );

    const int off0 = 0 - 10 * curves.count() / 2;

    for ( int i = 0; i < curves.size(); i++ )
    {
        QwtPlotCurve *curve = static_cast<QwtPlotCurve *>( curves[i] );
        if ( curve == d_curve )
            continue;

        if ( on )
        {
            curve->setSymbol( NULL );
            curve->setSamples( points );
        }
        else
        {
            curve->setSymbol( new Symbol() );

            curve->setSamples( points.translated( 0, off0 + i * 10 ) );
        }

    }

    d_curve->setVisible( on );

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
