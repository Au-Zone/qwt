/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GPL License, Version 2.0
 *****************************************************************************/

/*! \file */
#ifndef QWT_POLAR_POINT_H
#define QWT_POLAR_POINT_H 1

#include "qwt_global.h"
#include "qwt_double_rect.h"

/*!
  \brief A point in polar coordinates

  In polar coordinates a point is determined by an angle and a distance.
  See http://en.wikipedia.org/wiki/Polar_coordinate_system
*/

class QWT_EXPORT QwtPolarPoint
{
public:
    QwtPolarPoint();
    QwtPolarPoint(double radius, double azimuth);
    QwtPolarPoint(const QwtPolarPoint &);
    QwtPolarPoint(const QwtDoublePoint &);

    void setPoint(const QwtDoublePoint &);
    QwtDoublePoint toPoint() const;

    bool isValid() const;
    bool isNull() const;

    double radius() const;
    double azimuth() const;

    double &rRadius();
    double &rAzimuth();

    void setRadius(double);
    void setAzimuth(double);

    bool operator==(const QwtPolarPoint &) const;
    bool operator!=(const QwtPolarPoint &) const;

    QwtPolarPoint normalized() const;

private:
    double d_radius;
    double d_azimuth;
};

/*!
    Constructs a null point, with a radius and azimuth set to 0.0.
    \sa QwtDoublePoint::isNull
*/
inline QwtPolarPoint::QwtPolarPoint():
    d_radius(0.0),
    d_azimuth(0.0)
{
}

//! Constructs a point with coordinates specified by radius and azimuth.
inline QwtPolarPoint::QwtPolarPoint(double radius, double azimuth):
    d_radius(radius),
    d_azimuth(azimuth)
{
}

/*!
    Copy constructor.

    Constructs a point using the values of the point specified.
*/
inline QwtPolarPoint::QwtPolarPoint(const QwtPolarPoint &other):
    d_radius(other.d_radius),
    d_azimuth(other.d_azimuth)
{
}

//! Returns true if radius() >= 0.0
inline bool QwtPolarPoint::isValid() const
{ 
    return d_radius >= 0.0;
}

//! Returns true if radius() >= 0.0
inline bool QwtPolarPoint::isNull() const
{ 
    return d_radius == 0.0;
}

//! Returns the radius.
inline double QwtPolarPoint::radius() const
{ 
    return d_radius; 
}

//! Returns the azimuth.
inline double QwtPolarPoint::azimuth() const
{   
    return d_azimuth; 
}

//! Returns the radius.
inline double &QwtPolarPoint::rRadius()
{
    return d_radius;
}

//! Returns the azimuth.
inline double &QwtPolarPoint::rAzimuth()
{
    return d_azimuth;
}

//! Sets the radius to radius.
inline void QwtPolarPoint::setRadius(double radius)
{ 
    d_radius = radius; 
}

//! Sets the atimuth to atimuth.
inline void QwtPolarPoint::setAzimuth(double azimuth)
{ 
    d_azimuth = azimuth; 
}

#endif // QWT_POLAR_POINT_H
