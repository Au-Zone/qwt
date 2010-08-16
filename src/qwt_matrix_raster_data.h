/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_MATRIX_RASTER_DATA_H
#define QWT_MATRIX_RASTER_DATA_H 1

#include "qwt_global.h"
#include "qwt_raster_data.h"
#include <qvector.h>

class QWT_EXPORT QwtMatrixRasterData: public QwtRasterData
{
public:
    enum ResampleMode
    {
        NearestNeighbour,
        BilinearInterpolation
    };

    QwtMatrixRasterData();
    QwtMatrixRasterData( const QRectF &boundingRect, const QwtInterval &range,
        const QVector<double> &values, size_t numColumns );

    virtual ~QwtMatrixRasterData();

    void setResampleMode(ResampleMode mode);
    ResampleMode resampleMode() const;

    virtual void setBoundingRect( const QRectF & );
    void setMatrix( const QVector<double> &values, size_t numColumns );
    
    const QVector<double> values() const;
    size_t numColumns() const;
    size_t numRows() const;

    void setRange( const QwtInterval & );
    virtual QwtInterval range() const;

    virtual QRectF pixelRect( const QRectF & ) const;

    virtual double value( double x, double y ) const;

private:
    void update();

    class PrivateData;
    PrivateData *d_data;
};

#endif
