/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GPL License, Version 2.0
 *****************************************************************************/

/*! \file */
#ifndef QWT_POLAR_RECT_H
#define QWT_POLAR_RECT_H 1

#include "qwt_global.h"
#include "qwt_double_rect.h"
#include "qwt_polar_point.h"

class QWT_EXPORT QwtPolarRect  
{
public:
    QwtPolarRect();
    QwtPolarRect(double radius, double azimuth, double width, double height);
    QwtPolarRect(const QwtPolarPoint&, const QwtDoubleSize &);
    QwtPolarRect(const QwtDoubleRect&);

    void setRect(const QwtDoubleRect &);
    QwtDoubleRect toRect() const;

    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;

    QwtPolarRect normalized() const;

    const QwtPolarPoint &center() const;
    QwtPolarPoint &center();

    double radius()  const;
    double azimuth()  const;

    void setRadius(double);
    void setAzimuth(double);
    void setCenter(double, double);
    void setCenter(const QwtPolarPoint &);

    void setRect(double radius, double azimuth, double width, double height);
    void setRect(const QwtPolarPoint&, const QwtDoubleSize &);

    double width()   const;
    double height()  const;
    QwtDoubleSize size() const;

    void setWidth(double w );
    void setHeight(double h );
    void setSize(const QwtDoubleSize &);

    bool operator==(const QwtPolarRect &) const;
    bool operator!=(const QwtPolarRect &) const;

private:
    QwtPolarPoint d_center;
    double d_width;
    double d_height;
};

inline bool QwtPolarRect::isNull() const
{ 
    return isValid() && d_width == 0 && d_height == 0;
}

inline bool QwtPolarRect::isEmpty() const
{ 
    return !isValid() || d_width <= 0 || d_height <= 0;
}

inline bool QwtPolarRect::isValid() const
{ 
    return d_center.isValid() && d_width >= 0 && d_height >= 0;
}

inline const QwtPolarPoint &QwtPolarRect::center() const
{
    return d_center;
}

inline double QwtPolarRect::radius() const
{
    return d_center.radius();
}

inline double QwtPolarRect::azimuth() const
{
    return d_center.azimuth();
}

inline QwtPolarPoint &QwtPolarRect::center()
{
    return d_center;
}

inline void QwtPolarRect::setCenter(double radius, double azimuth)
{
    d_center.setRadius(radius);
    d_center.setAzimuth(azimuth);
}

inline void QwtPolarRect::setCenter(const QwtPolarPoint &center)
{
    d_center = center;
}

inline double QwtPolarRect::width() const
{ 
    return d_width;
}

inline double QwtPolarRect::height() const
{ 
    return d_height;
}

inline QwtDoubleSize QwtPolarRect::size() const
{ 
    return QwtDoubleSize(d_width, d_height);
}

inline void QwtPolarRect::setWidth(double width)
{
    d_width = width;
}

inline void QwtPolarRect::setHeight(double height)
{
    d_height = height;
}

#endif // QWT_POLAR_RECT_H
