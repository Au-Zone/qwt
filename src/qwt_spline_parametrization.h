/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_PARAMETRIZATION_H
#define QWT_SPLINE_PARAMETRIZATION_H 1

#include "qwt_global.h"
#include <qmath.h>
#include <qpoint.h>

/*!
  \brief Parametrization used by a spline interpolation

  When the x coordinates of the control points are not ordered monotonically 
  a parametrization has to be found, that provides a value t for each control 
  point that is increasing monotonically.

  QwtSpline calculates the parameter values at the curve points, by summing up
  increments, that are calculated for each point. Each increment is calculated
  from the coordinates of the point and its precessor. 

  - t[0] = 0;
  - t[i] = t[i-1] + spline->parametrization()->valueIncrement( point[i-1], p[i] );

  QwtSplineParametrization provides the most common used type of parametrizations 
  and offers an interface to inject custom implementations.

  A parametrization ParameterX means no parametrization.

  \sa QwtSpline::setParametrization()
 */
class QWT_EXPORT QwtSplineParametrization
{
public:
    //! Parametrization type
    enum Type
    {
        /*!
          No parametrization: t[i] = x[i]
          \sa valueX()
         */
        ParameterX,

        /*!
          Uniform parametrization: t[i] = i;
          \sa valueUniform()
         */
        ParameterUniform,

        /*!
          Centripetal parametrization
          \sa valueCentripetral()
         */
        ParameterCentripetral,

        /*!
          Parametrization using the length between two control points
          \sa valueChordal()
         */
        ParameterChordal,

        /*!
          Parametrization using the manhattan length between two control points
          \sa valueManhattan()
         */
        ParameterManhattan
    };

    explicit QwtSplineParametrization( int type );
    virtual ~QwtSplineParametrization();

    int type() const;

    virtual double valueIncrement( const QPointF &p1, const QPointF &p2 ) const;
    
    static double valueIncrementX( const QPointF &p1, const QPointF &p2 );
    static double valueIncrementCentripetal( const QPointF &p1, const QPointF &p2 );
    static double valueIncrementChordal( const QPointF &p1, const QPointF &p2 );
    static double valueIncrementUniform( const QPointF &p1, const QPointF &p2 );
    static double valueIncrementManhattan( const QPointF &p1, const QPointF &p2 );

    /*!
      Helper structure to provide functors to inline parameter calculations
      used in various spline implementation
     */
    struct param
    {
        param( const QwtSplineParametrization * );
        double operator()( const QPointF &p1, const QPointF &p2 ) const;

        const QwtSplineParametrization *parameter;
    };

    struct paramX
    {
        double operator()( const QPointF &p1, const QPointF &p2 ) const;
    };

    struct paramUniform
    {
        double operator()( const QPointF &p1, const QPointF &p2 ) const;
    };

    struct paramChordal
    {
        double operator()( const QPointF &p1, const QPointF &p2 ) const;
    };

    struct paramManhattan
    {
        double operator()( const QPointF &p1, const QPointF &p2 ) const;
    };

private:
    const int d_type;
};

inline double QwtSplineParametrization::valueIncrementX( 
    const QPointF &p1, const QPointF &p2 ) 
{
    return p2.x() - p1.x();
}

inline double QwtSplineParametrization::valueIncrementUniform(
    const QPointF &p1, const QPointF &p2 )
{
    Q_UNUSED( p1 )
    Q_UNUSED( p2 )

    return 1.0;
}

inline double QwtSplineParametrization::valueIncrementCentripetal(
    const QPointF &p1, const QPointF &p2 )
{
    const double dx = p1.x() - p2.x();
    const double dy = p1.y() - p2.y();

    return qPow( dx * dx + dy * dy, 0.25 );
}

inline double QwtSplineParametrization::valueIncrementChordal( 
    const QPointF &p1, const QPointF &p2 ) 
{
    const double dx = p1.x() - p2.x();
    const double dy = p1.y() - p2.y();

    return qSqrt( dx * dx + dy * dy );
}

inline double QwtSplineParametrization::valueIncrementManhattan(
    const QPointF &p1, const QPointF &p2 )
{
    return qAbs( p2.x() - p1.x() ) + qAbs( p2.y() - p1.y() );
}

inline QwtSplineParametrization::param::param( const QwtSplineParametrization *p ):
    parameter( p ) 
{
}
    
inline double QwtSplineParametrization::param::operator()( 
    const QPointF &p1, const QPointF &p2 ) const
{
    return parameter->valueIncrement( p1, p2 );
}

inline double QwtSplineParametrization::paramX::operator()( 
    const QPointF &p1, const QPointF &p2 ) const 
{
    return QwtSplineParametrization::valueIncrementX( p1, p2 );
}

inline double QwtSplineParametrization::paramUniform::operator()(
    const QPointF &p1, const QPointF &p2 ) const
{
    return QwtSplineParametrization::valueIncrementUniform( p1, p2 );
}
    
inline double QwtSplineParametrization::paramChordal::operator()( 
    const QPointF &p1, const QPointF &p2 ) const 
{
    return QwtSplineParametrization::valueIncrementChordal( p1, p2 );
}

inline double QwtSplineParametrization::paramManhattan::operator()( 
    const QPointF &p1, const QPointF &p2 ) const 
{
    return QwtSplineParametrization::valueIncrementManhattan( p1, p2 );
}

#endif
