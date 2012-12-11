#include <qapplication.h>
#include <qpainter.h>
#include <qwt_math.h>
#include <qwt_symbol.h>
#include <qwt_curve_fitter.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_map.h>
#include <qevent.h>
#include "curvdemo2.h"

class Curve: public QwtPlotCurve
{
public:
    virtual void updateSamples( double phase )
    {
        setSamples( points( phase ) );
    }

private:
    virtual QPolygonF points( double phase ) const = 0;
};

class Curve1: public Curve
{
public:
    Curve1()
    {
        setPen( QPen( QColor( 150, 150, 200 ), 2 ) );
        setStyle( QwtPlotCurve::Lines );

        QwtSplineCurveFitter *curveFitter = new QwtSplineCurveFitter();
        curveFitter->setSplineSize( 150 );
        setCurveFitter( curveFitter );

        setCurveAttribute( QwtPlotCurve::Fitted, true );

        QwtSymbol *symbol = new QwtSymbol( QwtSymbol::XCross );
        symbol->setPen( QPen( Qt::yellow, 2 ) );
        symbol->setSize( 7 );

        setSymbol( symbol );
    }

    virtual QPolygonF points( double phase ) const
    {
        QPolygonF points;

        const int numSamples = 15;
        for ( int i = 0; i < numSamples; i++ )
        {
            const double v = 6.28 * double( i ) / double( numSamples - 1 );
            points += QPointF( qSin( v - phase ), v );
        }

        return points;
    }
};

class Curve2: public Curve
{
public:
    Curve2()
    {
        setStyle( QwtPlotCurve::Sticks );
        setPen( QPen( QColor( 200, 150, 50 ) ) );

        setSymbol( new QwtSymbol( QwtSymbol::Ellipse,
            QColor( Qt::blue ), QColor( Qt::yellow ), QSize( 5, 5 ) ) );
    }

private:
    virtual QPolygonF points( double phase ) const
    {
        QPolygonF points;

        const int numSamples = 15;
        for ( int i = 0; i < numSamples; i++ )
        {
            const double v = 6.28 * double( i ) / double( numSamples - 1 );
            points += QPointF( v, qCos( 3.0 * ( v + phase ) ) );
        }

        return points;
    }
};

class Curve3: public Curve
{       
public: 
    Curve3()
    {
        setStyle( QwtPlotCurve::Lines );
        setPen( QColor( 100, 200, 150 ) );

        QwtSplineCurveFitter* curveFitter = new QwtSplineCurveFitter();
        curveFitter->setFitMode( QwtSplineCurveFitter::ParametricSpline );
        curveFitter->setSplineSize( 200 );
        setCurveFitter( curveFitter );

        setCurveAttribute( QwtPlotCurve::Fitted, true );
    }   

private:
    virtual QPolygonF points( double phase ) const
    {
        QPolygonF points;

        const int numSamples = 15;
        for ( int i = 0; i < numSamples; i++ )
        {
            const double v = 6.28 * double( i ) / double( numSamples - 1 );
            points += QPointF( qSin( v - phase ), qCos( 3.0 * ( v + phase ) ) );
        }

        return points;
    }
};  

class Curve4: public Curve
{       
public: 
    Curve4()
    {
        setStyle( QwtPlotCurve::Lines );
        setPen( QColor( Qt::red ) );

        QwtSplineCurveFitter *curveFitter = new QwtSplineCurveFitter();
        curveFitter->setSplineSize( 200 );
        setCurveFitter( curveFitter );

        setCurveAttribute( QwtPlotCurve::Fitted, true );

        initSamples();
    }   

private:
    virtual QPolygonF points( double phase ) const
    {
        const double s = 0.25 * qSin( phase );
        const double c = qSqrt( 1.0 - s * s );

        QPolygonF points;
        for ( size_t i = 0; i < dataSize(); i++ )
        {
            const QPointF p = sample( i );

            const double u = p.x();
            const double v = p.y();

            points += QPointF( u * c - v * s, 
                v * c + u * s );
        }

        return points;
    }

    void initSamples()
    {
        const int numSamples = 13;

        QPolygonF points;
        for ( int i = 0; i < numSamples; i++ )
        {
            const double angle = i * ( 2.0 * M_PI / ( numSamples - 1 ) );

            const double f = ( i % 2 ) ? 0.5 : 1.0;
            points += f * QPointF( qCos( angle ), qSin( angle ) );
        }

        setSamples( points );
    }
};  

MainWindow::MainWindow( QWidget *parent ):
    QFrame( parent),
    d_phase( 0.0 )
{
    setFrameStyle( QFrame::Box | QFrame::Raised );
    setLineWidth( 2 );
    setMidLineWidth( 3 );

    QPalette p = palette();
    p.setColor( backgroundRole(), QColor( 30, 30, 50 ) );
    setPalette( p );

    d_curves[0] = new Curve1();
    d_curves[1] = new Curve2();
    d_curves[2] = new Curve3();
    d_curves[3] = new Curve4();

    updateCurves();

    ( void )startTimer( 250 );
}

MainWindow::~MainWindow()
{
    for ( int i = 0; i < CurveCount; i++ )
        delete d_curves[ i ];
}

void MainWindow::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setRenderHint( QPainter::Antialiasing, true );
    painter.setClipRegion( event->region() );

    drawCurves( &painter, contentsRect() );
    drawFrame( &painter );
}

void MainWindow::drawCurves( QPainter *painter, const QRect &canvasRect )
{
    QwtScaleMap xMap;
    xMap.setPaintInterval( canvasRect.left(), canvasRect.right() );

    QwtScaleMap yMap;
    yMap.setPaintInterval( canvasRect.top(), canvasRect.bottom() );

    xMap.setScaleInterval( -1.5, 1.5 );
    yMap.setScaleInterval( 0.0, 6.28 );
    d_curves[0]->draw( painter, xMap, yMap, canvasRect );

    xMap.setScaleInterval( 0.0, 6.28 );
    yMap.setScaleInterval( -3.0, 1.1 );
    d_curves[1]->draw( painter, xMap, yMap, canvasRect );

    xMap.setScaleInterval( -1.1, 3.0 );
    yMap.setScaleInterval( -1.1, 3.0 );
    d_curves[2]->draw( painter, xMap, yMap, canvasRect );

    xMap.setScaleInterval( -5, 1.1 );
    yMap.setScaleInterval( -1.1, 5.0 );
    d_curves[3]->draw( painter, xMap, yMap, canvasRect );
}

void MainWindow::timerEvent( QTimerEvent * )
{
    updateCurves();
    repaint();
}

void MainWindow::updateCurves()
{
    for ( int i = 0; i < CurveCount; i++ )
        d_curves[i]->updateSamples( d_phase );

    d_phase += 0.0628;
    if ( d_phase > 6.28 )
        d_phase = 0.0;
}

int main ( int argc, char **argv )
{
    QApplication a( argc, argv );

    MainWindow w;

    w.resize( 300, 300 );
    w.show();

    return a.exec();
}
