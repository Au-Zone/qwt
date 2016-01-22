#ifndef _SPLINE_BASIS_H
#define _SPLINE_BASIS_H

#include <qwt_spline_approximation.h>

class SplineBasis: public QwtSplineApproximation
{
public:
    SplineBasis();
    virtual ~SplineBasis();

    virtual QPainterPath painterPath( const QPolygonF & ) const;
    virtual uint locality() const;
};

#endif  

