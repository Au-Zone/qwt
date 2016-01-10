#ifndef _SPLINE_BASIS_UNIFORM_H
#define _SPLINE_BASIS_UNIFORM_H

#include <QPainterPath>

class SplineBasisUniform
{
public:
    SplineBasisUniform();
    virtual ~SplineBasisUniform();

    QPainterPath painterPath( const QPolygonF & ) const;
    QPainterPath painterPathInterpolated( const QPolygonF & ) const;

private:
    QPolygonF interpolatingKnots( const QPolygonF & ) const;
    QPainterPath toBezierUniform( const QPolygonF & ) const;
    QPainterPath toBezierUniform2( const QPolygonF & ) const;
    QPainterPath toBezierSlopes( const QPolygonF & ) const;
};

#endif  

