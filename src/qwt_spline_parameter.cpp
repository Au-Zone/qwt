/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_parameter.h"
#include "qwt_math.h"

QwtSplineParameter::QwtSplineParameter( int type ):
    d_type( type )
{
}

QwtSplineParameter::~QwtSplineParameter()
{
}
 
double QwtSplineParameter::value( const QPointF &p1, const QPointF &p2 ) const
{
    switch( d_type )
    {
        case ParameterX:
        {
            return valueX( p1, p2 );
        }
        case ParameterCentripetral:
        {
            const double dx = p1.x() - p2.x();
            const double dy = p1.y() - p2.y();

            return ::pow( dx * dx + dy * dy, 0.25 );
        }
        case ParameterChordal:
        {
            return valueChordal( p1, p2 );
        }
        case ParameterManhattan:
        {
            return valueManhattan( p1, p2 );
        }
        case ParameterUniform:
        default:
        {
            return valueUniform( p1, p2 );
        }
    }
}

int QwtSplineParameter::type() const
{
    return d_type;
}
