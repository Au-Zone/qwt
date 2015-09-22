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
  \brief Curve parametrization used for a spline interpolation

  Parametrization is the process of finding a parameter value for
  each curve point - usually related to some physical quantity 
  ( distance, time ... ).

  The values are calculated by cummulating increments, that are provided
  by QwtSplineParametrization. As the curve parameters need to be
  montonically increasing, each increment need to be positive.
  
  - t[0] = 0;
  - t[i] = t[i-1] + valueIncrement( point[i-1], p[i] );

  QwtSplineParametrization provides the most common used type of
  parametrizations and offers an interface to inject custom implementations.

  \note The most relevant types of parametrization are trying to provide an
        approximation of the curve length. 

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
          \sa valueIncrementX()
         */
        ParameterX,

        /*!
          Uniform parametrization: t[i] = i;
          \sa valueIncrementUniform()
         */
        ParameterUniform,

        /*!
          Centripetal parametrization
          \sa valueIncrementCentripetal()
         */
        ParameterCentripetal,

        /*!
          Parametrization using the length between two control points
          \sa valueIncrementChordal()
         */
        ParameterChordal,

        /*!
          Parametrization using the manhattan length between two control points
          \sa valueIncrementManhattan()
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

#endif
