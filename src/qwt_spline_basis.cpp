#include <qwt_spline_basis.h>
#include <qwt_spline_parametrization.h>
#include <QDebug>

#if 0
static QPolygonF qwtBasisUniformKnots( const QPolygonF& points ) 
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

static QPainterPath qwtSplineBasisPathUniform( const QPolygonF& points, 
    QwtSplineApproximation::BoundaryType boundaryType ) 
{
    const int n = points.size();
    const QPointF *pd = points.constData();

    QPainterPath path;

    QPointF cp1 = ( 2.0 * pd[0] + pd[1] ) / 3.0;;

    if ( boundaryType == QwtSplineApproximation::ConditionalBoundaries )
    {
        path.moveTo( pd[0] );
    }
    else
    {
        const QPointF cpN = ( pd[n-1] + 2.0 * pd[0] ) / 3.0;
        path.moveTo( 0.5 * ( cpN + cp1 ) );
    }

    for ( int i = 1; i < n - 1; i++ )
    {
        const QPointF cp2 = ( pd[i-1] + 2.0 * pd[i] ) / 3.0;
        const QPointF cp3 = ( 2.0 * pd[i] + pd[i+1] ) / 3.0;

        path.cubicTo( cp1, cp2, 0.5 * ( cp2 + cp3 ) );

        cp1 = cp3;
    }

    if ( boundaryType == QwtSplineApproximation::ConditionalBoundaries )
    {
        const QPointF cp2 = ( pd[n-2] + 2.0 * pd[n-1] ) / 3.0;
        path.cubicTo( cp1, cp2, pd[n-1] );
    }
    else
    {
        const QPointF cp2 = ( pd[n-2] + 2.0 * pd[n-1] ) / 3.0;
        const QPointF cp3 = ( 2.0 * pd[n-1] + pd[0] ) / 3.0;

        path.cubicTo( cp1, cp2, 0.5 * ( cp2 + cp3 ) );

        if ( boundaryType == QwtSplineApproximation::ClosedPolygon )
        {
            const QPointF cp4 = ( pd[n-1] + 2.0 * pd[0] ) / 3.0;
            const QPointF cp5 = ( 2.0 * pd[0] + pd[1] ) / 3.0;

            path.cubicTo( cp3, cp4, 0.5 * ( cp4 + cp5 ) );
        }
    }

    return path;
}

static QPainterPath qwtSplineBasisPath( const QPolygonF &points, 
    const QwtSplineParametrization *param,
    QwtSplineApproximation::BoundaryType boundaryType )
{
    const int n = points.size();
    const QPointF *pd = points.constData();

    QPainterPath path;

    double t0, t1, t2, t012;

    if ( boundaryType == QwtSplineApproximation::ConditionalBoundaries )
    {
        t0 = param->valueIncrement( pd[0], pd[1] );
        t1 = t0;
        t2 = t0;
        t012 = t0 + t1 + t2;

        path.moveTo( pd[0] );
    }
    else
    {
        t0 = param->valueIncrement( pd[n-2], pd[n-1] );
        t1 = param->valueIncrement( pd[n-1], pd[0] );
        t2 = param->valueIncrement( pd[0], pd[1] );
        t012 = t0 + t1 + t2;

        const double tN = param->valueIncrement( pd[n-3], pd[n-2] );
        const double tN01 = tN + t0 + t1;
            
        const QPointF cp2 = ( t1 * pd[n-1] + ( tN + t0 ) * pd[0] ) / tN01;
        const QPointF cp3 = ( ( t1 + t2 ) * pd[0] + t0 * pd[1] ) / t012;

        const QPointF p2 = ( t1 * cp2 + t0 * cp3 ) / ( t0 + t1 );

        path.moveTo( p2 );
    }

    QPointF cp1 = ( ( t1 + t2 ) * pd[0] + t0 * pd[1] ) / t012;

    for ( int i = 1; i < n - 1; i++ )
    {
        const double t3 = param->valueIncrement( pd[i], pd[i+1] );
        const double t123 = t1 + t2 + t3;

        const QPointF cp2 = ( t2 * pd[i-1] + ( t0 + t1 ) * pd[i] ) / t012;
        const QPointF cp3 = ( ( t2 + t3 ) * pd[i] + t1 * pd[i+1] ) / t123;

        const QPointF p2 = ( t2 * cp2 + t1 * cp3 ) / ( t1 + t2 );

        path.cubicTo( cp1, cp2, p2 );

        cp1 = cp3;

        t0 = t1;
        t1 = t2;
        t2 = t3;
        t012 = t123;
    }

    if ( boundaryType == QwtSplineApproximation::ConditionalBoundaries )
    {
        const QPointF cp2 = ( t2 * pd[n-2] + ( t0 + t1 ) * pd[n-1] ) / t012;
        path.cubicTo( cp1, cp2, pd[n-1] );
    }
    else
    {
        const double t3 = param->valueIncrement( pd[n-1], pd[0] );
        const double t123 = t1 + t2 + t3;
        
        const QPointF cp2 = ( t2 * pd[n-2] + ( t0 + t1 ) * pd[n-1] ) / t012;
        const QPointF cp3 = ( ( t2 + t3 ) * pd[n-1] + t1 * pd[0] ) / t123;
        
        const QPointF p2 = ( t2 * cp2 + t1 * cp3 ) / ( t1 + t2 );
        
        path.cubicTo( cp1, cp2, p2 );
        
        if ( boundaryType == QwtSplineApproximation::ClosedPolygon )
        {
            const double t4 = param->valueIncrement( pd[0], pd[1] );
            const double t234 = t2 + t3 + t4;
        
            const QPointF cp4 = ( t3 * pd[n-1] + ( t1 + t2 ) * points[0] ) / t123;
            const QPointF cp5 = ( ( t3 + t4 ) * pd[0] + t2 * pd[1] ) / t234;

            const QPointF p2 = ( t3 * cp4 + t2 * cp5 ) / ( t2 + t3 );
 
            path.cubicTo( cp3, cp4, p2 );
        }
    }

    return path;
}

QwtSplineBasis::QwtSplineBasis()
{
}

QwtSplineBasis::~QwtSplineBasis()
{
}

uint QwtSplineBasis::locality() const
{
    return 2;
}

QPainterPath QwtSplineBasis::painterPath( const QPolygonF &points ) const
{
    // PeriodicPolygon/ClosedPolygon not implemented yet

    if ( points.size()  < 4 )
        return QPainterPath();

    QPainterPath path;

    switch( parametrization()->type() )
    {
        case QwtSplineParametrization::ParameterUniform:
        {
            path = qwtSplineBasisPathUniform( points, boundaryType() );
            break;
        }
        default:
        {
            path = qwtSplineBasisPath( points, parametrization(), boundaryType() );
        }
    }

    return path;
}
