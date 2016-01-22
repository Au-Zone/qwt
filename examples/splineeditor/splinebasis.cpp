#include "splinebasis.h"
#include <qwt_spline_parametrization.h>

#if 0
static QPolygonF uniformKnots( const QPolygonF& points ) 
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
#endif

static QPainterPath qwtBasisPathUniform( const QPolygonF& points ) 
{
    QPainterPath path;
    path.moveTo( points[0] );

    QPointF cp1 = ( 2.0 * points[0] + points[1] ) / 3.0;

    const int n = points.size();
    for ( int i = 1; i < n - 1; i++ )
    {
        const QPointF cp2 = ( points[i-1] + 2.0 * points[i] ) / 3.0;
        const QPointF cp3 = ( 2.0 * points[i] + points[i+1] ) / 3.0;

        path.cubicTo( cp1, cp2, 0.5 * ( cp2 + cp3 ) );

        cp1 = cp3;
    }

    const QPointF cp2 = ( points[n-2] + 2.0 * points[n-1] ) / 3.0;
    path.cubicTo( cp1, cp2, points[n-1] );

    return path;
}

static QPainterPath qwtBasisPath( 
    const QPolygonF &points, const QwtSplineParametrization *param )
{
    QPainterPath path;
    path.moveTo( points[0] );

    double t0 = param->valueIncrement( points[0], points[1] );
    double t1 = t0;
    double t2 = t0;
    double t012 = t0 + t1 + t2;

    QPointF cp1 = ( ( t1 + t2 ) * points[0] + t0 * points[1] ) / t012;

    const int n = points.size();
    for ( int i = 1; i < n - 1; i++ )
    {
        const double t3 = param->valueIncrement( points[i], points[i+1] );
        const double t123 = t1 + t2 + t3;

        const QPointF cp2 = ( t2 * points[i-1] + ( t0 + t1 ) * points[i] ) / t012;
        const QPointF cp3 = ( ( t2 + t3 ) * points[i] + t1 * points[i+1] ) / t123;

        const QPointF p2 = ( t2 * cp2 + t1 * cp3 ) / ( t1 + t2 );

        path.cubicTo( cp1, cp2, p2 );

        cp1 = cp3;

        t0 = t1;
        t1 = t2;
        t2 = t3;
        t012 = t123;
    }

    const QPointF cp2 = ( t2 * points[n-2] + ( t0 + t1 ) * points[n-1] ) / t012;
    path.cubicTo( cp1, cp2, points[n-1] );

    return path;
}

SplineBasis::SplineBasis()
{
}

SplineBasis::~SplineBasis()
{
}

uint SplineBasis::locality() const
{
    return 2;
}


QPainterPath SplineBasis::painterPath( const QPolygonF &points ) const
{
    if ( points.size()  < 4 )
        return QPainterPath();

    QPainterPath path;

    switch( parametrization()->type() )
    {
        case QwtSplineParametrization::ParameterUniform:
        {
            path = qwtBasisPathUniform( points );
            break;
        }
        default:
        {
            path = qwtBasisPath( points, parametrization() );
        }
    }

    return path;
}
