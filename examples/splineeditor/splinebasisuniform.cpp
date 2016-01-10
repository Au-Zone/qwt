#include "splinebasisuniform.h"

SplineBasisUniform::SplineBasisUniform()
{
}

SplineBasisUniform::~SplineBasisUniform()
{
}

QPainterPath SplineBasisUniform::painterPath( const QPolygonF &points ) const
{
    return toBezierUniform( points );
}

QPolygonF SplineBasisUniform::interpolatingKnots( const QPolygonF& points ) const
{
    const int n = points.size();

    if ( n < 3 )
        return points;

    QVector<double> u( n - 2 );
    QVector<double> kx( n - 2 );
    QVector<double> ky( n - 2 );

    u[n-3] = 4.0;
    kx[n-3] = 6.0 * points[n-2].x() - points[n-1].x();
    ky[n-3] = 6.0 * points[n-2].y() - points[n-1].y();

    for ( int i = n - 4; i >= 0; i-- )
    {
        u[i] = 4.0 - 1.0 / u[i+1];
        kx[i] = 6.0 * points[i+1].x() - kx[i+1] / u[i+1];
        ky[i] = 6.0 * points[i+1].y() - ky[i+1] / u[i+1];
    }

    QVector<QPointF> knots( n );

    knots[0] = points[0];

    for ( int i = 1; i < n - 1; i++ )
    {
        knots[i].rx() = ( kx[i-1] - knots[i-1].x() ) / u[i-1];
        knots[i].ry() = ( ky[i-1] - knots[i-1].y() ) / u[i-1];
    }

    knots[n-1] = points[n-1];

    return knots;
}

QPainterPath SplineBasisUniform::toBezierUniform( const QPolygonF& knots ) const
{
    const int n = knots.size();

    QPainterPath path;
    path.moveTo( knots[0] );

    QPointF cp1 = ( 2.0 * knots[0] + knots[1] ) / 3.0;

    for ( int i = 1; i < n - 1; i++ )
    {
        const QPointF cp2 = ( knots[i-1] + 2.0 * knots[i] ) / 3.0;
        const QPointF cp3 = ( 2.0 * knots[i] + knots[i+1] ) / 3.0;

        // the same as ( 0.5 * ( knots[i-1] + knots[i+1] ) + 2.0 * knots[i] ) / 3.0;
        const QPointF p2 = 0.5 * ( cp2 + cp3 );

        path.cubicTo( cp1, cp2, p2 );

        cp1 = cp3;
    }

    const QPointF cp2 = ( knots[n-2] + 2.0 * knots[n-1] ) / 3.0;
    path.cubicTo( cp1, cp2, knots[n-1] );

    return path;
}


QPainterPath SplineBasisUniform::toBezierSlopes( const QPolygonF& knots ) const
{
    const double dt = 1.0;

    const int n = knots.size();

    QPainterPath path;
    path.moveTo( knots[0] );

    double mx1 = ( knots[1].x() - knots[0].x() ) / dt;
    double my1 = ( knots[1].y() - knots[0].y() ) / dt;
    
    QPointF p1 = knots[0];

    for ( int i = 1; i < n - 1; i++ )
    {
        const QPointF p2 = ( 0.5 * ( knots[i-1] + knots[i+1] ) + 2.0 * knots[i] ) / 3.0;
        const double mx2 = ( 2.0 * knots[i].x() + knots[i+1].x() - 3.0 * p2.x() ) / dt;
        const double my2 = ( 2.0 * knots[i].y() + knots[i+1].y() - 3.0 * p2.y() ) / dt;

        const double dt3 = dt / 3.0;

        path.cubicTo( 
            p1.x() + mx1 * dt3, p1.y() + my1 * dt3,
            p2.x() - mx2 * dt3, p2.y() - my2 * dt3,
            p2.x(), p2.y() );

        p1 = p2;
        mx1 = mx2;
        my1 = my2;
    }

    {
        const QPointF &p2 = knots[n-1];
        const double mx2 = ( knots[n-1].x() - knots[n-2].x() ) / dt;
        const double my2 = ( knots[n-1].y() - knots[n-2].y() ) / dt;

        const double dt3 = dt / 3.0;

        path.cubicTo( 
            p1.x() + mx1 * dt3, p1.y() + my1 * dt3,
            p2.x() - mx2 * dt3, p2.y() - my2 * dt3,
            p2.x(), p2.y() );
    }

    return path;
}

QPainterPath SplineBasisUniform::painterPathInterpolated( const QPolygonF &points ) const
{
    const int n = points.size();
    if (n <= 1)
        return QPainterPath();

    const QPolygonF knots = interpolatingKnots( points );
    return toBezierUniform( knots );
}


