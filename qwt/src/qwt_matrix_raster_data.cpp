/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_matrix_raster_data.h"
#include <qnumeric.h>
#include <qmath.h>

class QwtMatrixRasterData::PrivateData
{
public:
    PrivateData():
        resampleMode(QwtMatrixRasterData::NearestNeighbour),
        numColumns(0)
    {
    }

    QwtMatrixRasterData::ResampleMode resampleMode;
    QwtInterval range;

    QVector<double> values;
    size_t numColumns;
    size_t numRows;

    double dx;
    double dy;
};

QwtMatrixRasterData::QwtMatrixRasterData()
{
    d_data = new PrivateData();
    update();
}

QwtMatrixRasterData::QwtMatrixRasterData( 
    const QRectF &boundingRect, const QwtInterval &range,
    const QVector<double> &values, size_t numColumns )
{
    d_data = new PrivateData();
    d_data->range = range;
    d_data->values = values;
    d_data->numColumns = numColumns;
    
    setBoundingRect(boundingRect);
    update();
}

QwtMatrixRasterData::~QwtMatrixRasterData()
{
    delete d_data;
}

void QwtMatrixRasterData::setBoundingRect( const QRectF &rect )
{
    QwtRasterData::setBoundingRect( rect );
    update();
}

void QwtMatrixRasterData::setMatrix( 
    const QVector<double> &values, size_t numColumns )
{
    d_data->values = values;
    d_data->numColumns = numColumns;
    update();
}

const QVector<double> QwtMatrixRasterData::values() const
{
    return d_data->values;
}

size_t QwtMatrixRasterData::numColumns() const
{
    return d_data->numColumns;
}

size_t QwtMatrixRasterData::numRows() const
{
    return d_data->numRows;
}

void QwtMatrixRasterData::setRange( const QwtInterval &range )
{
    d_data->range = range;
}

QwtInterval QwtMatrixRasterData::range() const
{
    return d_data->range;
}

QSize QwtMatrixRasterData::rasterHint( const QRectF &rect ) const
{
    if ( d_data->resampleMode == NearestNeighbour )
    {
        const QRectF br = boundingRect();
        if ( !rect.isEmpty() && !br.isEmpty() &&
            d_data->numRows > 0 && d_data->numColumns > 0 )
        {
            const double rx = rect.width() / br.width() * d_data->numColumns;
            const double ry = rect.height() / br.height() * d_data->numRows;

            return QSize( qCeil( rx ), qCeil( ry ) );
        }
    }

    return QSize();
}

double QwtMatrixRasterData::value( double x, double y ) const
{
    const QRectF br = boundingRect();
    if ( !br.contains( x, y ) )
        return qQNaN();

    double value;

    switch(d_data->resampleMode)
    {
        case BilinearInterpolation:
        {
            const int col = int( (x - br.x() ) / d_data->dx );
            const int row = int( (y - br.y() ) / d_data->dy );

            const double *values = d_data->values.data();

            const double v00 = values[ row * d_data->numColumns + col ];
            const double v10 = values[ ( row + 1 ) * d_data->numColumns + col ];
            const double v01 = values[ row * d_data->numColumns + col + 1 ];
            const double v11 = values[ ( row + 1 ) * d_data->numColumns + col + 1 ];

            const double dx = ( br.x() + col * d_data->dx - x ) / d_data->dx;
            const double dy = ( br.y() + row * d_data->dy - y ) / d_data->dy;

            const double q0 = ( 1.0 - dx ) * v00 + dx * v10;
            const double q1 = ( 1.0 - dx ) * v01 + dx * v11;

            value = ( 1.0 - dy ) * q0 + dy * q1;
            break;
        }
        case NearestNeighbour:
        default:
        {
            const int row = qRound( (y - br.y() ) / d_data->dy );
            const int col = qRound( (x - br.x() ) / d_data->dx );

            value = d_data->values[ row * d_data->numColumns + col ];
        }
    }

    return value;
}

void QwtMatrixRasterData::update()
{
    d_data->numRows = 0;
    d_data->dx = 0.0;
    d_data->dy = 0.0;

    if ( d_data->numColumns > 0 )
    {
        d_data->numRows = d_data->values.size() / d_data->numColumns;

        const QRectF br = boundingRect();
        if ( !br.isEmpty() )
        {
            d_data->dx = br.width() / d_data->numColumns;
            d_data->dy = br.width() / d_data->numRows;
        }
    }
}
