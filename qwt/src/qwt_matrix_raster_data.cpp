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

    inline double value(size_t row, size_t col) const
    {
        return values.data()[ row * numColumns + col ];
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

void QwtMatrixRasterData::setResampleMode(ResampleMode mode)
{
    d_data->resampleMode = mode;
}

QwtMatrixRasterData::ResampleMode QwtMatrixRasterData::resampleMode() const
{
    return d_data->resampleMode;
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

QRectF QwtMatrixRasterData::pixelHint( const QRectF & ) const
{
    QRectF rect;
    if ( d_data->resampleMode == NearestNeighbour )
    {
        rect = QRectF( boundingRect().topLeft(), 
            QSizeF(d_data->dx, d_data->dy) );
    }

    return rect;
}

double QwtMatrixRasterData::value( double x, double y ) const
{
    const QRectF br = boundingRect();
    if ( x < br.left() || y < br.top() 
        || x >= br.right() || y >= br.bottom() )
    {
        // exluding the top + right borders !
        return qQNaN();
    }

    double value;

    switch( d_data->resampleMode )
    {
        case BilinearInterpolation:
        {
            int col1 = qRound( (x - br.x() ) / d_data->dx ) - 1;
            int row1 = qRound( (y - br.y() ) / d_data->dy ) - 1;
            int col2 = col1 + 1;
            int row2 = row1 + 1;

            if ( col1 < 0 )
                col1 = col2;
            else if ( col2 >= (int)d_data->numColumns )
                col2 = col1;

            if ( row1 < 0 )
                row1 = row2;
            else if ( row2 >= (int)d_data->numRows )
                row2 = row1;

            const double ddx = 
                qAbs( ( br.x() + ( col1 + 0.5 ) * d_data->dx - x ) / d_data->dx );
            const double ddy =  
                qAbs( ( br.y() + ( row1 + 0.5 ) * d_data->dy - y ) / d_data->dy );

            const double qx = ( d_data->dx - ddx ) / d_data->dx;
            const double qy = ( d_data->dy - ddy ) / d_data->dy;

            const double v11 = d_data->value( row1, col1 );
            const double v21 = d_data->value( row1, col2 );
            const double v12 = d_data->value( row2, col1 );
            const double v22 = d_data->value( row2, col2 );

            const double r1 = qx * v11 + ( 1.0 - qx ) * v21;
            const double r2 = qx * v12 + ( 1.0 - qx ) * v22;

            value = qy * r1 + ( 1.0 - qy ) * r2;
            break;
        }
        case NearestNeighbour:
        default:
        {
            const int row = int( (y - br.y() ) / d_data->dy );
            const int col = int( (x - br.x() ) / d_data->dx );

            value = d_data->value( row, col );
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
