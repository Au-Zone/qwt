/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_CUBIC_H
#define QWT_SPLINE_CUBIC_H 1

#include "qwt_global.h"
#include "qwt_spline.h"

class QWT_EXPORT QwtSplineCubic: public QwtSpline
{
public:
    enum EndpointCondition 
    {
        Natural,
        ParabolicRunout,
        CubicRunout,
        NotAKnot,
        Periodic
    };

    QwtSplineCubic();
    virtual ~QwtSplineCubic();

    void setEndConditions( EndpointCondition );

    void setClamped( double slopeBegin, double slopeEnd );
    void setClamped2( double curvatureBegin, double curvatureEnd );
    void setClamped3( double valueBegin, double valueEnd );

    virtual QVector<double> slopes( const QPolygonF & ) const;
    virtual QVector<double> curvatures( const QPolygonF & ) const;

    static void toPolynom2( const QPointF &p1, double cv1,
        const QPointF &p2, double cv2, double &a, double &b, double &c );
    static void toCurvatures( const QPointF &p1, const QPointF &p2,
        double a, double b, double c, double &cv1, double &cv2 );

private:
    class PrivateData;
    PrivateData *d_data;
};

inline void QwtSplineCubic::toPolynom2(
    const QPointF &p1, double cv1,
    const QPointF &p2, double cv2,
    double &a, double &b, double &c )
{   
    const double dx = p2.x() - p1.x();
    const double slope = ( p2.y() - p1.y() ) / dx;
    
    a = ( cv2 - cv1 ) / ( 6.0 * dx );
    b = 0.5 * cv1;
    c = slope - ( a * dx + b ) * dx;
}   

inline void QwtSplineCubic::toCurvatures( 
    const QPointF &p1, const QPointF &p2,
    double a, double b, double c, double &cv1, double &cv2 )
{
    const double dx = p2.x() - p1.x();
    const double slope = ( p2.y() - p1.y() ) / dx;
    
    const double m1 = c;
    const double m2 = ( ( 3.0 * a * dx ) + b ) * dx + c;
    
    cv1 = 2.0 * ( 3 * slope - 2 * m1 - m2 ) / dx;
    cv2 = 2.0 * ( -3 * slope + m1 + 2 * m2 ) / dx;
}   

#endif
