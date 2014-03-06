/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_curve_fitter.h"
#include "qwt_math.h"
#include <qline.h>
#include <qpainterpath.h>

// - bezier

static inline double qwtLineLength( const QPointF &p_start, const QPointF &p_end )
{
   const double dx = p_start.x() - p_end.x();
   const double dy = p_start.y() - p_end.y();

   return qSqrt( dx * dx + dy * dy );
}

static inline QLineF qwtControlLine(const QPointF &p0, const QPointF &p1, 
    const QPointF &p2, const QPointF &p3 )
{
    static const double one_third  = 1.0/3.0;
    static const double one_sixth = 1.0/6.0;

    const double d02 = qwtLineLength(p0, p2);
    const double d13 = qwtLineLength(p1, p3);
    const double d12_2 = 0.5 * qwtLineLength(p1, p2);

    const bool b1 = ( d02 / 6.0 ) < d12_2;
    const bool b2 = ( d13 / 6.0 ) < d12_2;

    QPointF off1, off2;

    if( b1 && b2 )
    {
        // this is the normal case where both 1/6th 
        // vectors are less than half of d12_2

        const double s1 = (p0 != p1) ? one_sixth : one_third;
        off1 = (p2 - p0) * s1;

        const double s2 = (p2 != p3) ? one_sixth : one_third;
        off2 = (p1 - p3) * s2;
    }
    else if ( !b1 && b2 )
    {
        // for this case d02/6 is more than half of d12_2, so
        // the d13/6 vector needs to be reduced
        off1 = (p2 - p0) * (d12_2 / d02);
        off2 = (p1 - p3) * (d12_2 / d02);
    }
    else if ( b1 && !b2 )
    {
        off1 = (p2 - p0) * (d12_2 / d13);
        off2 = (p1 - p3) * (d12_2 / d13);
    }
    else
    {
        off1 = (p2 - p0) * (d12_2 / d02);
        off2 = (p1 - p3) * (d12_2 / d13); 
    }   

    return QLineF( p1 + off1, p2 + off2 );
}


//! Constructor
QwtSplineCurveFitter::QwtSplineCurveFitter()
{
}

//! Destructor
QwtSplineCurveFitter::~QwtSplineCurveFitter()
{
}

/*!
  Find a curve which has the best fit to a series of data points

  \param points Series of data points
  \return Curve points
*/
QPolygonF QwtSplineCurveFitter::fitCurve( const QPolygonF &points ) const
{
    const int size = points.size();
    if ( size <= 2 )
        return points;

    const QPointF *p = points.constData();

    QVector<QLineF> controlLines;

    controlLines += qwtControlLine( p[0], p[0], p[1], p[2]);

    for( int i = 1; i < size - 2; ++i )
        controlLines += qwtControlLine( p[i-1], p[i], p[i+1], p[i+2]);

    controlLines += qwtControlLine( p[size - 3], p[size - 2], p[size - 1], p[size - 1] );


    QPainterPath path;
    path.moveTo( points[0] );

    for ( int i = 1; i < points.size(); i++ )
    {
        const QLineF &l = controlLines[i - 1];
        path.cubicTo( l.p1(), l.p2(), points[i] );
    }

    QPolygonF fittedPoints;

    const QList<QPolygonF> subPaths = path.toSubpathPolygons();
    if ( !subPaths.isEmpty() )
        fittedPoints = subPaths.first();

    return fittedPoints;
}
