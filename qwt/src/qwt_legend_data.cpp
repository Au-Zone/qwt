/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_legend_data.h"

QwtLegendData::QwtLegendData()
{
}

QwtLegendData::~QwtLegendData()
{
}

void QwtLegendData::setValues( const QMap<int, QVariant> &map )
{
    d_map = map;
}

const QMap<int, QVariant> &QwtLegendData::values() const
{
    return d_map;
}

void QwtLegendData::setValue( int role, const QVariant &data )
{
    d_map[role] = data;
}

QVariant QwtLegendData::value( int role ) const
{
    if ( !d_map.contains( role ) )
        return QVariant();

    return d_map[role];
}

bool QwtLegendData::isValid() const
{
    return !d_map.isEmpty();
}
