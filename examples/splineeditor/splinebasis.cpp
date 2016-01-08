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

    double t1 = d_parametrization->valueIncrement( points[0], points[1] );
    double t2 = d_parametrization->valueIncrement( points[1], points[2] );

    double a0 = 0.0;

    for ( int i = 0; i < n - 4; i++ )
    {
        const double t3 = d_parametrization->valueIncrement( points[i+2], points[i+3] );

        const double a1 = t1 / ( t1 + t2 );
        const double a2 = t1 / ( t1 + t2 + t3 );

        const QPointF cp1 = ( 1.0 - a0 ) * points[i+1] + a0 * points[i+2];
        const QPointF cp2 = ( 1.0 - a1 ) * cp1 + a1 * points[i+2];

        const QPointF p = ( 1.0 - a2 ) * points[i+2] + a2 * points[i+3];
        const QPointF p2 = ( 1.0 - a1 ) * cp2 + a1 * p;

#if 0
    qDebug() << "==" << i;
    qDebug() << points[i].x() << points[i+1].x() << points[i+2].x() << points[i+3].x();
    qDebug() << cp1.x() << cp2.x() << p2.x();
#endif
        path.cubicTo( cp1, cp2, p2 );

        t1 = t2;
        t2 = t3;
        a0 = a2;
    }

    const QPointF p = ( 1.0 - a0 ) * points[n-3] + a0 * points[n-2];

    path.cubicTo( p, points[n-2], points[n-1] );

    return path;
}
