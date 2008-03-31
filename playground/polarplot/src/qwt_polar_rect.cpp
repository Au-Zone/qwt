/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GPL License, Version 2.0
 *****************************************************************************/

#include <qglobal.h>
#include "qwt_polar_rect.h"

QwtPolarRect::QwtPolarRect():
    d_width(0.0),
    d_height(0.0)
{
}

QwtPolarRect::QwtPolarRect(double radius, double azimuth, 
        double width, double height):
    d_center(radius, azimuth),
    d_width(width),
    d_height(height)
{
}

QwtPolarRect::QwtPolarRect(
        const QwtPolarPoint &center, const QwtDoubleSize &size):
    d_center(center),
    d_width(size.width()),
    d_height(size.height())
{
}

QwtPolarRect::QwtPolarRect(const QwtDoubleRect &rect):
    d_center(rect.center()),
    d_width(rect.width()),
    d_height(rect.height())
{
}

void QwtPolarRect::setRect(const QwtDoubleRect &rect)
{
    d_center.setPoint(rect.center());
    d_width = rect.width();
    d_height = rect.height();
}

QwtDoubleRect QwtPolarRect::toRect() const
{
    QwtDoubleRect rect(0, 0, d_width, d_height);
    rect.moveCenter(d_center.toPoint());

    return rect;
}

void QwtPolarRect::setRect(double radius, double azimuth, 
    double width, double height)
{
    d_center.setRadius(radius);
    d_center.setAzimuth(azimuth);
    d_width = width;
    d_height = height;
}

void QwtPolarRect::setRect(const QwtPolarPoint &center, 
    const QwtDoubleSize &size)
{
    d_center = center;
    d_width = size.width();
    d_height = size.height();
}

bool QwtPolarRect::operator==(const QwtPolarRect &other) const
{
    return d_center == other.d_center && 
        d_width == other.d_width && d_height == other.d_height;
}

bool QwtPolarRect::operator!=(const QwtPolarRect &other) const
{
    return d_center != other.d_center ||
        d_width != other.d_width || d_height != other.d_height;
}

QwtPolarRect QwtPolarRect::normalized() const
{
    QwtPolarPoint center = d_center.normalized();
    return QwtPolarRect(center.radius(), center.azimuth(),
        d_width, d_height);
}

