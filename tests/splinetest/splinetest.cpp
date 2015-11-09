#include <qwt_spline_cubic.h>
#include <qpolygon.h>
#include <qdebug.h>

#define DEBUG_ERRORS 1

#if QT_VERSION < 0x040600

static inline bool qFuzzyIsNull(double d)
{
    return qAbs(d) <= 0.000000000001;
}

static inline bool qFuzzyIsNull(float f)
{
    return qAbs(f) <= 0.00001f;
}

#endif

class CubicSpline: public QwtSplineCubic
{
public:
    CubicSpline( const char* name, int minPoints = 3 ):
        d_name( name ),
        d_minPoints( minPoints )
    {
    }

    virtual ~CubicSpline()
    {
    }

    int minPoints() const
    {
        return d_minPoints;
    }

    void testSpline( const QPolygonF &points ) const
    {
        const QVector<double> m = slopes( points );
        const QVector<double> cv = curvatures( points );

        if ( m.size() != points.size() )
        {
            qDebug() << qPrintable( d_name ) << "("
                << points.size() << "):" << "not implemented";
            return;
        }

        const bool okStart = verifyStart( points, m, cv );
        const int numErrorsM = verifyNodesM( points, m );
        const int numErrorsCV = verifyNodesCV( points, cv );
        const bool okEnd = verifyEnd( points, m, cv );

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
    
protected:

    double sum3( double v1, double v2, double v3 ) const
    {
        // Kahan
        QVector<double> v;
        v += v1;
        v += v2;
        v += v3;

        double sum = v[0];
        double c = 0.0;

        for (int i = 1; i < v.size(); i++ ) 
        {
            const double y = v[i] - c;
            const double t = sum + y;
            c = ( t - sum ) - y;
            sum = t;
        }

        return sum;
    }


    virtual bool verifyStart( const QPolygonF &, 
        const QVector<double> &, const QVector<double> & ) const = 0;

    virtual bool verifyEnd( const QPolygonF &, 
        const QVector<double> &, const QVector<double> & ) const = 0;

    int verifyNodesCV( const QPolygonF &p, const QVector<double> &cv ) const
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

    int verifyNodesM( const QPolygonF &p, const QVector<double> &m ) const
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

    inline bool fuzzyCompare( double a, double b ) const
    {
        return ( qFuzzyIsNull(a) && qFuzzyIsNull(b) ) || qFuzzyCompare(a, b);
    }

    QwtSplinePolynomial polynomial( int index, 
        const QPolygonF &points, const QVector<double> &m ) const
    {
        return QwtSplinePolynomial::fromSlopes( 
            points[index], m[index], points[index+1], m[index+1] );
    }

    QwtSplinePolynomial polynomialCV( int index,
        const QPolygonF &points, const QVector<double> &cv ) const
    {
        return QwtSplinePolynomial::fromCurvatures(
            points[index], cv[index], points[index+1], cv[index+1] );
    }

private:
    const QString d_name;
    const int d_minPoints;
};

class SplineLinearRunout: public CubicSpline
{
public:
    SplineLinearRunout( double ratioBegin, double ratioEnd ):
        CubicSpline( "Linear Runout Spline" )
    {
        ratioBegin = qBound( 0.0, ratioBegin, 1.0 );
        ratioEnd = qBound( 0.0, ratioEnd, 1.0 );

        setBoundaryCondition( QwtSpline::AtBeginning, QwtSplineCubic::LinearRunout );
        setBoundaryValue( QwtSpline::AtBeginning, ratioBegin );

        setBoundaryCondition( QwtSpline::AtEnd, QwtSplineCubic::LinearRunout );
        setBoundaryValue( QwtSpline::AtEnd, ratioEnd );
    }

protected:
    virtual bool verifyStart( const QPolygonF &points, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        const double s = ( points[1].y() - points[0].y() ) / 
            ( points[1].x() - points[0].x() );

		const double ratio = boundaryValue( QwtSpline::AtBeginning );

        return fuzzyCompare( m[0], s - ratio * ( s - m[1] ) );
    }
    
    virtual bool verifyEnd( const QPolygonF &points,
        const QVector<double> &m, const QVector<double> & ) const
    {
        const int n = points.size();
        const double s = ( points[n-1].y() - points[n-2].y() ) / 
            ( points[n-1].x() - points[n-2].x() );

        const double ratio = boundaryValue( QwtSpline::AtEnd );

        return fuzzyCompare( m[n-1], s - ratio * ( s - m[n-2] ) );
    }
};

class SplineCubicRunout: public CubicSpline
{
public:
    SplineCubicRunout():
        CubicSpline( "Cubic Runout Spline" )
    {
        setBoundaryCondition( QwtSpline::AtBeginning, QwtSplineCubic::CubicRunout );
        setBoundaryCondition( QwtSpline::AtEnd, QwtSplineCubic::CubicRunout );
    }

protected:
    virtual bool verifyStart( const QPolygonF &points, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        const QwtSplinePolynomial p1 = polynomial( 0, points, m );
        const QwtSplinePolynomial p2 = polynomial( 1, points, m );

        const double b3 = 0.5 * p2.curvatureAt( points[2].x() - points[1].x() );

        return fuzzyCompare( p1.c2, 2 * p2.c2 - b3 );
    }
    
    virtual bool verifyEnd( const QPolygonF &points,
        const QVector<double> &m, const QVector<double> & ) const
    {
        const int n = points.size();

        const QwtSplinePolynomial p1 = polynomial( n - 2, points, m );
        const QwtSplinePolynomial p2 = polynomial( n - 3, points, m );

        const double b3 = 0.5 * p1.curvatureAt( points[n-1].x() - points[n-2].x() );

        return fuzzyCompare( b3, 2 * p1.c2 - p2.c2 );
    }
};

class SplineNotAKnot: public CubicSpline
{   
public:
    SplineNotAKnot():
        CubicSpline( "Not A Knot Spline", 4 )
    {
        setBoundaryCondition( QwtSpline::AtBeginning, QwtSplineCubic::NotAKnot );
        setBoundaryCondition( QwtSpline::AtEnd, QwtSplineCubic::NotAKnot );
    }

protected:
    virtual bool verifyStart( const QPolygonF &points, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        const QwtSplinePolynomial p1 = polynomial( 0, points, m );
        const QwtSplinePolynomial p2 = polynomial( 1, points, m );

        return fuzzyCompare( p1.c3, p2.c3 );
    }
    
    virtual bool verifyEnd( const QPolygonF &points, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        const int n = points.size();

        const QwtSplinePolynomial p1 = polynomial( n - 2, points, m );
        const QwtSplinePolynomial p2 = polynomial( n - 3, points, m );

        return fuzzyCompare( p1.c3, p2.c3 );
    }
};

class SplinePeriodic: public CubicSpline
{   
public:
    SplinePeriodic():
        CubicSpline( "Periodic Spline" )
    {
        setBoundaryType( QwtSplineCubic::PeriodicPolygon );
    }

protected:
    virtual bool verifyStart( const QPolygonF &points, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        const int n = points.size();

        const QwtSplinePolynomial p1 = polynomial( n - 2, points, m );
        const QwtSplinePolynomial p2 = polynomial( 0, points, m );

        const double dx = points[n-1].x() - points[n-2].x();
        return( fuzzyCompare( p1.curvatureAt( dx ), p2.curvatureAt( 0.0 ) ) &&
            fuzzyCompare( p1.slopeAt( dx ), p2.slopeAt( 0.0 ) ) );
    }
    
    virtual bool verifyEnd( const QPolygonF &points, 
        const QVector<double> &m, const QVector<double> &cv ) const
    {
        return verifyStart( points, m, cv );
    }
};

class SplineClamped1: public CubicSpline
{   
public:
    SplineClamped1( double slopeBegin, double slopeEnd ):
        CubicSpline( "Clamped Spline" )
    {
        setBoundaryCondition( QwtSpline::AtBeginning, QwtSpline::Clamped1 );
        setBoundaryValue( QwtSpline::AtBeginning, slopeBegin );

        setBoundaryCondition( QwtSpline::AtEnd, QwtSpline::Clamped1 );
        setBoundaryValue( QwtSpline::AtEnd, slopeEnd );
    }

protected:
    
    virtual bool verifyStart( const QPolygonF &, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        return fuzzyCompare( m.first(), boundaryValue( QwtSpline::AtBeginning ) );
    }
    
    virtual bool verifyEnd( const QPolygonF &, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        return fuzzyCompare( m.last(), boundaryValue( QwtSpline::AtEnd ) );
    }
};

class SplineClamped2: public CubicSpline
{   
public:
    SplineClamped2( double cvBegin, double cvEnd ):
        CubicSpline( "Clamped2 Spline" )
    {
        setBoundaryCondition( QwtSpline::AtBeginning, QwtSpline::Clamped2 );
        setBoundaryValue( QwtSpline::AtBeginning, cvBegin );

        setBoundaryCondition( QwtSpline::AtEnd, QwtSpline::Clamped2 );
        setBoundaryValue( QwtSpline::AtEnd, cvEnd );
    }

protected:
    virtual bool verifyStart( const QPolygonF &, 
        const QVector<double> &, const QVector<double> &cv ) const
    {
        return fuzzyCompare( cv.first(), boundaryValue( QwtSpline::AtBeginning ) );
    }

    virtual bool verifyEnd( const QPolygonF &,
        const QVector<double> &, const QVector<double> &cv ) const
    {
        return fuzzyCompare( cv.last(), boundaryValue( QwtSpline::AtEnd ) );
    }
};

class SplineClamped3: public CubicSpline
{   
public:
    SplineClamped3( double valueBegin, double valueEnd ):
        CubicSpline( "Clamped3 Spline" )
    {
        setBoundaryCondition( QwtSpline::AtBeginning, QwtSpline::Clamped3 );
        setBoundaryValue( QwtSpline::AtBeginning, valueBegin );

        setBoundaryCondition( QwtSpline::AtEnd, QwtSpline::Clamped3 );
        setBoundaryValue( QwtSpline::AtEnd, valueEnd );
    }   

protected:
    virtual bool verifyStart( const QPolygonF &points, 
        const QVector<double> &, const QVector<double> &cv ) const
    {
        const QwtSplinePolynomial p = polynomialCV( 0, points, cv );
        return fuzzyCompare( 6.0 * p.c3, boundaryValue( QwtSpline::AtBeginning ) );
    }
    
    virtual bool verifyEnd( const QPolygonF &points, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        const QwtSplinePolynomial p = polynomial( points.size() - 2, points, m );
        return fuzzyCompare( 6.0 * p.c3, boundaryValue( QwtSpline::AtEnd ) );
    }
};

void testSplines( QVector<CubicSpline *> splines, const QPolygonF &points )
{
    for ( int i = 0; i < splines.size(); i++ )
    {
        if ( splines[i]->minPoints() <= points.size() )
            splines[i]->testSpline( points );
    }
}

int main()
{
    QVector<CubicSpline *> splines;
    splines += new SplineLinearRunout( 0.3, 0.7 );
    splines += new SplineCubicRunout();
    splines += new SplineNotAKnot();
    splines += new SplinePeriodic();
    splines += new SplineClamped1( 0.5, 2.0 );
    splines += new SplineClamped2( 0.4, -0.8 );
    splines += new SplineClamped3( 0.03, 0.01 );
    
    QPolygonF points;

    // 3 points

    points << QPointF( 10, 50 ) << QPointF( 60, 30 ) << QPointF( 82, 50 );

    testSplines( splines, points );

    // 4 points

    points.clear();
    points << QPointF( 10, 50 ) << QPointF( 60, 30 )
        << QPointF( 70, 5 ) << QPointF( 82, 50 );

    testSplines( splines, points );

    // 5 points
    points.clear();
    points << QPointF( 10, 50 ) << QPointF( 20, 20 ) << QPointF( 60, 30 )
        << QPointF( 70, 5 ) << QPointF( 82, 50 );

    testSplines( splines, points );

    // 12 points

    points.clear();
    points << QPointF( 10, 50 ) << QPointF( 20, 90 ) << QPointF( 25, 60 )
        << QPointF( 35, 38 ) << QPointF( 42, 40 ) << QPointF( 55, 60 )
        << QPointF( 60, 50 ) << QPointF( 65, 80 ) << QPointF( 73, 30 )
        << QPointF( 82, 30 ) << QPointF( 87, 40 ) << QPointF( 95, 50 );

    testSplines( splines, points );

    points.clear();

#if 1
    // many points

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

    testSplines( splines, points );
    points.clear();

#endif

    for ( int i = 0; i < splines.size(); i++ )
        delete splines[i];
}
