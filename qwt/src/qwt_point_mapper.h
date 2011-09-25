/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2003   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POINT_MAPPER_H
#define QWT_POINT_MAPPER_H

#include "qwt_global.h"
#include "qwt_series_data.h"
#include <qimage.h>

class QwtScaleMap;
class QPolygonF;
class QPolygon;

class QWT_EXPORT QwtPointMapper
{
public:
    enum TransformationFlag
    {
        RoundPoints = 0x01,
        WeedOutPoints = 0x02
    };

    typedef QFlags<TransformationFlag> TransformationFlags;

    QwtPointMapper();
    ~QwtPointMapper();

    void setFlags( TransformationFlags );
    TransformationFlags flags() const;

    void setFlag( TransformationFlag, bool on = true );
    bool testFlag( TransformationFlag ) const;

    void setBoundingRect( const QRectF & );
    QRectF boundingRect() const;

    QPolygonF toPolygonF( const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                          const QwtSeriesData<QPointF> *series, int from, int to ) const;

    QPolygon toPolygon( const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                        const QwtSeriesData<QPointF> *series, int from, int to ) const;

    QPolygon toPoints( const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                       const QwtSeriesData<QPointF> *series, int from, int to ) const;

    QPolygonF toPointsF( const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                         const QwtSeriesData<QPointF> *series, int from, int to ) const;

    QImage toImage( const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                    const QwtSeriesData<QPointF> *series, int from, int to, QRgb rgb ) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPointMapper::TransformationFlags )

#endif
