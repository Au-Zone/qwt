#include <qwt_spline_cubic.h>
#include <qpolygon.h>
#include <qdebug.h>

#define DEBUG_ERRORS 1

static inline bool fuzzyCompare( double a, double b ) 
{
#if QT_VERSION < 0x040600
    const int eps = 0.000000000001;
    return ( ( qAbs(a) <= eps ) && ( qAbs(b) <= eps ) ) || qFuzzyCompare(a, b);
#else
    return ( qFuzzyIsNull(a) && qFuzzyIsNull(b) ) || qFuzzyCompare(a, b);
#endif
}

class CubicSpline: public QwtSplineCubic
{
public:
    CubicSpline( const char* name, int minPoints = 3 );
    virtual ~CubicSpline();

    int minPoints() const;
    void testSpline( const QPolygonF &points ) const;
    
protected:

    bool verifyBoundary( QwtSpline::BoundaryPosition, const QPolygonF &points,
        const QVector<double> &m, const QVector<double> &cv ) const;

    int verifyNodesCV( const QPolygonF &p, const QVector<double> &cv ) const;
    int verifyNodesM( const QPolygonF &p, const QVector<double> &m ) const;

    QwtSplinePolynomial polynomial( int index, 
        const QPolygonF &points, const QVector<double> &m ) const;

    QwtSplinePolynomial polynomialCV( int index,
        const QPolygonF &points, const QVector<double> &cv ) const;

private:
    const QString d_name;
    const int d_minPoints;
};

CubicSpline::CubicSpline( const char* name, int minPoints ):
    d_name( name ),
    d_minPoints( minPoints )
{
}

CubicSpline::~CubicSpline()
{
}

int CubicSpline::minPoints() const
{
    return d_minPoints;
}

void CubicSpline::testSpline( const QPolygonF &points ) const
{
    const QVector<double> m = slopes( points );
    const QVector<double> cv = curvatures( points );

    if ( m.size() != points.size() )
    {
        qDebug() << qPrintable( d_name ) << "("
            << points.size() << "):" << "not implemented";
        return;
    }

    const bool okStart = verifyBoundary( QwtSpline::AtBeginning, points, m, cv );
    const int numErrorsM = verifyNodesM( points, m );
    const int numErrorsCV = verifyNodesCV( points, cv );
    const bool okEnd = verifyBoundary( QwtSpline::AtEnd, points, m, cv );

    if ( !okStart || numErrorsM > 0 || numErrorsCV > 0 || !okEnd )
    {
        qDebug() << qPrintable( d_name ) << "("
            << points.size() << "):" << false;

#if DEBUG_ERRORS > 0
        if ( !okStart )
            qDebug() << "  invalid start condition";
    
        if ( numErrorsM > 0 )
            qDebug() << "  invalid node conditions ( slope )" << numErrorsM;
        
        if ( numErrorsCV > 0 )
            qDebug() << "  invalid node conditions ( curvature )" << numErrorsCV;

        if ( !okEnd )
            qDebug() << "  invalid end condition";
#endif
    }
}

bool CubicSpline::verifyBoundary( QwtSpline::BoundaryPosition pos, const QPolygonF &points,
    const QVector<double> &m, const QVector<double> &cv ) const
{
    if ( boundaryType() != QwtSpline::ConditionalBoundaries )
    {
        // periodic or closed 

        const int n = points.size();

        const QwtSplinePolynomial p1 = polynomial( n - 2, points, m );
        const QwtSplinePolynomial p2 = polynomial( 0, points, m );

        const double dx = points[n-1].x() - points[n-2].x();
        return fuzzyCompare( p1.curvatureAt( dx ), p2.curvatureAt( 0.0 ) ) &&
            fuzzyCompare( p1.slopeAt( dx ), p2.slopeAt( 0.0 ) );
    }

    bool ok = false;

    switch( boundaryCondition( pos ) )
    {
        case QwtSpline::Clamped1:
        {
            const double mt = ( pos == QwtSpline::AtBeginning ) ? m.first() : m.last();
            ok = fuzzyCompare( mt, boundaryValue( pos ) );

            break;
        }
        case QwtSpline::Clamped2:
        {
            const double cvt = ( pos == QwtSpline::AtBeginning ) ? cv.first() : cv.last();
            ok = fuzzyCompare( cvt, boundaryValue( pos ) );

            break;
        }
        case QwtSpline::Clamped3:
        {
            double c3;
            if ( pos == QwtSpline::AtBeginning )
                c3 = polynomialCV( 0, points, cv ).c3;
            else
                c3 = polynomial( points.size() - 2, points, m ).c3;

            ok = fuzzyCompare( 6.0 * c3, boundaryValue( pos ) );
            break;
        }
        case QwtSpline::LinearRunout:
        {
            const double ratio = boundaryValue( pos );
            if ( pos == QwtSpline::AtBeginning )
            {
                const double s = ( points[1].y() - points[0].y() ) /
                    ( points[1].x() - points[0].x() );

                ok = fuzzyCompare( m[0], s - ratio * ( s - m[1] ) );
            }
            else
            {
                const int n = points.size();
                const double s = ( points[n-1].y() - points[n-2].y() ) /
                    ( points[n-1].x() - points[n-2].x() );

                ok = fuzzyCompare( m[n-1], s - ratio * ( s - m[n-2] ) );
            }
            break;
        }
        case QwtSpline::CubicRunout:
        {
            if ( pos == QwtSpline::AtBeginning )
            {
                const QwtSplinePolynomial p1 = polynomial( 0, points, m );
                const QwtSplinePolynomial p2 = polynomial( 1, points, m );

                const double b3 = 0.5 * p2.curvatureAt( points[2].x() - points[1].x() );
    
                ok = fuzzyCompare( p1.c2, 2 * p2.c2 - b3 );
            }
            else
            {
                const int n = points.size();

                const QwtSplinePolynomial p1 = polynomial( n - 2, points, m );
                const QwtSplinePolynomial p2 = polynomial( n - 3, points, m );
    
                const double b3 = 0.5 * p1.curvatureAt( points[n-1].x() - points[n-2].x() );
    
                ok = fuzzyCompare( b3, 2 * p1.c2 - p2.c2 );
            }
            break;
        }
        case QwtSpline::NotAKnot:
        {
            if ( pos == QwtSpline::AtBeginning )
            {
                const QwtSplinePolynomial p1 = polynomial( 0, points, m );
                const QwtSplinePolynomial p2 = polynomial( 1, points, m );
    
                ok = fuzzyCompare( p1.c3, p2.c3 );
            }
            else
            {
                const int n = points.size();

                const QwtSplinePolynomial p1 = polynomial( n - 2, points, m );
                const QwtSplinePolynomial p2 = polynomial( n - 3, points, m );

                ok = fuzzyCompare( p1.c3, p2.c3 );
            }
            break;
        }
    }

    return ok;
}

int CubicSpline::verifyNodesCV( const QPolygonF &p, const QVector<double> &cv ) const
{
    const int size = p.size();

    int numErrors = 0;

    double h1 = p[1].x() - p[0].x();
    double s1 = ( p[1].y() - p[0].y() ) / h1;

    for ( int i = 1; i < size - 2; i++ )
    {
        const double h2 = p[i+1].x() - p[i].x();
        const double s2 = ( p[i+1].y() - p[i].y() ) / h2;

        const double v = ( h1 + h2 ) * cv[i] + 0.5 * ( h1 * cv[i-1] + h2 * cv[i+1] );
        if ( !fuzzyCompare( v, 3 * ( s2 - s1 ) ) )
        {
#if DEBUG_ERRORS > 1
            qDebug() << "invalid node condition (cv)" << i 
                << cv[i-1] << cv[i] << cv[i+1] 
                << v - 3 * ( s2 - s1 );
#endif

            numErrors++;
        }

        h1 = h2;
        s1 = s2;
    }

    return numErrors;
}

int CubicSpline::verifyNodesM( const QPolygonF &p, const QVector<double> &m ) const
{
    const int size = p.size();

    int numErrors = 0;

    double dx1 = p[1].x() - p[0].x();
    QwtSplinePolynomial polynomial1 = QwtSplinePolynomial::fromSlopes( 
        dx1, p[1].y() - p[0].y(), m[0], m[1] );

    for ( int i = 1; i < size - 1; i++ )
    {
        const double dx2 = p[i+1].x() - p[i].x();
        const double dy2 = p[i+1].y() - p[i].y();

        const QwtSplinePolynomial polynomial2 = 
            QwtSplinePolynomial::fromSlopes( dx2, dy2, m[i], m[i+1] );

        const double cv1 = polynomial1.curvatureAt( dx1 );
        const double cv2 = polynomial2.curvatureAt( 0.0 );
        if ( !fuzzyCompare( cv1, cv2 ) )
        {
#if DEBUG_ERRORS > 1
            qDebug() << "invalid node condition (m)" << i << 
                cv1 << cv1 << cv2 - cv1;
#endif

            numErrors++;
        }

        dx1 = dx2;
        polynomial1 = polynomial2;
    }

    return numErrors;
}


QwtSplinePolynomial CubicSpline::polynomial( int index, 
    const QPolygonF &points, const QVector<double> &m ) const
{
    return QwtSplinePolynomial::fromSlopes( 
        points[index], m[index], points[index+1], m[index+1] );
}

QwtSplinePolynomial CubicSpline::polynomialCV( int index,
    const QPolygonF &points, const QVector<double> &cv ) const
{
    return QwtSplinePolynomial::fromCurvatures(
        points[index], cv[index], points[index+1], cv[index+1] );
}

void testSplinesC2( const QPolygonF &points )
{
    QVector<CubicSpline *> splines;

    CubicSpline* spline = new CubicSpline( "Periodic Spline" );
    spline->setBoundaryType( QwtSplineCubic::PeriodicPolygon );
    splines += spline;

    spline = new CubicSpline( "Clamped1 Spline" );
    spline->setBoundaryCondition( QwtSpline::AtBeginning, QwtSpline::Clamped1 );
    spline->setBoundaryValue( QwtSpline::AtBeginning, 0.5 );
    spline->setBoundaryCondition( QwtSpline::AtEnd, QwtSpline::Clamped1 );
    spline->setBoundaryValue( QwtSpline::AtEnd, 2.0 );
    splines += spline;

    spline = new CubicSpline( "Clamped2 Spline" );
    spline->setBoundaryCondition( QwtSpline::AtBeginning, QwtSpline::Clamped2 );
    spline->setBoundaryValue( QwtSpline::AtBeginning, 0.4 );
    spline->setBoundaryCondition( QwtSpline::AtEnd, QwtSpline::Clamped2 );
    spline->setBoundaryValue( QwtSpline::AtEnd, -0.8 );
    splines += spline;

    spline = new CubicSpline( "Clamped3 Spline" );
    spline->setBoundaryCondition( QwtSpline::AtBeginning, QwtSpline::Clamped3 );
    spline->setBoundaryValue( QwtSpline::AtBeginning, 0.03 );
    spline->setBoundaryCondition( QwtSpline::AtEnd, QwtSpline::Clamped3 );
    spline->setBoundaryValue( QwtSpline::AtEnd, 0.01 );
    splines += spline;

    spline = new CubicSpline( "Linear Runout Spline" );
    spline->setBoundaryCondition( QwtSpline::AtBeginning, QwtSplineCubic::LinearRunout );
    spline->setBoundaryValue( QwtSpline::AtBeginning, 0.3 );
    spline->setBoundaryCondition( QwtSpline::AtEnd, QwtSplineCubic::LinearRunout );
    spline->setBoundaryValue( QwtSpline::AtBeginning, 0.7 );
    splines += spline;

    spline = new CubicSpline( "Cubic Runout Spline" );
    spline->setBoundaryCondition( QwtSpline::AtBeginning, QwtSplineCubic::CubicRunout );
    spline->setBoundaryCondition( QwtSpline::AtEnd, QwtSplineCubic::CubicRunout );
    splines += spline;

    spline = new CubicSpline( "Not A Knot Spline", 4 );
    spline->setBoundaryCondition( QwtSpline::AtBeginning, QwtSplineCubic::NotAKnot );
    spline->setBoundaryCondition( QwtSpline::AtEnd, QwtSplineCubic::NotAKnot );
    splines += spline;
    
    for ( int i = 0; i < splines.size(); i++ )
    {
        if ( splines[i]->minPoints() <= points.size() )
        {
            splines[i]->testSpline( points );
            delete splines[i];
        }
    }
}

int main()
{
    QPolygonF points;

    // 3 points

    points << QPointF( 10, 50 ) << QPointF( 60, 30 ) << QPointF( 82, 50 );

    testSplinesC2( points );

    // 4 points

    points.clear();
    points << QPointF( 10, 50 ) << QPointF( 60, 30 )
        << QPointF( 70, 5 ) << QPointF( 82, 50 );

    testSplinesC2( points );

    // 5 points
    points.clear();
    points << QPointF( 10, 50 ) << QPointF( 20, 20 ) << QPointF( 60, 30 )
        << QPointF( 70, 5 ) << QPointF( 82, 50 );

    testSplinesC2( points );

    // 12 points

    points.clear();
    points << QPointF( 10, 50 ) << QPointF( 20, 90 ) << QPointF( 25, 60 )
        << QPointF( 35, 38 ) << QPointF( 42, 40 ) << QPointF( 55, 60 )
        << QPointF( 60, 50 ) << QPointF( 65, 80 ) << QPointF( 73, 30 )
        << QPointF( 82, 30 ) << QPointF( 87, 40 ) << QPointF( 95, 50 );

    testSplinesC2( points );


    // many points
    points.clear();

    const double x1 = 10.0;
    const double x2 = 1000.0;
    const double y1 = -10000.0;
    const double y2 = 10000.0;

    points += QPointF( x1, y1 );

    const int n = 100;
    const double dx = ( x2 - x1 ) / n;
    const int mod = y2 - y1;
    for ( int i = 1; i < n - 1; i++ )
    {
        const double r = qrand() % mod;
        points += QPointF( x1 + i * dx, y1 + r );
    }
    points += QPointF( x2, y1 );

    testSplinesC2( points );
}
