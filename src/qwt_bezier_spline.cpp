/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_bezier_spline.h"
#include <qpoint.h>
#include <cmath>

static const double one_third  = 1.0 / 3.0;
static const double one_sixth = 1.0 / 6.0;

class QwtBezierSpline::PrivateData
{
public:
    class ControlPoints
    {
    public:
        QPointF bz_0;
        QPointF bz_1;
    };

    // Bezier curves control points
    QVector<ControlPoints> bz_pts;

    // control points - knots
    QPolygonF points;
};

static int lookup( double x, const QPolygonF &values )
{
#if 0
//qLowerBound/qHigherBound ???
#endif
    int i1;
    const int size = values.size();

    if ( x <= values[0].x() )
        i1 = 0;
    else if ( x >= values[size - 2].x() )
        i1 = size - 2;
    else
    {
        i1 = 0;
        int i2 = size - 2;
        int i3 = 0;

        while ( i2 - i1 > 1 )
        {
            i3 = i1 + ( ( i2 - i1 ) >> 1 );

            if ( values[i3].x() > x )
                i2 = i3;
            else
                i1 = i3;
        }
    }
    return i1;
}

// calculate distance between two points
static inline double ptDist( const QPointF &p_start, const QPointF &p_end )
{
    return ::sqrt( ::pow( ( p_start.x() - p_end.x() ), 2.0 ) +
                   pow( ( p_start.y() - p_end.y() ), 2.0 ) );
}

void QwtBezierSpline::computeBezierPoints( const QPointF &pt_0, const QPointF &pt_1,
        const QPointF &pt_2, const QPointF &pt_3, int bz_curve )
{
    QPointF pt_tmp( 0, 0 );

    const double pt_12_dist = ptDist( pt_1, pt_2 ) / 2.0;
    const double pt_02_dist = ptDist( pt_0, pt_2 );
    const double pt_13_dist = ptDist( pt_1, pt_3 );

    if( ( pt_02_dist / 6.0 < pt_12_dist ) && ( pt_13_dist / 6.0 < pt_12_dist ) ) // IF_0_S
    {
        // this is the normal case where both 1/6th vectors are less than half of pt_12_dist
        double scale_fact = 0;

        if( pt_0 != pt_1 )
        {
            scale_fact = one_sixth;
        }
        else
        {
            scale_fact = one_third;    // end point interval
        }
        pt_tmp = ( pt_2 - pt_0 ) * scale_fact;
        d_data->bz_pts[bz_curve].bz_0 = pt_1 + pt_tmp;
        // -----------------
        if( pt_2 != pt_3 )
        {
            scale_fact = one_sixth;
        }
        else
        {
            scale_fact = one_third;    // end point interval
        }
        pt_tmp = ( pt_1 - pt_3 ) * scale_fact;
        d_data->bz_pts[bz_curve].bz_1 = pt_2 + pt_tmp;

    }
    else  // IF_0_EL
    {

        if( ( pt_02_dist / 6.0 >= pt_12_dist ) && ( pt_13_dist / 6.0 >= pt_12_dist ) ) // IF_1_S
        {
            // this is the case where both 1/6th vectors are > than half of pt_12_dist
            pt_tmp = ( pt_2 - pt_0 ) * ( pt_12_dist / pt_02_dist ); // subtract and scale
            d_data->bz_pts[bz_curve].bz_0 = pt_1 + pt_tmp;
            // -----------------
            pt_tmp = ( pt_1 - pt_3 ) * ( pt_12_dist / pt_13_dist ); // subtract and scale
            d_data->bz_pts[bz_curve].bz_1 = pt_2 + pt_tmp;
        }
        else  // IF_1_EL
        {
            if( pt_02_dist / 6.0 >= pt_12_dist ) // IF_2_S
            {
                // for this case pt_02_dist/6 is more than half of pt_12_dist, so
                // the pt_13_dist/6 vector needs to be reduced
                pt_tmp = ( pt_2 - pt_0 ) * ( pt_12_dist / pt_02_dist );
                d_data->bz_pts[bz_curve].bz_0 = pt_1 + pt_tmp;
                // -----------------
                pt_tmp = ( pt_1 - pt_3 ) * ( pt_12_dist / pt_02_dist );
                d_data->bz_pts[bz_curve].bz_1 = pt_2 + pt_tmp;
            }
            else  // IF_2_EL
            {
                // if d13 / 6 >= d12 / 2
                pt_tmp = ( pt_2 - pt_0 ) * ( pt_12_dist / pt_13_dist );
                d_data->bz_pts[bz_curve].bz_0 = pt_1 + pt_tmp;
                // -----------------
                pt_tmp = ( pt_1 - pt_3 ) * ( pt_12_dist / pt_13_dist );
                d_data->bz_pts[bz_curve].bz_1 = pt_2 + pt_tmp;
            }  // IF_2_E
        }  // IF_1_E
    }  // IF_0_E
}


//! Constructor
QwtBezierSpline::QwtBezierSpline()
{
    d_data = new PrivateData;
}

/*!
   Copy constructor
   \param other Bezier used for initialization
*/
QwtBezierSpline::QwtBezierSpline( const QwtBezierSpline &other )
{
    d_data = new PrivateData( *other.d_data );
}

/*!
   Assignment operator
   \param other Bezier used for initialization
   \return *this
*/
QwtBezierSpline &QwtBezierSpline::operator=( const QwtBezierSpline &other )
{
    *d_data = *other.d_data;
    return *this;
}

//! Destructor
QwtBezierSpline::~QwtBezierSpline()
{
    delete d_data;
}

/*!
  \brief Calculate the bezier coefficients

  \param points Points
  \return true if successful
  \warning The sequence of x (but not y) values has to be strictly monotone
           increasing, which means <code>points[i].x() < points[i+1].x()</code>.
       If this is not the case, the function will return false
*/
bool QwtBezierSpline::setPoints( const QPolygonF& points )
{
    const int size = points.size();
    if ( size <= 2 )
    {
        reset();
        return false;
    }

    d_data->points = points;
    d_data->bz_pts.resize( size - 1 );

    const bool ok = buildBezier( points );
    if ( !ok )
        reset();

    return ok;
}

/*!
   \return Points, that have been by setPoints()
*/
QPolygonF QwtBezierSpline::points() const
{
    return d_data->points;
}

//! \return bz0 control points
void QwtBezierSpline::controlPointsBz0() const
{
    //todo: return vector of calculated Bezier control points
}

//! \return bz1 control points
void QwtBezierSpline::controlPointsBz1() const
{
    //todo: return vector of calculated Bezier control points
}

//! Free allocated memory and set size to 0
void QwtBezierSpline::reset()
{
    d_data->bz_pts.resize( 0 );
    d_data->points.resize( 0 );
}

//! True if valid
bool QwtBezierSpline::isValid() const
{
    return d_data->bz_pts.size() > 0;
}

/*!
  Calculate the interpolated function value corresponding
  to a given argument x.

  \param x Coordinate
  \return Interpolated coordinate
*/
double QwtBezierSpline::value( double x ) const
{
    if ( d_data->bz_pts.size() == 0 )
        return 0.0;

    // ================================================================
    // Four control point Bezier interpolation
    // param ranges from 0 to 1, start to end of curve
    const int i = lookup( x, d_data->points );
    const PrivateData::ControlPoints ptr_bz = d_data->bz_pts[i];
    const QPointF * ptr_pt = &d_data->points[i];

    const double diff_1 = ( x - ptr_pt->x() ) / ( ( ptr_pt + 1 )->x() - ptr_pt->x() );
    const double diff_2 = diff_1 * diff_1;
    const double diff_3 = diff_2 * diff_1;
    const double param  = 1.0 - diff_1;  // range from 0 to 1, knot[i] -> knot[i+1]

    return ( ( ( param * ptr_pt->y() + 3.0 * diff_1 * ptr_bz.bz_0.y() ) *
               param + 3.0 * diff_2 * ptr_bz.bz_1.y() ) *
             param + diff_3 * ( ptr_pt + 1 )->y() );
}

/*!
  \brief Determines the control points for a Microsoft bezier curve
  \return true if successful
*/
bool QwtBezierSpline::buildBezier( const QPolygonF &points )
{
    const QPointF * p = points.constData();
    const int n = d_data->points.size() - 2;

    for( int i = 0; i <= n; i++ )
    {
        if( i == 0 )
        {
            computeBezierPoints( p[i], p[i], p[i + 1], p[i + 2], i );
        }
        else
        {
            if ( i == n )
            {
                computeBezierPoints( p[i - 1], p[i], p[i + 1], p[i + 1], i );
            }
            else
            {
                computeBezierPoints( p[i - 1], p[i], p[i + 1], p[i + 2], i );
            }
        }
    }

    return true;
}
