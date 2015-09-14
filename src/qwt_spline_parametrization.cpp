/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_parametrization.h"
#include "qwt_math.h"

static inline double qwtValueCentripetral( const QPointF &p1, const QPointF &p2 )
{
    const double dx = p1.x() - p2.x();
    const double dy = p1.y() - p2.y();

    return ::pow( dx * dx + dy * dy, 0.25 );
}

QwtSplineParametrization::QwtSplineParametrization( int type ):
    d_type( type )
{
}

QwtSplineParametrization::~QwtSplineParametrization()
{
}
 
double QwtSplineParametrization::valueCentripetal( const QPointF &p1, const QPointF &p2 )
{
    return qwtValueCentripetral( p1, p2 );
}

double QwtSplineParametrization::value( const QPointF &p1, const QPointF &p2 ) const
{
    switch( d_type )
    {
        case QwtSplineParametrization::ParameterX:
        {
            return valueX( p1, p2 );
        }
        case QwtSplineParametrization::ParameterCentripetral:
        {
            return qwtValueCentripetral( p1, p2 );
        }
        case QwtSplineParametrization::ParameterChordal:
        {
            return valueChordal( p1, p2 );
        }
        case QwtSplineParametrization::ParameterManhattan:
        {
            return valueManhattan( p1, p2 );
        }
        case QwtSplineParametrization::ParameterUniform:
        default:
        {
            return valueUniform( p1, p2 );
        }
    }
}

int QwtSplineParametrization::type() const
{
    return d_type;
}
