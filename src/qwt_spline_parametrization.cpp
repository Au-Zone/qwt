/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_parametrization.h"

/*!
  Constructor
  \param type Parametrization type
  \sa type()
*/
QwtSplineParametrization::QwtSplineParametrization( int type ):
    d_type( type )
{
}

//! Destructor
QwtSplineParametrization::~QwtSplineParametrization()
{
}
 
/*!     
  \brief Calculate the parameter value increment for 2 points
            
  \param point1 First point
  \param point2 Second point
        
  \return Value increment
 */
double QwtSplineParametrization::valueIncrement( const QPointF &p1, const QPointF &p2 ) const
{
    switch( d_type )
    {
        case QwtSplineParametrization::ParameterX:
        {
            return valueIncrementX( p1, p2 );
        }
        case QwtSplineParametrization::ParameterCentripetal:
        {
            return valueIncrementCentripetal( p1, p2 );
        }
        case QwtSplineParametrization::ParameterChordal:
        {
            return valueIncrementChordal( p1, p2 );
        }
        case QwtSplineParametrization::ParameterManhattan:
        {
            return valueIncrementManhattan( p1, p2 );
        }
        case QwtSplineParametrization::ParameterUniform:
        default:
        {
            return valueIncrementUniform( p1, p2 );
        }
    }
}

//! \return Parametrization type
int QwtSplineParametrization::type() const
{
    return d_type;
}
