#include <qapplication.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_picker.h>

class MyTransformation: public QwtScaleTransformation
{
public:
    MyTransformation():
        QwtScaleTransformation( Other )
    {
    }

#if 1
#define TRANS sqrt
#define INV qwtSqr
#else
#define TRANS qwtSqr
#define INV sqrt
#endif

    virtual double xForm( double s, double s1, double s2,
        double p1, double p2 ) const
    {
        double dp = p2 - p1;
        double ds = TRANS(s2) - TRANS(s1);
        double ds1 = TRANS(s) - TRANS(s1);

        double p = p1 + dp / ds * ds1;

        return p;
    }

    virtual double invXForm( double p, double p1, double p2,
        double s1, double s2 ) const
    {
        double dp = p2 - p1;
        double dp1 = p - p1;

        double s = INV( TRANS(s1) + ( dp1 / dp ) * ( TRANS(s2) - TRANS(s1) ) );
        return s;
    }

    virtual QwtScaleTransformation *copy() const
    {
        return new MyTransformation();
    }

};

class MyScaleEngine: public QwtLinearScaleEngine
{
public:
    virtual QwtScaleTransformation *transformation() const
    {
#if 1
        return new MyTransformation();
#else
        return new QwtScaleTransformation( QwtScaleTransformation::Log10 );
#endif
    }
};

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QwtPlot plot;
    plot.setTitle( "Plot Demo" );
    plot.setCanvasBackground( Qt::white );
    plot.setAxisScale(QwtPlot::yLeft, 0.0, 10.0 );
    plot.setAxisScaleEngine(QwtPlot::xBottom, new MyScaleEngine );
    plot.setAxisScale(QwtPlot::xBottom, 0.0, 100.0 );

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->attach(&plot);

    QwtPlotCurve *curve = new QwtPlotCurve();
    curve->setTitle("Some Points");
    curve->setPen( QPen( Qt::blue, 4 ) ),
    curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse, 
        QBrush( Qt::yellow ), QPen( Qt::red, 2 ), QSize( 8, 8 ) );
    curve->setSymbol( symbol );

    QPolygonF points;
    points << QPointF( 10.0, 4.4 ) << QPointF( 20.0, 3.0 )
        << QPointF( 30.0, 4.5 ) << QPointF( 40.0, 6.8 )
        << QPointF( 50.0, 7.9 ) << QPointF( 60.0, 7.1 );
    curve->setSamples( points );

    curve->attach( &plot );

    QwtPlotPicker *picker = new QwtPlotPicker( plot.canvas() );
    picker->setTrackerMode( QwtPlotPicker::AlwaysOn );
    plot.resize(600,400);
    plot.show();

    return a.exec(); 
}
