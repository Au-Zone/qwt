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

/*!
  \brief A class representing a matrix of values as raster data

  QwtMatrixRasterData implements an interface for a matrix of
  equidistant values, that can be used by a QwtPlotRasterItem. 
  It implements a couple of resampling algorithms, to provide
  values for positions, that or not on the value matrix.
*/
class QWT_EXPORT QwtMatrixRasterData: public QwtRasterData
{
public:
    /*!
      Resampling algorithm

      - NearestNeighbour\n
        Return the value from the matrix, that is nearest to the
        the requested position.

      - BilinearInterpolation\n
        Interpolate the value from the distances and values of the 
        4 surrounding in the matrix,

     The default setting is NearestNeighbour;
    */
    enum ResampleMode
    {
        NearestNeighbour,
        BilinearInterpolation
    };

    QwtMatrixRasterData();
    virtual ~QwtMatrixRasterData();

    void setResampleMode(ResampleMode mode);
    ResampleMode resampleMode() const;

    virtual void setInterval( Qt::Axis, const QwtInterval & );
    void setValueMatrix( const QVector<double> &values, size_t numColumns );
    
    const QVector<double> valueMatrix() const;
    size_t numColumns() const;
    size_t numRows() const;

    virtual QRectF pixelHint( const QRectF & ) const;

    virtual double value( double x, double y ) const;

private:
    void update();

    class PrivateData;
    PrivateData *d_data;
};

#endif
