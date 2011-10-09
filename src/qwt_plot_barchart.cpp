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
#include "qwt_plot.h"
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

static inline double qwtTransformWidth(
    const QwtScaleMap &map, double value, double width )
{
    const double w2 = 0.5 * width;

    const double v1 = map.transform( value - w2 );
    const double v2 = map.transform( value + w2 );

    return qAbs( v2 - v1 );
}

class QwtPlotBarChart::PrivateData
{
public:
    PrivateData():
        style( QwtPlotBarChart::Grouped ),
        layoutPolicy( QwtPlotBarChart::AutoAdjustSamples ),
        layoutHint( 0.5 ),
        spacing( 5 ),
        baseline( 0.0 ),
        symbol( NULL )
    {
    }

    QwtPlotBarChart::ChartStyle style;
    QwtPlotBarChart::LayoutPolicy layoutPolicy;
    double layoutHint;
    int spacing;
    double baseline;
    ChartAttributes chartAttributes;

    QwtColumnSymbol *symbol;
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
    setItemAttribute( QwtPlotItem::Margins, true );

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

void QwtPlotBarChart::setSymbol( QwtColumnSymbol *symbol )
{
    if ( symbol != d_data->symbol )
    {
        delete d_data->symbol;
        d_data->symbol = symbol;
        itemChanged();
    }
}

/*!
  \return Current symbol or NULL, when no symbol has been assigned
  \sa setSymbol()
*/
const QwtColumnSymbol *QwtPlotBarChart::symbol() const
{
    return d_data->symbol;
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

void QwtPlotBarChart::setChartAttribute( ChartAttribute attribute, bool on )
{
    if ( on )
        d_data->chartAttributes |= attribute;
    else
        d_data->chartAttributes &= ~attribute;
}

bool QwtPlotBarChart::testChartAttribute( ChartAttribute attribute ) const
{
    return ( d_data->chartAttributes & attribute );
}

void QwtPlotBarChart::setLayoutPolicy( LayoutPolicy policy )
{
    if ( policy != d_data->layoutPolicy )
    {
        d_data->layoutPolicy = policy;
        itemChanged();
    }
}

QwtPlotBarChart::LayoutPolicy QwtPlotBarChart::layoutPolicy() const
{
    return d_data->layoutPolicy;
}

void QwtPlotBarChart::setLayoutHint( double hint )
{
    hint = qMax( 0.0, hint );
    if ( hint != d_data->layoutHint )
    {
        d_data->layoutHint = hint;
        itemChanged();
    }
}

double QwtPlotBarChart::layoutHint() const
{
    return d_data->layoutHint;
}

void QwtPlotBarChart::setSpacing( int spacing )
{
    spacing = qMax( spacing, 0 );
    if ( spacing != d_data->spacing )
    {
        d_data->spacing = spacing;
        itemChanged();
    }
}

int QwtPlotBarChart::spacing() const
{
    return d_data->spacing;
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
    const size_t numSamples = d_series->size();

    if ( numSamples == 0 )
        return QwtPlotSeriesItem<QwtSetSample>::boundingRect();

    QRectF rect;

    if ( d_data->style != QwtPlotBarChart::Stacked )
    {
        rect = QwtPlotSeriesItem<QwtSetSample>::boundingRect();
    }
    else
    {
        double xMin, xMax, yMin, yMax;

        yMin = yMax = d_data->baseline;

        for ( size_t i = 0; i < numSamples; i++ )
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


    const QRectF br = d_series->boundingRect();
    const QwtInterval interval( br.left(), br.right() );

    painter->save();

    for ( int i = from; i <= to; i++ )
    {
        drawSample( painter, xMap, yMap,
            canvasRect, interval, i, sample( i ) );
    }

    painter->restore();
}

/*!
  Draw a sample

  \param painter Painter
  \param xMap x map
  \param yMap y map
  \param canvasRect Contents rect of the canvas
  \param boundingInterval Bounding interval of sample values
  \param from Index of the first point to be painted
  \param to Index of the last point to be painted

  \sa drawSeries()
*/
void QwtPlotBarChart::drawSample( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, const QwtInterval &boundingInterval,
    int index, const QwtSetSample& sample ) const
{
    if ( sample.set.size() <= 0 )
        return;

    double sampleW;

    if ( orientation() == Qt::Horizontal )
    {
        sampleW = sampleWidth( yMap, canvasRect.height(),
            boundingInterval.width(), sample );
    }
    else
    {
        sampleW = sampleWidth( xMap, canvasRect.width(),
            boundingInterval.width(), sample );
    }

    if ( d_data->style == Stacked )
    {
        drawStackedBars( painter, xMap, yMap,
            canvasRect, index, sampleW, sample );
    }
    else
    {
        drawGroupedBars( painter, xMap, yMap,
            canvasRect, index, sampleW, sample );
    }
}

void QwtPlotBarChart::drawGroupedBars( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, int index, double sampleWidth,
    const QwtSetSample& sample ) const
{
    Q_UNUSED( canvasRect );

    const int numBars = sample.set.size();
    if ( numBars == 0 )
        return;

    if ( orientation() == Qt::Vertical )
    {
        const double barWidth = sampleWidth / numBars;

        const double y1 = yMap.transform( d_data->baseline );
        const double x0 = xMap.transform( sample.value ) - 0.5 * sampleWidth;

        for ( int i = 0; i < numBars; i++ )
        {
            const double x1 = x0 + i * barWidth;
            const double x2 = x1 + barWidth;

            const double y2 = yMap.transform( sample.set[i] );

            QwtColumnRect bar;
            bar.direction = ( y1 < y2 ) ?
                QwtColumnRect::TopToBottom : QwtColumnRect::BottomToTop;

            bar.hInterval = QwtInterval( x1, x2 ).normalized();
            if ( i != 0 )
                bar.hInterval.setBorderFlags( QwtInterval::ExcludeMinimum );

            bar.vInterval = QwtInterval( y1, y2 ).normalized();

            drawBar( painter, index, i, bar );

            if ( d_data->chartAttributes & ShowLabels )
            {
                const QwtText text = label( index, i, sample );
                drawLabel( painter, index, i, bar, text );
            }
        }
    }
    else
    {
        const double barHeight = sampleWidth / numBars;

        const double x1 = xMap.transform( d_data->baseline );
        const double y0 = yMap.transform( sample.value ) - 0.5 * sampleWidth;

        for ( int i = 0; i < numBars; i++ )
        {
            double y1 = y0 + i * barHeight;
            double y2 = y1 + barHeight;

            double x2 = xMap.transform( sample.set[i] );

            QwtColumnRect bar;
            bar.direction = x1 < x2 ?
                QwtColumnRect::LeftToRight : QwtColumnRect::RightToLeft;

            bar.hInterval = QwtInterval( x1, x2 ).normalized();

            bar.vInterval = QwtInterval( y1, y2 );
            if ( i != 0 )
                bar.vInterval.setBorderFlags( QwtInterval::ExcludeMinimum );

            drawBar( painter, index, i, bar );

            if ( d_data->chartAttributes & ShowLabels )
            {
                const QwtText text = label( index, i, sample );
                drawLabel( painter, index, i, bar, text );
            }
        }
    }
}

void QwtPlotBarChart::drawStackedBars( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, int index, 
    double sampleWidth, const QwtSetSample& sample ) const
{
    Q_UNUSED( canvasRect ); // clipping the bars ?

    const int numBars = sample.set.size();
    if ( numBars == 0 )
        return;

    QwtInterval::BorderFlag borderFlags = QwtInterval::IncludeBorders;

    if ( orientation() == Qt::Vertical )
    {
        const double x1 = xMap.transform( sample.value ) - 0.5 * sampleWidth;
        const double x2 = x1 + sampleWidth;

        const bool increasing = qwtIsIncreasing( yMap, sample.set );

        QwtColumnRect bar;
        bar.direction = increasing ?
            QwtColumnRect::TopToBottom : QwtColumnRect::BottomToTop;

        bar.hInterval = QwtInterval( x1, x2 ).normalized();

        double sum = d_data->baseline;

        const int numBars = sample.set.size();
        for ( int i = 0; i < numBars; i++ )
        {
            const double si = sample.set[ i ];
            if ( si == 0.0 )
                continue;

            const double y1 = yMap.transform( sum );
            const double y2 = yMap.transform( sum + si );

            if ( ( y2 > y1 ) != increasing )
            {
                // stacked bars need to be in the same direction
                continue;
            }

            bar.vInterval = QwtInterval( y1, y2 ).normalized();
            bar.vInterval.setBorderFlags( borderFlags );

            drawBar( painter, index, i, bar );

            sum += si;

            if ( increasing )
                borderFlags = QwtInterval::ExcludeMinimum;
            else
                borderFlags = QwtInterval::ExcludeMaximum;
        }
    }
    else
    {
        const double y1 = yMap.transform( sample.value ) - 0.5 * sampleWidth;
        const double y2 = y1 + sampleWidth;

        const bool increasing = qwtIsIncreasing( xMap, sample.set );

        QwtColumnRect bar;
        bar.direction = increasing ?
            QwtColumnRect::LeftToRight : QwtColumnRect::RightToLeft;
        bar.vInterval = QwtInterval( y1, y2 ).normalized();

        double sum = d_data->baseline;

        for ( int i = 0; i < sample.set.size(); i++ )
        {
            const double si = sample.set[ i ];
            if ( si == 0.0 )
                continue;

            const double x1 = xMap.transform( sum );
            const double x2 = xMap.transform( sum + si );

            if ( ( x2 > x1 ) != increasing )
            {
                // stacked bars need to be in the same direction
                continue;
            }

            bar.hInterval = QwtInterval( x1, x2 ).normalized();
            bar.hInterval.setBorderFlags( borderFlags );

            drawBar( painter, index, i, bar );

            sum += si;

            if ( increasing )
                borderFlags = QwtInterval::ExcludeMinimum;
            else
                borderFlags = QwtInterval::ExcludeMaximum;
        }
    }
}

void QwtPlotBarChart::drawBar( QPainter *painter,
    int sampleIndex, int barIndex, const QwtColumnRect &rect ) const
{
    Q_UNUSED( sampleIndex );

    static Qt::GlobalColor colors[] =
        { Qt::blue, Qt::red, Qt::green, Qt::magenta, Qt::yellow };

    const int colorIndex = barIndex % ( sizeof( colors ) / sizeof( colors[0] ) );

    if ( d_data->symbol &&
        ( d_data->symbol->style() != QwtColumnSymbol::NoStyle ) )
    {
        d_data->symbol->setPalette( QPalette( colors[ colorIndex ] ) );
        d_data->symbol->draw( painter, rect );
    }
    else
    {
        QRectF r = rect.toRect();
        if ( QwtPainter::roundingAlignment( painter ) )
        {
            r.setLeft( qRound( r.left() ) );
            r.setRight( qRound( r.right() ) );
            r.setTop( qRound( r.top() ) );
            r.setBottom( qRound( r.bottom() ) );
        }

        painter->setPen( QPen( Qt::black, 1 ) );
        painter->setBrush( colors[ colorIndex ] );
        QwtPainter::drawRect( painter, r );
    }
}

void QwtPlotBarChart::drawLabel( QPainter *painter, int sampleIndex,
    int barIndex, const QwtColumnRect &rect, const QwtText &text ) const
{
    Q_UNUSED( painter );
    Q_UNUSED( sampleIndex );
    Q_UNUSED( barIndex );
    Q_UNUSED( rect );
    Q_UNUSED( text );
}

void QwtPlotBarChart::drawLegendIdentifier(
    QPainter *painter, const QRectF &rect ) const
{
    Q_UNUSED( painter );
    Q_UNUSED( rect );
}

double QwtPlotBarChart::sampleWidth( const QwtScaleMap &map,
    double canvasSize, double boundingSize,
    const QwtSetSample& sample ) const
{
    double width;

    switch( d_data->layoutPolicy )
    {
        case ScaleSamplesToAxes:
        {
            width = qwtTransformWidth( map,
                sample.value, d_data->layoutHint );
            break;
        }
        case ScaleSampleToCanvas:
        {
            width = canvasSize * d_data->layoutHint;
            break;
        }
        case FixedSampleSize:
        {
            width = d_data->layoutHint;
            break;
        }
        case AutoAdjustSamples:
        default:
        {
            const size_t numSamples = dataSize();

            double w = 1.0;
            if ( numSamples > 1 )
            {
                w = qAbs( boundingSize / ( numSamples - 1 ) );
            }

            width = qwtTransformWidth( map, sample.value, w );
            width -= d_data->spacing;
        }
    }

    return width;
}

void QwtPlotBarChart::getCanvasMarginHint( const QwtScaleMap &xMap, 
    const QwtScaleMap &yMap, const QRectF &canvasRect,
    double &left, double &top, double &right, double &bottom ) const
{
    const size_t numSamples = d_series->size();

    double margin = -1.0;

    switch( layoutPolicy() )
    {
        case ScaleSampleToCanvas:
        {
            if ( orientation() == Qt::Vertical )
                margin = 0.5 * canvasRect.width() * d_data->layoutHint;
            else
                margin = 0.5 * canvasRect.height() * d_data->layoutHint;

            break;
        }
        case FixedSampleSize:
        {
            margin = 0.5 * d_data->layoutHint;
            break;
        }
        case AutoAdjustSamples:
        case ScaleSamplesToAxes:
        default:
        {
            if ( numSamples <= 0 || plot() == NULL )
                break;

            // doesn't work for nonlinear scales

            double spacing = 0.0;
            double sampleWidthS = 1.0;

            if ( layoutPolicy() == ScaleSamplesToAxes )
            {
                sampleWidthS = d_data->layoutHint;
            }
            else
            {
                spacing = d_data->spacing;

                if ( numSamples > 1 )
                {
                    const QRectF br = d_series->boundingRect();
                    sampleWidthS = qAbs( br.width() / ( numSamples - 1 ) );
                }
            }

            QwtScaleMap map;
            if ( orientation() == Qt::Vertical )
            {
                map = xMap;
                map.setPaintInterval( 0.0, canvasRect.width() );
            }
            else
            {
                map = yMap;
                map.setPaintInterval( 0.0, canvasRect.height() );
            }

            const double value = d_series->sample( 0 ).value;

            double sampleWidthP;

            // first approximation
            sampleWidthP = qwtTransformWidth( map, value, sampleWidthS );
            margin = 0.5 * ( sampleWidthP - spacing );

            // calculating the same including the approximated margins
            map.setPaintInterval( map.p1() + margin, map.p2() - margin );

            sampleWidthP = qwtTransformWidth( map, value, sampleWidthS );
            margin = 0.5 * ( sampleWidthP - spacing );
        }
    }

    if ( orientation() == Qt::Vertical )
    {
        left = right = margin;
        top = bottom = -1.0; // no hint
    }
    else
    {
        left = right = -1.0; // no hint
        top = bottom = margin;
    }
}

QwtText QwtPlotBarChart::label(
    int sampleIndex, int barIndex, const QwtSetSample& sample ) const
{
    Q_UNUSED( sampleIndex );

    QString labelText;
    if ( barIndex >= 0 && barIndex <= sample.set.size() )
        labelText.setNum( sample.set[ barIndex ] );

    return QwtText( labelText );
}
