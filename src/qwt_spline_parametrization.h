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
  a "trick" called parametrization has to be used, that provides a value t for
  each control point that is increasing monotonically:

  - t[0] = 0;
  - t[i] = t[i-1] + value( point[i-1], p[i] );
  
  QwtSplineParametrization provides some common used strategies 
  of parametrization and offers an interface to inject custom parametrizations
  for the interpolations offered by QwtSpline:

  A parametrization ParameterX means no parametrization.

  \warning Calculating a parametrized spline is usually less performant 
           as the the interpolation has to be done twice for 
           ( t[i], x[i] ) and ( t[i], y[i] ).

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
          Parametrization using the distance between two control points
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

    virtual double value( const QPointF &p1, const QPointF &p2 ) const;
    
    static double valueX( const QPointF &p1, const QPointF &p2 );
    static double valueCentripetal( const QPointF &p1, const QPointF &p2 );
    static double valueChordal( const QPointF &p1, const QPointF &p2 );
    static double valueUniform( const QPointF &p1, const QPointF &p2 );
    static double valueManhattan( const QPointF &p1, const QPointF &p2 );

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

inline double QwtSplineParametrization::valueX( 
    const QPointF &p1, const QPointF &p2 ) 
{
    return p2.x() - p1.x();
}

inline double QwtSplineParametrization::valueUniform(
    const QPointF &p1, const QPointF &p2 )
{
    Q_UNUSED( p1 )
    Q_UNUSED( p2 )

    return 1.0;
}

inline double QwtSplineParametrization::valueChordal( 
    const QPointF &p1, const QPointF &p2 ) 
{
    const double dx = p1.x() - p2.x();
    const double dy = p1.y() - p2.y();

    return qSqrt( dx * dx + dy * dy );
}

inline double QwtSplineParametrization::valueManhattan(
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
    return parameter->value( p1, p2 );
}

inline double QwtSplineParametrization::paramX::operator()( 
    const QPointF &p1, const QPointF &p2 ) const 
{
    return QwtSplineParametrization::valueX( p1, p2 );
}

inline double QwtSplineParametrization::paramUniform::operator()(
    const QPointF &p1, const QPointF &p2 ) const
{
    return QwtSplineParametrization::valueUniform( p1, p2 );
}
    
inline double QwtSplineParametrization::paramChordal::operator()( 
    const QPointF &p1, const QPointF &p2 ) const 
{
    return QwtSplineParametrization::valueChordal( p1, p2 );
}

inline double QwtSplineParametrization::paramManhattan::operator()( 
    const QPointF &p1, const QPointF &p2 ) const 
{
    return QwtSplineParametrization::valueManhattan( p1, p2 );
}

#endif
