/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_H
#define QWT_SPLINE_H 1

#include "qwt_global.h"
#include "qwt_spline_polynomial.h"
#include <qpolygon.h>
#include <qpainterpath.h>
#include <qmath.h>

class QwtSplineParametrization;

/*!
  \brief Base class for a spline interpolation

  Geometric Continuity

    G0: curves are joined
    G1: first derivatives are proportional at the join point
        The curve tangents thus have the same direction, but not necessarily the 
        same magnitude. i.e., C1'(1) = (a,b,c) and C2'(0) = (k*a, k*b, k*c).
    G2: first and second derivatives are proportional at join point 

  Parametric Continuity

    C0: curves are joined
    C1: first derivatives equal
    C2: first and second derivatives are equal

  Geometric continuity requires the geometry to be continuous, while parametric 
  continuity requires that the underlying parameterization be continuous as well.

  Parametric continuity of order n implies geometric continuity of order n, but not vice-versa. 

  QwtSpline is a base class for spline interpolations of any continuity.
*/
class QWT_EXPORT QwtSpline
{
public:
    enum Position
    {
        Beginning,
        End
    };

    enum BoundaryType
    {
        ConditionalBoundaries,
        PeriodicPolygon,

        /*!
          ClosedPolygon is similar to PeriodicPolygon beside, that
          the interpolation includes the connection between the last 
          and the first control point.

          \note Only works for parametrizations, where the parameter increment 
                for the the final closing line is positive. 
                This excludes QwtSplineParametrization::ParameterX and 
                QwtSplineParametrization::ParameterY
         */

        ClosedPolygon
    };

    enum BoundaryCondition
    {
        // clamping is possible for all splines

        Clamped1,

        // Natural := Clamped2 with boundary values: 0.0
        Clamped2,

        // Parabolic runout := Clamped3 with boundary values: 0.0
        Clamped3,

        // conditions, that require C1 continuity
        LinearRunout, 

        // conditions, that require C2 continuity
        CubicRunout, 
        NotAKnot
    };

    QwtSpline();
    virtual ~QwtSpline();

    void setParametrization( int type );
    void setParametrization( QwtSplineParametrization * );
    const QwtSplineParametrization *parametrization() const;

    void setBoundaryType( BoundaryType );
    BoundaryType boundaryType() const;

    void setBoundaryConditions( BoundaryCondition );
    BoundaryCondition boundaryCondition() const;

    void setBoundaryValues( double valueBegin, double valueEnd );

    double boundaryValueBegin() const;
    double boundaryValueEnd() const;

    virtual QPolygonF equidistantPolygon( const QPolygonF &, 
        double distance, bool withNodes ) const;

    virtual QPainterPath painterPath( const QPolygonF & ) const;
    virtual QVector<QLineF> bezierControlLines( const QPolygonF &points ) const = 0;

    virtual uint locality() const;

private:
    // Disabled copy constructor and operator=
    explicit QwtSpline( const QwtSpline & );
    QwtSpline &operator=( const QwtSpline & );

    class PrivateData;
    PrivateData *d_data;
};

/*!
  \brief Base class for spline interpolations providing a 
         first order geometric continuity ( G1 ) between adjoing curves
 */
class QWT_EXPORT QwtSplineG1: public QwtSpline
{           
public:     
    QwtSplineG1();
    virtual ~QwtSplineG1();
};

/*!
  \brief Base class for spline interpolations providing a 
         first order parametric continuity ( C1 ) between adjoing curves
 */
class QWT_EXPORT QwtSplineC1: public QwtSplineG1
{
public:

    QwtSplineC1();
    virtual ~QwtSplineC1();

    virtual QPainterPath painterPath( const QPolygonF & ) const;
    virtual QVector<QLineF> bezierControlLines( const QPolygonF & ) const;

    virtual QPolygonF equidistantPolygon( const QPolygonF &,
        double distance, bool withNodes ) const;

    // calculating the parametric equations
    virtual QVector<QwtSplinePolynomial> polynomials( const QPolygonF & ) const;
    virtual QVector<double> slopes( const QPolygonF & ) const = 0;
};

/*!
  \brief Base class for spline interpolations providing a 
         second order parametric continuity ( C2 ) between adjoing curves
 */
class QWT_EXPORT QwtSplineC2: public QwtSplineC1
{
public:
    QwtSplineC2();
    virtual ~QwtSplineC2();

    virtual QPainterPath painterPath( const QPolygonF & ) const;
    virtual QVector<QLineF> bezierControlLines( const QPolygonF & ) const;

    virtual QPolygonF equidistantPolygon( const QPolygonF &,
        double distance, bool withNodes ) const;

    // calculating the parametric equations
    virtual QVector<QwtSplinePolynomial> polynomials( const QPolygonF & ) const;
    virtual QVector<double> slopes( const QPolygonF & ) const;
    virtual QVector<double> curvatures( const QPolygonF & ) const = 0;
};

#endif
