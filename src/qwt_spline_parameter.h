/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_PARAMETER_H
#define QWT_SPLINE_PARAMETER_H 1

#include "qwt_global.h"
#include <qmath.h>
#include <qpoint.h>

class QWT_EXPORT QwtSplineParameter
{
public:
    enum Type
    {
        ParameterX,
        ParameterUniform,
        ParameterCentripetral,
        ParameterChordal,
        ParameterManhattan
    };

    explicit QwtSplineParameter( int type );
    virtual ~QwtSplineParameter();

    int type() const;

    virtual double value( const QPointF &p1, const QPointF &p2 ) const;
    
    static double valueX( const QPointF &p1, const QPointF &p2 );
    static double valueChordal( const QPointF &p1, const QPointF &p2 );
    static double valueUniform( const QPointF &p1, const QPointF &p2 );
    static double valueManhattan( const QPointF &p1, const QPointF &p2 );

    // functors
    struct param
    {
        param( const QwtSplineParameter * );
        double operator()( const QPointF &p1, const QPointF &p2 ) const;

        const QwtSplineParameter *parameter;
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

inline double QwtSplineParameter::valueX( 
    const QPointF &p1, const QPointF &p2 ) 
{
    return p2.x() - p1.x();
}

inline double QwtSplineParameter::valueUniform(
    const QPointF &p1, const QPointF &p2 )
{
    Q_UNUSED( p1 )
    Q_UNUSED( p2 )

    return 1.0;
}

inline double QwtSplineParameter::valueChordal( 
    const QPointF &p1, const QPointF &p2 ) 
{
    const double dx = p1.x() - p2.x();
    const double dy = p1.y() - p2.y();

    return qSqrt( dx * dx + dy * dy );
}

inline double QwtSplineParameter::valueManhattan(
    const QPointF &p1, const QPointF &p2 )
{
    return qAbs( p2.x() - p1.x() ) + qAbs( p2.y() - p1.y() );
}

inline QwtSplineParameter::param::param( const QwtSplineParameter *p ):
    parameter( p ) 
{
}
    
inline double QwtSplineParameter::param::operator()( 
    const QPointF &p1, const QPointF &p2 ) const
{
    return parameter->value( p1, p2 );
}

inline double QwtSplineParameter::paramX::operator()( 
    const QPointF &p1, const QPointF &p2 ) const 
{
    return QwtSplineParameter::valueX( p1, p2 );
}

inline double QwtSplineParameter::paramUniform::operator()(
    const QPointF &p1, const QPointF &p2 ) const
{
    return QwtSplineParameter::valueUniform( p1, p2 );
}
    
inline double QwtSplineParameter::paramChordal::operator()( 
    const QPointF &p1, const QPointF &p2 ) const 
{
    return QwtSplineParameter::valueChordal( p1, p2 );
}

inline double QwtSplineParameter::paramManhattan::operator()( 
    const QPointF &p1, const QPointF &p2 ) const 
{
    return QwtSplineParameter::valueManhattan( p1, p2 );
}

#endif
