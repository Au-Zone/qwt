#include "splinebasis.h"
#include <qwt_spline_parametrization.h>

SplineBasis::SplineBasis()
{
    d_parametrization = 
        new QwtSplineParametrization( QwtSplineParametrization::ParameterChordal );
}

SplineBasis::~SplineBasis()
{
    delete d_parametrization;
}

void SplineBasis::setParametrization( int type )
{
    if ( d_parametrization->type() != type )
    {
        delete d_parametrization;
        d_parametrization = new QwtSplineParametrization( type );
    }
}

void SplineBasis::setParametrization( QwtSplineParametrization *parametrization )
{
    if ( parametrization != NULL && parametrization != d_parametrization )
    {
        delete d_parametrization;
        d_parametrization = parametrization;
    }
}

const QwtSplineParametrization *SplineBasis::parametrization() const
{
    return d_parametrization;
}

QPainterPath SplineBasis::painterPath( const QPolygonF &points ) const
{
    const int n = points.size();
    if ( n  < 4 )
        return QPainterPath();

    QPainterPath path;
    path.moveTo( points[0] );

    double t0 = d_parametrization->valueIncrement( points[0], points[1] );
    double t1 = t0;
    double t2 = t0;
    double t012 = t0 + t1 + t2;

    QPointF cp1 = ( ( t1 + t2 ) * points[0] + t0 * points[1] ) / t012;

    for ( int i = 1; i < n - 1; i++ )
    {
        const double t3 = d_parametrization->valueIncrement( points[i], points[i+1] );
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
