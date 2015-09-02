/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_H
#define QWT_SPLINE_H 1

#include "qwt_global.h"
#include "qwt_spline_polynom.h"
#include <qpolygon.h>
#include <qpainterpath.h>
#include <qmath.h>

class QwtSplineParameter;

class QWT_EXPORT QwtSpline
{
public:
    QwtSpline();
    virtual ~QwtSpline();

    void setParametrization( int type );
    void setParametrization( QwtSplineParameter * );
    const QwtSplineParameter *parametrization() const;

    void setClosing( bool );
    bool isClosing() const;

    virtual QPainterPath pathP( const QPolygonF & ) const;
    virtual QPolygonF polygonP( const QPolygonF &, 
        double distance, bool withNodes ) const;

    virtual QVector<QLineF> bezierControlPointsP( const QPolygonF &points ) const = 0;

private:
    // Disabled copy constructor and operator=
    explicit QwtSpline( const QwtSpline & );
    QwtSpline &operator=( const QwtSpline & );

    QwtSplineParameter *d_parameter;
    bool d_isClosing;
};

class QWT_EXPORT QwtSplineG1: public QwtSpline
{           
public:     
    QwtSplineG1();
    virtual ~QwtSplineG1();
};

class QWT_EXPORT QwtSplineC1: public QwtSplineG1
{
public:
    enum BoundaryCondition 
    {
        Clamped,
        Clamped2,
        Clamped3,

        Natural,

        LinearRunout,
        ParabolicRunout,
        CubicRunout,

        NotAKnot,
        Periodic
    };

    QwtSplineC1();
    virtual ~QwtSplineC1();

    void setBoundaryConditions( BoundaryCondition );
    BoundaryCondition boundaryCondition() const;

    void setBoundaryValues( double valueBegin, double valueEnd );

    double boundaryValueBegin() const;
    double boundaryValueEnd() const;

    virtual QPainterPath pathP( const QPolygonF & ) const;
    virtual QVector<QLineF> bezierControlPointsP( const QPolygonF &points ) const;

    virtual QVector<double> slopesX( const QPolygonF & ) const = 0;

    virtual QPolygonF polygonX( int numPoints, const QPolygonF & ) const;
    virtual QVector<QwtSplinePolynom> polynomsX( const QPolygonF & ) const;

//protected:
    virtual double slopeBegin( const QPolygonF &points, double m1, double m2 ) const;
    virtual double slopeEnd( const QPolygonF &points, double m1, double m2 ) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

class QWT_EXPORT QwtSplineC2: public QwtSplineC1
{
public:
    QwtSplineC2();
    virtual ~QwtSplineC2();

    virtual QVector<double> curvaturesX( const QPolygonF & ) const = 0;
};

#endif
