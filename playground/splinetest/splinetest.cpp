#include <qwt_spline_cubic.h>
#include <qpolygon.h>
#include <qdebug.h>

#define DEBUG_ERRORS 0

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
        if ( m.size() != points.size() )
        {
            qDebug() << qPrintable( d_name ) << "("
                << points.size() << "):" << "not implemented";
            return;
        }

        bool ok = true;

        if ( !verifyStart( points, m ) )
        {
#if DEBUG_ERRORS > 0
            qDebug() << "invalid start condition";
#endif
            ok = false;
        }

        if ( !verifyNodes( points, m ) )
        {
#if DEBUG_ERRORS > 0
            qDebug() << "invalid node conditions";
#endif
            ok = false;
        }

        if ( !verifyEnd( points, m ) )
        {
#if DEBUG_ERRORS > 0
            qDebug() << "invalid end condition";
#endif
            ok = false;
        }

        if ( !ok )
        {
            qDebug() << qPrintable( d_name ) << "("
                << points.size() << "):" << ok;
        }
    }
    
protected:
    virtual bool verifyStart( const QPolygonF &, const QVector<double> & ) const = 0;
    virtual bool verifyEnd( const QPolygonF &, const QVector<double> & ) const = 0;

    bool verifyNodes( const QPolygonF &p, const QVector<double> &m ) const
    {
        const int size = p.size();

        bool ok = true;

        for ( int i = 1; i < size - 1; i++ )
        {
            double a1, b1, c1;
            double a2, b2, c2;

            toPolynom( p[i-1], m[i-1], p[i], m[i], a1, b1, c1 );
            toPolynom( p[i], m[i], p[i+1], m[i+1], a2, b2, c2 );

            const double bX = 3.0 * a1 * ( p[i].x() - p[i-1].x() ) + b1;
            if ( !fuzzyCompare( bX, b2 ) )
            {
#if DEBUG_ERRORS > 1
                qDebug() << "invalid node condition" << i << 
                    bX << b2 << bX - b2;
#endif

                ok = false;
            }
        }

        return ok;
    }

    void toPolynom( const QPointF &p1, double m1,
        const QPointF &p2, double m2, double &a, double &b, double &c ) const
    {
        const double dx = p2.x() - p1.x();
        const double slope = ( p2.y() - p1.y() ) / dx;

        c = m1;
        b = ( 3.0 * slope - 2 * m1 - m2 ) / dx;
        a = ( ( m2 - m1 ) / dx - 2.0 * b ) / ( 3.0 * dx );
    }

    inline bool fuzzyCompare( double a, double b ) const
    {
        return ( qFuzzyIsNull(a) && qFuzzyIsNull(b) ) || qFuzzyCompare(a, b);
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
        setEndConditions( QwtSplineCubic::ParabolicRunout );
    }

protected:

    virtual bool verifyStart( const QPolygonF &points, const QVector<double> &m ) const
    {
        double a, b, c;
        toPolynom( points[0], m[0], points[1], m[1], a, b, c );

        return fuzzyCompare( a, 0.0 );
    }

    virtual bool verifyEnd( const QPolygonF &points, const QVector<double> &m ) const
    {
        const int n = points.size();

        double a, b, c;
        toPolynom( points[n-2], m[n-2], points[n-1], m[n-1], a, b, c );

        return fuzzyCompare( a, 0.0 );
    }
};

class SplineCubicRunout: public CubicSpline
{
public:
    SplineCubicRunout():
        CubicSpline( "Cubic Runout Spline" )
    {
        setEndConditions( QwtSplineCubic::CubicRunout );
    }

protected:
    virtual bool verifyStart( const QPolygonF &points, const QVector<double> &m ) const
    {
        double a1, b1, c1;
        toPolynom( points[0], m[0], points[1], m[1], a1, b1, c1 );

        double a2, b2, c2;
        toPolynom( points[1], m[1], points[2], m[2], a2, b2, c2 );

        const double b3 = 3 * a2 * ( points[2].x() - points[1].x() ) + b2;

        return fuzzyCompare( b1, 2 * b2 - b3 );
    }
    
    virtual bool verifyEnd( const QPolygonF &points, const QVector<double> &m ) const
    {
        const int n = points.size();

        double a1, b1, c1;
        toPolynom( points[n-2], m[n-2], points[n-1], m[n-1], a1, b1, c1 );

        double a2, b2, c2;
        toPolynom( points[n-3], m[n-3], points[n-2], m[n-2], a2, b2, c2 );

        const double b3 = 3 * a1 * ( points[n-1].x() - points[n-2].x() ) + b1;

        return fuzzyCompare( b3, 2 * b1 - b2 );
    }
};

class SplineNotAKnot: public CubicSpline
{   
public:
    SplineNotAKnot():
        CubicSpline( "Not A Knot Spline", 4 )
    {
        setEndConditions( QwtSplineCubic::NotAKnot );
    }

protected:
    virtual bool verifyStart( const QPolygonF &points, const QVector<double> &m ) const
    {
        double a1, a2, b, c;
        toPolynom( points[0], m[0], points[1], m[1], a1, b, c );
        toPolynom( points[1], m[1], points[2], m[2], a2, b, c );

        return fuzzyCompare( a1, a2 );
    }
    
    virtual bool verifyEnd( const QPolygonF &points, const QVector<double> &m ) const
    {
        const int n = points.size();

        double a1, a2, b, c;
        toPolynom( points[n-3], m[n-3], points[n-2], m[n-2], a1, b, c );
        toPolynom( points[n-2], m[n-2], points[n-1], m[n-1], a2, b, c );

        return fuzzyCompare( a1, a2 );
    }
};

class SplinePeriodic: public CubicSpline
{   
public:
    SplinePeriodic():
        CubicSpline( "Periodic Spline" )
    {
        setEndConditions( QwtSplineCubic::Periodic );
    }

protected:
    virtual bool verifyStart( const QPolygonF &points, const QVector<double> &m ) const
    {
        const int n = points.size();

        double a1, b1, c1;
        toPolynom( points[n-2], m[n-2], points[n-1], m[n-1], a1, b1, c1 );

        double a2, b2, c2;
        toPolynom( points[0], m[0], points[1], m[1], a2, b2, c2 );

        const double dx = points[n-1].x() - points[n-2].x();
        return( fuzzyCompare( 6.0 * a1 * dx + 2 * b1, 2 * b2 ) &&
            fuzzyCompare( 3 * a1 * dx * dx + 2 * b1 * dx + c1, m[0] ) );
    }
    
    virtual bool verifyEnd( const QPolygonF &points, const QVector<double> &m ) const
    {
        return verifyStart( points, m );
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
        setClamped( slopeBegin, slopeEnd );
    }

protected:
    
    virtual bool verifyStart( const QPolygonF &, const QVector<double> &m ) const
    {
        return fuzzyCompare( m.first(), d_slopeBegin );
    }
    
    virtual bool verifyEnd( const QPolygonF &, const QVector<double> &m ) const
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
        setClamped2( cvBegin, cvEnd );
    }

protected:
    virtual bool verifyStart( const QPolygonF &points, const QVector<double> &m ) const
    {
        const double dx = points[1].x() - points[0].x();
        const double dy = points[1].y() - points[0].y();

        const double cv = 2 * ( 3 * dy / dx - 2 * m[0] - m[1] ) / dx;
        return fuzzyCompare( d_cvBegin, cv );
    }

    virtual bool verifyEnd( const QPolygonF &points, const QVector<double> &m ) const
    {
        const int n = points.size();

        const double dx = points[n-1].x() - points[n-2].x();
        const double dy = points[n-1].y() - points[n-2].y();

        const double cv = 2 * ( -3 * dy / dx + m[n-2] + 2 * m[n-1] ) / dx;
        return fuzzyCompare( d_cvEnd, cv );
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
        setClamped3( valueBegin, valueEnd );
    }   

protected:
    virtual bool verifyStart( const QPolygonF &points, const QVector<double> &m ) const
    {
        const double dx = points[1].x() - points[0].x();
        const double dy = points[1].y() - points[0].y();

        const double slope = dy / dx;

        const double cv1 = 2 * ( 3 * slope - 2 * m[0] - m[1] ) / dx;
        const double cv2 = 2 * ( -3 * slope + m[0] + 2 * m[1] ) / dx;

        return fuzzyCompare( d_valueBegin, ( cv2 - cv1 ) / dx );
    }
    
    virtual bool verifyEnd( const QPolygonF &points, const QVector<double> &m ) const
    {
        const int n = points.size();

        const double dx = points[n-1].x() - points[n-2].x();
        const double dy = points[n-1].y() - points[n-2].y();

        const double slope = dy / dx;

        const double cv1 = 2 * ( 3 * slope - 2 * m[n-2] - m[n-1] ) / dx;
        const double cv2 = 2 * ( -3 * slope + m[n-2] + 2 * m[n-1] ) / dx;

        return fuzzyCompare( d_valueEnd, ( cv2 - cv1 ) / dx );
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

#if 0
    // 100 points

    const double x1 = 10.0;
    const double x2 = 1000.0;
    const double y1 = -10000.0;
    const double y2 = 10000.0;

    points += QPointF( x1, y1 );

    const int n = 100;
    const double dx = ( x2 - x1 ) / n;
    const int mod = y2 - y1;
    for ( int i = 1; i < n; i++ )
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
