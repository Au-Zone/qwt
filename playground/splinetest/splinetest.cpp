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
        const QVector<double> m = slopesX( points );
        const QVector<double> cv = curvaturesX( points );

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
        QwtSplinePolynom polynom1 = QwtSplinePolynom::fromSlopes( 
            dx1, p[1].y() - p[0].y(), m[0], m[1] );

        for ( int i = 1; i < size - 1; i++ )
        {
            const double dx2 = p[i+1].x() - p[i].x();
            const double dy2 = p[i+1].y() - p[i].y();

            const QwtSplinePolynom polynom2 = 
                QwtSplinePolynom::fromSlopes( dx2, dy2, m[i], m[i+1] );

            const double cv1 = polynom1.curvature( dx1 );
            const double cv2 = polynom2.curvature( 0.0 );
            if ( !fuzzyCompare( cv1, cv2 ) )
            {
#if DEBUG_ERRORS > 1
                qDebug() << "invalid node condition (m)" << i << 
                    cv1 << cv1 << cv2 - cv1;
#endif

                numErrors++;
            }

            dx1 = dx2;
            polynom1 = polynom2;
        }

        return numErrors;
    }

    inline bool fuzzyCompare( double a, double b ) const
    {
        return ( qFuzzyIsNull(a) && qFuzzyIsNull(b) ) || qFuzzyCompare(a, b);
    }

    QwtSplinePolynom polynom( int index, 
        const QPolygonF &points, const QVector<double> &m ) const
    {
        return QwtSplinePolynom::fromSlopes( 
            points[index], m[index], points[index+1], m[index+1] );
    }

    QwtSplinePolynom polynomCV( int index,
        const QPolygonF &points, const QVector<double> &cv ) const
    {
        return QwtSplinePolynom::fromCurvatures(
            points[index], cv[index], points[index+1], cv[index+1] );
    }

private:
    const QString d_name;
    const int d_minPoints;
};

class SplineParabolicRunout: public CubicSpline
{
public:
    SplineParabolicRunout():
        CubicSpline( "Parabolic Runout Spline" )
    {
        setBoundaryConditions( QwtSplineCubic::ParabolicRunout );
    }

protected:

    virtual bool verifyStart( const QPolygonF &points, 
        const QVector<double> &, const QVector<double> &cv ) const
    {
        const QwtSplinePolynom p = polynomCV( 0, points, cv );
        return fuzzyCompare( p.c3, 0.0 );
    }

    virtual bool verifyEnd( const QPolygonF &points, 
        const QVector<double> &, const QVector<double> &cv ) const
    {
        const QwtSplinePolynom p = polynomCV( points.size() - 2, points, cv );
        return fuzzyCompare( p.c3, 0.0 );
    }
};

class SplineCubicRunout: public CubicSpline
{
public:
    SplineCubicRunout():
        CubicSpline( "Cubic Runout Spline" )
    {
        setBoundaryConditions( QwtSplineCubic::CubicRunout );
    }

protected:
    virtual bool verifyStart( const QPolygonF &points, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        const QwtSplinePolynom p1 = polynom( 0, points, m );
        const QwtSplinePolynom p2 = polynom( 1, points, m );

        const double b3 = 0.5 * p2.curvature( points[2].x() - points[1].x() );

        return fuzzyCompare( p1.c2, 2 * p2.c2 - b3 );
    }
    
    virtual bool verifyEnd( const QPolygonF &points,
        const QVector<double> &m, const QVector<double> & ) const
    {
        const int n = points.size();

        const QwtSplinePolynom p1 = polynom( n - 2, points, m );
        const QwtSplinePolynom p2 = polynom( n - 3, points, m );

        const double b3 = 0.5 * p1.curvature( points[n-1].x() - points[n-2].x() );

        return fuzzyCompare( b3, 2 * p1.c2 - p2.c2 );
    }
};

class SplineNotAKnot: public CubicSpline
{   
public:
    SplineNotAKnot():
        CubicSpline( "Not A Knot Spline", 4 )
    {
        setBoundaryConditions( QwtSplineCubic::NotAKnot );
    }

protected:
    virtual bool verifyStart( const QPolygonF &points, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        const QwtSplinePolynom p1 = polynom( 0, points, m );
        const QwtSplinePolynom p2 = polynom( 1, points, m );

        return fuzzyCompare( p1.c3, p2.c3 );
    }
    
    virtual bool verifyEnd( const QPolygonF &points, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        const int n = points.size();

        const QwtSplinePolynom p1 = polynom( n - 2, points, m );
        const QwtSplinePolynom p2 = polynom( n - 3, points, m );

        return fuzzyCompare( p1.c3, p2.c3 );
    }
};

class SplinePeriodic: public CubicSpline
{   
public:
    SplinePeriodic():
        CubicSpline( "Periodic Spline" )
    {
        setBoundaryConditions( QwtSplineCubic::Periodic );
    }

protected:
    virtual bool verifyStart( const QPolygonF &points, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        const int n = points.size();

        const QwtSplinePolynom p1 = polynom( n - 2, points, m );
        const QwtSplinePolynom p2 = polynom( 0, points, m );

        const double dx = points[n-1].x() - points[n-2].x();
        return( fuzzyCompare( p1.curvature( dx ), p2.curvature( 0.0 ) ) &&
            fuzzyCompare( p1.slope( dx ), p2.slope( 0.0 ) ) );
    }
    
    virtual bool verifyEnd( const QPolygonF &points, 
        const QVector<double> &m, const QVector<double> &cv ) const
    {
        return verifyStart( points, m, cv );
    }
};

class SplineClamped: public CubicSpline
{   
public:
    SplineClamped( double slopeBegin, double slopeEnd ):
        CubicSpline( "Clamped Spline" ),
        d_slopeBegin( slopeBegin ),
        d_slopeEnd( slopeEnd )
    {
        setBoundaryConditions( QwtSplineCubic::Clamped );
        setBoundaryValues( slopeBegin, slopeEnd );
    }

protected:
    
    virtual bool verifyStart( const QPolygonF &, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        return fuzzyCompare( m.first(), d_slopeBegin );
    }
    
    virtual bool verifyEnd( const QPolygonF &, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        return fuzzyCompare( m.last(), d_slopeEnd );
    }

private:
    const double d_slopeBegin;
    const double d_slopeEnd;
};

class SplineClamped2: public CubicSpline
{   
public:
    SplineClamped2( double cvBegin, double cvEnd ):
        CubicSpline( "Clamped2 Spline" ),
        d_cvBegin( cvBegin ),
        d_cvEnd( cvEnd )
    {
        setBoundaryConditions( QwtSplineCubic::Clamped2 );
        setBoundaryValues( cvBegin, cvEnd );
    }

protected:
    virtual bool verifyStart( const QPolygonF &, 
        const QVector<double> &, const QVector<double> &cv ) const
    {
        return fuzzyCompare( d_cvBegin, cv.first() );
    }

    virtual bool verifyEnd( const QPolygonF &,
        const QVector<double> &, const QVector<double> &cv ) const
    {
        return fuzzyCompare( d_cvEnd, cv.last() );
    }

private:
    const double d_cvBegin;
    const double d_cvEnd;
};

class SplineClamped3: public CubicSpline
{   
public:
    SplineClamped3( double valueBegin, double valueEnd ):
        CubicSpline( "Clamped3 Spline" ),
        d_valueBegin( valueBegin ),
        d_valueEnd( valueEnd )
    {
        setBoundaryConditions( QwtSplineCubic::Clamped3 );
        setBoundaryValues( valueBegin, valueEnd );
    }   

protected:
    virtual bool verifyStart( const QPolygonF &points, 
        const QVector<double> &, const QVector<double> &cv ) const
    {
        const QwtSplinePolynom p = polynomCV( 0, points, cv );
        return fuzzyCompare( d_valueBegin, 6.0 * p.c3 );
    }
    
    virtual bool verifyEnd( const QPolygonF &points, 
        const QVector<double> &m, const QVector<double> & ) const
    {
        const QwtSplinePolynom p = polynom( points.size() - 2, points, m );
        return fuzzyCompare( d_valueEnd, 6.0 * p.c3 );
    }
    
private:
    const double d_valueBegin;
    const double d_valueEnd;
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
    splines += new SplineParabolicRunout();
    splines += new SplineCubicRunout();
    splines += new SplineNotAKnot();
    splines += new SplinePeriodic();
    splines += new SplineClamped( 0.5, 2.0 );
    splines += new SplineClamped2( 0.4, -0.8 );
    splines += new SplineClamped3( 0.03, 0.01 );
    
    QPolygonF points;

    // 3 points

    points << QPointF( 10, 50 ) << QPointF( 60, 30 ) 
        << QPointF( 82, 50 );

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
        const double r = random() % mod;
        points += QPointF( x1 + i * dx, y1 + r );
    }
    points += QPointF( x2, y1 );

    testSplines( splines, points );
    points.clear();

#endif

    for ( int i = 0; i < splines.size(); i++ )
        delete splines[i];
}
