/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qwt_point_mapper.h>
#include <qwt_scale_map.h>
#include <qwt_pixel_matrix.h>
#include <qpolygon.h>
#include <qimage.h>

class QwtPointMapper::PrivateData
{
public:
    PrivateData():
        boundingRect( 0.0, 0.0, -1.0, -1.0 )
    {
    }

    QRectF boundingRect;
    QwtPointMapper::TransformationFlags flags;
};

QwtPointMapper::QwtPointMapper()
{
    d_data = new PrivateData();
}

QwtPointMapper::~QwtPointMapper()
{
    delete d_data;
}

void QwtPointMapper::setFlags( TransformationFlags flags )
{
    d_data->flags = flags;
}

QwtPointMapper::TransformationFlags QwtPointMapper::flags() const
{
    return d_data->flags;
}

void QwtPointMapper::setFlag( TransformationFlag flag, bool on )
{
    if ( on )
        d_data->flags |= flag;
    else
        d_data->flags &= ~flag;
}

bool QwtPointMapper::testFlag( TransformationFlag flag ) const
{
    return d_data->flags & flag;
}

void QwtPointMapper::setBoundingRect( const QRectF &rect )
{
    d_data->boundingRect = rect;
}

QRectF QwtPointMapper::boundingRect() const
{
    return d_data->boundingRect;
}

QPolygonF QwtPointMapper::toPolygonF(
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QwtSeriesData<QPointF> *series, int from, int to ) const
{
    QPolygonF polyline( to - from + 1 );
    QPointF *points = polyline.data();

    if ( d_data->flags & RoundPoints )
    {
        if ( d_data->flags & WeedOutPoints )
        {
            const QPointF sample0 = series->sample( from );
            points[0].rx() = qRound( xMap.transform( sample0.x() ) );
            points[0].ry() = qRound( yMap.transform( sample0.y() ) );

            int pos = 0;
            for ( int i = from + 1; i <= to; i++ )
            {
                const QPointF sample = series->sample( i );

                const double x = qRound( xMap.transform( sample.x() ) );
                const double y = qRound( yMap.transform( sample.y() ) );

                if ( points[pos].x() != x || points[pos].y() != y )
                {
                    pos++;
                    points[pos].rx() = x;
                    points[pos].ry() = y;
                }
            }

            polyline.resize( pos + 1 );
        }
        else
        {
            for ( int i = from; i <= to; i++ )
            {
                const QPointF sample = series->sample( i );

                const int x = qRound( xMap.transform( sample.x() ) );
                const int y = qRound( yMap.transform( sample.y() ) );

                points[i - from].rx() = static_cast<double>( x );
                points[i - from].ry() = static_cast<double>( y );
            }
        }
    }
    else
    {
        if ( d_data->flags & WeedOutPoints )
        {
            const QPointF sample0 = series->sample( from );
            points[0].rx() = qRound( xMap.transform( sample0.x() ) );
            points[0].ry() = qRound( yMap.transform( sample0.y() ) );

            int pos = 0;
            for ( int i = from + 1; i <= to; i++ )
            {
                const QPointF sample = series->sample( i );

                const double x = xMap.transform( sample.x() );
                const double y = yMap.transform( sample.y() );

                // we could use qFuzzyCompare ?
                if ( points[pos].x() != x || points[pos].y() != y )
                {
                    pos++;
                    points[pos].rx() = x;
                    points[pos].ry() = y;
                }
            }

            polyline.resize( pos + 1 );
        }
        else
        {
            for ( int i = from; i <= to; i++ )
            {
                const QPointF sample = series->sample( i );
                points[i - from].rx() = xMap.transform( sample.x() );
                points[i - from].ry() = yMap.transform( sample.y() );
            }
        }
    }

    return polyline;
}

QPolygon QwtPointMapper::toPolygon(
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QwtSeriesData<QPointF> *series, int from, int to ) const
{
    QPolygon polyline( to - from + 1 );
    QPoint *points = polyline.data();

    if ( d_data->flags & WeedOutPoints )
    {
        const QPointF sample0 = series->sample( from );
        points[0].rx() = qRound( xMap.transform( sample0.x() ) );
        points[0].ry() = qRound( yMap.transform( sample0.y() ) );

        int pos = 0;
        for ( int i = from + 1; i <= to; i++ )
        {
            const QPointF sample = series->sample( i );

            const int x = qRound( xMap.transform( sample.x() ) );
            const int y = qRound( yMap.transform( sample.y() ) );

            if ( points[pos].x() != x && points[pos].y() != y )
            {
                pos++;
                points[pos].rx() = x;
                points[pos].ry() = y;
            }
        }

        polyline.resize( pos + 1 );
    }
    else
    {
        for ( int i = from; i <= to; i++ )
        {
            const QPointF sample = series->sample( i );

#if 1
            points[i - from].rx() = qRound( xMap.transform( sample.x() ) );
            points[i - from].ry() = qRound( yMap.transform( sample.y() ) );
#else
            // A little bit faster, but differs from qRound
            // for negative values. Should be no problem as we are
            // rounding coordinates, where negative values are clipped off anyway
            // ( at least when there is no painter transformation )

            points[i - from].rx() =
                static_cast<int>( xMap.transform( sample.x() ) + 0.5 );
            points[i - from].ry() =
                static_cast<int>( yMap.transform( sample.y() ) + 0.5 );
#endif
        }
    }

    return polyline;
}

QPolygon QwtPointMapper::toPoints(
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QwtSeriesData<QPointF> *series, int from, int to ) const
{
    QPolygon polygon( to - from + 1 );
    QPoint *points = polygon.data();
    int numPoints = 0;

    if ( ( d_data->flags & WeedOutPoints )
            && !d_data->boundingRect.isEmpty() )
    {
        const QRect rect = d_data->boundingRect.toAlignedRect();

        QwtPixelMatrix pixelMatrix( rect );

        for ( int i = from; i <= to; i++ )
        {
            const QPointF sample = series->sample( i );

            const QPoint pos(
                qRound( xMap.transform( sample.x() ) ),
                qRound( yMap.transform( sample.y() ) )
            );

            if ( pixelMatrix.testAndSetPixel( pos, true ) == false )
                points[ numPoints++ ] = pos;
        }

        polygon.resize( numPoints );
    }
    else
    {
        for ( int i = from; i <= to; i++ )
        {
            const QPointF sample = series->sample( i );

            const int x = qRound( xMap.transform( sample.x() ) );
            const int y = qRound( yMap.transform( sample.y() ) );

            points[ numPoints ].rx() = x;
            points[ numPoints ].ry() = y;

            numPoints++;
        }
    }

    return polygon;
}

QPolygonF QwtPointMapper::toPointsF(
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QwtSeriesData<QPointF> *series, int from, int to ) const
{
    const bool on = d_data->flags & WeedOutPoints;
    d_data->flags &= ~WeedOutPoints;

    QPolygonF points = toPolygonF( xMap, yMap, series, from, to );

    if ( on )
        d_data->flags |= WeedOutPoints;

    return points;
}

QImage QwtPointMapper::toImage(
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QwtSeriesData<QPointF> *series, int from, int to, QRgb rgb ) const
{
    const QRect rect = d_data->boundingRect.toAlignedRect();

    QImage image( rect.size(), QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    const int w = image.width();
    const int h = image.height();

    const int x0 = rect.x();
    const int y0 = rect.y();

    QRgb *bits = reinterpret_cast<QRgb *>( image.bits() );
    for ( int i = from; i <= to; i++ )
    {
        const QPointF sample = series->sample( i );

        const int x = qRound( xMap.transform( sample.x() ) ) - x0;
        const int y = qRound( yMap.transform( sample.y() ) ) - y0;

        if ( x >= 0 && x < w && y >= 0 && y < w )
            bits[ y * h + x ] = rgb;
    }

    return image;
}

