#ifndef _SPLINE_BASIS_H
#define _SPLINE_BASIS_H

#include <QPainterPath>

class QwtSplineParametrization;

class SplineBasis
{
public:
    SplineBasis();
    virtual ~SplineBasis();

    void setParametrization( int type );
    void setParametrization( QwtSplineParametrization * );
    const QwtSplineParametrization *parametrization() const;

    QPainterPath painterPath( const QPolygonF & ) const;

private:
    QPainterPath toBezierNURBS( const QPolygonF & ) const;
    QwtSplineParametrization* d_parametrization;
};

#endif  

