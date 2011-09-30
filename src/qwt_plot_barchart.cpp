/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_barchart.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include "qwt_column_symbol.h"
#include <qpainter.h>
#include <qpalette.h>

inline static bool qwtIsIncreasing(
    const QwtScaleMap &map, const QVector<double> &values )
{
    bool isInverting = map.isInverting();

    for ( int i = 0; i < values.size(); i++ )
    {
        const double y = values[ i ];
        if ( y != 0.0 )
            return ( map.isInverting() != ( y > 0.0 ) );
    }

    return !isInverting;
}

class QwtPlotBarChart::PrivateData
{
public:
    PrivateData():
        style( QwtPlotBarChart::Grouped ),
        barWidth( 0.5 ),
        baseline( 0.0 )
    {
    }

    QwtPlotBarChart::ChartStyle style;
    double barWidth;
    double baseline;
};

QwtPlotBarChart::QwtPlotBarChart( const QwtText &title ):
    QwtPlotSeriesItem<QwtSetSample>( title )
{
    init();
}

QwtPlotBarChart::QwtPlotBarChart( const QString &title ):
    QwtPlotSeriesItem<QwtSetSample>( QwtText( title ) )
{
    init();
}

QwtPlotBarChart::~QwtPlotBarChart()
{
    delete d_data;
}

void QwtPlotBarChart::init()
{
    setItemAttribute( QwtPlotItem::Legend, true );
    setItemAttribute( QwtPlotItem::AutoScale, true );

    d_data = new PrivateData;
    d_series = new QwtSetSeriesData();

    setZ( 19.0 );
}

//! \return QwtPlotItem::Rtti_PlotBarChart
int QwtPlotBarChart::rtti() const
{
    return QwtPlotItem::Rtti_PlotBarChart;
}

void QwtPlotBarChart::setSamples(
    const QVector<QwtSetSample> &samples )
{
    delete d_series;
    d_series = new QwtSetSeriesData( samples );
    itemChanged();
}

void QwtPlotBarChart::setSamples(
    const QVector< QVector<double> > &samples )
{
#if 1
    QVector<QwtSetSample> s;
    for ( int i = 0; i < samples.size(); i++ )
        s += QwtSetSample( i, samples[ i ] );

    delete d_series;
    d_series = new QwtSetSeriesData( s );
#endif

    itemChanged();
}

void QwtPlotBarChart::setSamples( const QVector<double> &values )
{
#if 1
    QVector<QwtSetSample> s;
    for ( int i = 0; i < values.size(); i++ )
    {
        QVector<double> set;
        set += values[ i ];

        s += QwtSetSample( i, set );
    }

    delete d_series;
    d_series = new QwtSetSeriesData( s );
#endif

    itemChanged();
}
void QwtPlotBarChart::setStyle( ChartStyle style )
{
    if ( style != d_data->style )
    {
        d_data->style = style;
        itemChanged();
    }
}

QwtPlotBarChart::ChartStyle QwtPlotBarChart::style() const
{
    return d_data->style;
}

void QwtPlotBarChart::setBarWidth( double width )
{
    width = qMax( 0.0, width );
    if ( width != d_data->barWidth )
    {
        d_data->barWidth = width;
        itemChanged();
    }
}

double QwtPlotBarChart::barWidth() const
{
    return d_data->barWidth;
}

void QwtPlotBarChart::setBaseline( double value )
{
    if ( value != d_data->baseline )
    {
        d_data->baseline = value;
        itemChanged();
    }
}

double QwtPlotBarChart::baseline() const
{
    return d_data->baseline;
}

QRectF QwtPlotBarChart::boundingRect() const
{
    QRectF rect;

    if ( d_data->style != QwtPlotBarChart::Stacked
            || d_series->size() == 0 )
    {
        rect = QwtPlotSeriesItem<QwtSetSample>::boundingRect();
    }
    else
    {
        double xMin, xMax, yMin, yMax;

        yMin = yMax = d_data->baseline;

        for ( size_t i = 0; i < d_series->size(); i++ )
        {
            const QwtSetSample sample = d_series->sample( i );
            if ( i == 0 )
            {
                xMin = xMax = sample.value;
            }
            else
            {
                xMin = qMin( xMin, sample.value );
                xMax = qMax( xMax, sample.value );
            }

            const double y = d_data->baseline + sample.added();

            yMin = qMin( yMin, y );
            yMax = qMax( yMax, y );
        }
        rect.setRect( xMin, yMin, xMax - xMin, yMax - yMin );
    }

    if ( rect.isValid() && ( orientation() == Qt::Horizontal ) )
        rect.setRect( rect.y(), rect.x(), rect.height(), rect.width() );

    return rect;
}

/*!
  Draw an interval of the bar chart

  \param painter Painter
  \param xMap Maps x-values into pixel coordinates.
  \param yMap Maps y-values into pixel coordinates.
  \param canvasRect Contents rect of the canvas
  \param from Index of the first point to be painted
  \param to Index of the last point to be painted. If to < 0 the
         curve will be painted to its last point.

  \sa drawSymbols()
*/
void QwtPlotBarChart::drawSeries( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, int from, int to ) const
{
    if ( to < 0 )
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from > to )
        return;

    painter->save();

    for ( int i = from; i <= to; i++ )
        drawSample( painter, xMap, yMap, canvasRect, sample( i ) );

    painter->restore();
}

/*!
  Draw a sample

  \param painter Painter
  \param xMap x map
  \param yMap y map
  \param canvasRect Contents rect of the canvas
  \param from Index of the first point to be painted
  \param to Index of the last point to be painted

  \sa drawSeries()
*/
void QwtPlotBarChart::drawSample( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, const QwtSetSample& sample ) const
{
    if ( sample.set.size() <= 0 )
        return;

    if ( d_data->style == Stacked )
        drawStackedBars( painter, xMap, yMap, canvasRect, sample );
    else
        drawGroupedBars( painter, xMap, yMap, canvasRect, sample );
}

void QwtPlotBarChart::drawGroupedBars( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, const QwtSetSample& sample ) const
{
    Q_UNUSED( canvasRect );

    const double width2 = 0.5 * d_data->barWidth * sample.set.size();

    if ( orientation() == Qt::Vertical )
    {
        const double y1 = yMap.transform( d_data->baseline );

        for ( int i = 0; i < sample.set.size(); i++ )
        {
            const double v1 = sample.value - width2 + i * d_data->barWidth;
            const double v2 = v1 + d_data->barWidth;

            const double x1 = xMap.transform( v1 );
            const double x2 = xMap.transform( v2 );
            const double y2 = yMap.transform( sample.set[i] );

            QwtColumnRect bar;
            bar.direction = y1 < y2 ?
                            QwtColumnRect::TopToBottom : QwtColumnRect::BottomToTop;

            bar.hInterval = QwtInterval( x1, x2 ).normalized();
            bar.vInterval = QwtInterval( y1, y2 ).normalized();

            drawBar( painter, i, bar );
        }
    }
    else
    {
        const double x1 = xMap.transform( d_data->baseline );

        for ( int i = 0; i < sample.set.size(); i++ )
        {
            const double v1 = sample.value - width2 + i * d_data->barWidth;
            const double v2 = v1 + d_data->barWidth;

            const double y1 = yMap.transform( v1 );
            const double y2 = yMap.transform( v2 );
            const double x2 = xMap.transform( sample.set[i] );

            QwtColumnRect bar;
            bar.direction = x1 < x2 ?
                            QwtColumnRect::LeftToRight : QwtColumnRect::RightToLeft;

            bar.hInterval = QwtInterval( x1, x2 ).normalized();
            bar.vInterval = QwtInterval( y1, y2 ).normalized();

            drawBar( painter, i, bar );
        }
    }
}

void QwtPlotBarChart::drawStackedBars( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, const QwtSetSample& sample ) const
{
    Q_UNUSED( canvasRect ); // clipping the bars ?

    const double bw2 = 0.5 * d_data->barWidth;

    if ( orientation() == Qt::Vertical )
    {
        const double x1 = xMap.transform( sample.value - bw2 );
        const double x2 = xMap.transform( sample.value + bw2 );

        const bool increasing = qwtIsIncreasing( yMap, sample.set );

        QwtColumnRect bar;
        bar.direction = increasing ?
                        QwtColumnRect::TopToBottom : QwtColumnRect::BottomToTop;
        bar.hInterval = QwtInterval( x1, x2 ).normalized();

        double sum = d_data->baseline;

        for ( int i = 0; i < sample.set.size(); i++ )
        {
            const double si = sample.set[ i ];
            if ( si != 0.0 )
            {
                const double y1 = yMap.transform( sum );
                const double y2 = yMap.transform( sum + si );
                if ( ( y2 > y1 ) != increasing )
                {
                    // stacked bars need to be in the same direction
                    continue;
                }

                bar.vInterval = QwtInterval( y1, y2 ).normalized();

                drawBar( painter, i, bar );

                sum += si;
            }
        }
    }
    else
    {
        const double y1 = yMap.transform( sample.value - bw2 );
        const double y2 = yMap.transform( sample.value + bw2 );

        const bool increasing = qwtIsIncreasing( xMap, sample.set );

        QwtColumnRect bar;
        bar.direction = increasing ?
                        QwtColumnRect::LeftToRight : QwtColumnRect::RightToLeft;
        bar.vInterval = QwtInterval( y1, y2 ).normalized();

        double sum = d_data->baseline;

        for ( int i = 0; i < sample.set.size(); i++ )
        {
            const double si = sample.set[ i ];
            if ( si != 0.0 )
            {
                const double x1 = xMap.transform( sum );
                const double x2 = xMap.transform( sum + si );
                if ( ( x2 > x1 ) != increasing )
                {
                    // stacked bars need to be in the same direction
                    continue;
                }

                bar.hInterval = QwtInterval( x1, x2 ).normalized();
                drawBar( painter, i, bar );

                sum += si;
            }
        }
    }
}

void QwtPlotBarChart::drawBar( QPainter *painter,
    int index, const QwtColumnRect &rect ) const
{
    static Qt::GlobalColor colors[] =
    { Qt::blue, Qt::red, Qt::green, Qt::magenta, Qt::yellow };

    const int colorIndex = index % ( sizeof( colors ) / sizeof( colors[0] ) );

    QwtColumnSymbol symbol( QwtColumnSymbol::Box );
    symbol.setLineWidth( 1 );
    symbol.setFrameStyle( QwtColumnSymbol::Plain );

    symbol.setPalette( QPalette( colors[ colorIndex ] ) );
    symbol.draw( painter, rect );
}

void QwtPlotBarChart::drawLegendIdentifier(
    QPainter *painter, const QRectF &rect ) const
{
    Q_UNUSED( painter );
    Q_UNUSED( rect );
}
