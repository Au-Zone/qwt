/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_tradingcurve.h"
#include "qwt_scale_map.h"
#include "qwt_clipper.h"
#include "qwt_painter.h"
#include <qpainter.h>

static inline bool qwtIsSampleInside( const QwtOHLCSample &sample,
    double tMin, double tMax, double vMin, double vMax )
{
    const double t = sample.time;
    const QwtInterval interval = sample.boundingInterval();

    const bool isOffScreen = ( t < tMin ) || ( t > tMax )
        || ( interval.maxValue() < vMin ) || ( interval.minValue() > vMax );

    return !isOffScreen;
}

static inline void qwtDrawBar( QPainter *painter, 
    const QwtOHLCSample &sample, Qt::Orientation orientation, double width )
{
    const int w2 = qCeil( 0.5 * width );

    if ( orientation == Qt::Vertical )
    {
        QwtPainter::drawLine( painter,
            sample.time, sample.low, sample.time, sample.high );

        QwtPainter::drawLine( painter,
            sample.time - w2, sample.open, sample.time, sample.open );
        QwtPainter::drawLine( painter,
            sample.time + w2, sample.close, sample.time, sample.close );
    }
    else
    {
        QwtPainter::drawLine( painter, sample.low, sample.time,
            sample.high, sample.time );
        QwtPainter::drawLine( painter,
            sample.open, sample.time - w2, sample.open, sample.time );
        QwtPainter::drawLine( painter,
            sample.close, sample.time + w2, sample.close, sample.time );
    }
}

static inline void qwtDrawCandleStick( QPainter *painter,
    const QwtOHLCSample &sample, Qt::Orientation orientation, double width )
{
    const int w2 = qCeil( 0.5 * width );

    const double t = sample.time;
    const double v1 = qMin( sample.low, sample.high );
    const double v2 = qMin( sample.open, sample.close );
    const double v3 = qMax( sample.low, sample.high );
    const double v4 = qMax( sample.open, sample.close );

    if ( orientation == Qt::Vertical )
    {
        QwtPainter::drawLine( painter, t, v1, t, v2 ); 
        QwtPainter::drawLine( painter, t, v3, t, v4 ); 

        QRectF rect( t - w2, sample.open,
            2 * w2, sample.close - sample.open );

        QwtPainter::drawRect( painter, rect );
    }
    else
    {
        QwtPainter::drawLine( painter, v1, t, v2, t ); 
        QwtPainter::drawLine( painter, v3, t, v4, t ); 

        const QRectF rect( sample.open, t - w2,
            sample.close - sample.open, 2 * w2 );

        QwtPainter::drawRect( painter, rect );
    }
}

class QwtPlotTradingCurve::PrivateData
{
public:
    PrivateData():
        symbolStyle( QwtPlotTradingCurve::CandleStick ),
        symbolWidth( 6.0 ),
        paintAttributes( QwtPlotTradingCurve::ClipSymbols )
    {
        symbolBrush[0] = QBrush( Qt::white );
        symbolBrush[1] = QBrush( Qt::black );
    }

    QwtPlotTradingCurve::SymbolStyle symbolStyle;
    double symbolWidth;

    QPen symbolPen;
    QBrush symbolBrush[2]; // Increasing/Decreasing

    QwtPlotTradingCurve::PaintAttributes paintAttributes;
};

/*!
  Constructor
  \param title Title of the curve
*/
QwtPlotTradingCurve::QwtPlotTradingCurve( const QwtText &title ):
    QwtPlotSeriesItem<QwtOHLCSample>( title )
{
    init();
}

/*!
  Constructor
  \param title Title of the curve
*/
QwtPlotTradingCurve::QwtPlotTradingCurve( const QString &title ):
    QwtPlotSeriesItem<QwtOHLCSample>( QwtText( title ) )
{
    init();
}

//! Destructor
QwtPlotTradingCurve::~QwtPlotTradingCurve()
{
    delete d_data;
}

//! Initialize internal members
void QwtPlotTradingCurve::init()
{
    setItemAttribute( QwtPlotItem::Legend, true );
    setItemAttribute( QwtPlotItem::AutoScale, true );

    d_data = new PrivateData;
    d_series = new QwtTradingChartData();

    setZ( 19.0 );
}

//! \return QwtPlotItem::Rtti_PlotTradingCurve
int QwtPlotTradingCurve::rtti() const
{
    return QwtPlotTradingCurve::Rtti_PlotTradingCurve;
}

/*!
  Specify an attribute how to draw the curve

  \param attribute Paint attribute
  \param on On/Off
  \sa testPaintAttribute()
*/
void QwtPlotTradingCurve::setPaintAttribute(
    PaintAttribute attribute, bool on )
{
    if ( on )
        d_data->paintAttributes |= attribute;
    else
        d_data->paintAttributes &= ~attribute;
}

/*!
    \brief Return the current paint attributes
    \sa PaintAttribute, setPaintAttribute()
*/
bool QwtPlotTradingCurve::testPaintAttribute(
    PaintAttribute attribute ) const
{
    return ( d_data->paintAttributes & attribute );
}

/*!
  Initialize data with an array of samples.
  \param samples Vector of samples
*/
void QwtPlotTradingCurve::setSamples(
    const QVector<QwtOHLCSample> &samples )
{
    delete d_series;
    d_series = new QwtTradingChartData( samples );
    itemChanged();
}

void QwtPlotTradingCurve::setSymbolStyle( SymbolStyle style )
{
    if ( style != d_data->symbolStyle )
    {
        d_data->symbolStyle = style;
        itemChanged();
    }
}

QwtPlotTradingCurve::SymbolStyle QwtPlotTradingCurve::symbolStyle() const
{
    return d_data->symbolStyle;
}

void QwtPlotTradingCurve::setSymbolPen( const QPen &pen )
{
    if ( pen != d_data->symbolPen )
    {
        d_data->symbolPen = pen;
        itemChanged();
    }
}

QPen QwtPlotTradingCurve::symbolPen() const
{
    return d_data->symbolPen;
}

void QwtPlotTradingCurve::setSymbolBrush( 
    Direction direction, const QBrush &brush )
{
    if ( direction < 0 || direction >= 2 )
        return;

    if ( brush != d_data->symbolBrush[ direction ] )
    {
        d_data->symbolBrush[ direction ] = brush;
        itemChanged();
    }
}

QBrush QwtPlotTradingCurve::symbolBrush( Direction direction ) const
{
    if ( direction < 0 || direction >= 2 )
        return QBrush();

    return d_data->symbolBrush[ direction ];
}

/*!
  \return Bounding rectangle of all samples.
  For an empty series the rectangle is invalid.
*/
QRectF QwtPlotTradingCurve::boundingRect() const
{
    QRectF rect = QwtPlotSeriesItem<QwtOHLCSample>::boundingRect();
    if ( rect.isValid() && orientation() == Qt::Vertical )
        rect.setRect( rect.y(), rect.x(), rect.height(), rect.width() );

    return rect;
}

void QwtPlotTradingCurve::drawSeries( QPainter *painter,
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

    drawSymbols( painter, xMap, yMap, canvasRect, from, to );

    painter->restore();
}

void QwtPlotTradingCurve::drawSymbols( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, int from, int to ) const
{
    if ( d_data->symbolStyle == QwtPlotTradingCurve::NoSymbol )
        return;

    const QRectF tr = QwtScaleMap::invTransform( xMap, yMap, canvasRect );

    const QwtScaleMap *timeMap, *valueMap; 
    double tMin, tMax, vMin, vMax;

    const Qt::Orientation orient = orientation();
    if ( orient == Qt::Vertical )
    {
        timeMap = &xMap;
        valueMap = &yMap;

        tMin = tr.left();
        tMax = tr.right();
        vMin = tr.top();
        vMax = tr.bottom();
    }
    else
    {
        timeMap = &yMap;
        valueMap = &xMap;

        vMin = tr.left();
        vMax = tr.right();
        tMin = tr.top();
        tMax = tr.bottom();
    }

    const bool doClip = d_data->paintAttributes & ClipSymbols;
    const bool doAlign = QwtPainter::roundingAlignment( painter );

    QPen pen = d_data->symbolPen;
    pen.setCapStyle( Qt::FlatCap );

    painter->setPen( pen );

    for ( int i = from; i <= to; i++ )
    {
        const QwtOHLCSample s = sample( i );

        if ( !doClip || qwtIsSampleInside( s, tMin, tMax, vMin, vMax ) )
        {
            QwtOHLCSample translatedSample;

            translatedSample.time = timeMap->transform( s.time );
            translatedSample.open = valueMap->transform( s.open );
            translatedSample.high = valueMap->transform( s.high );
            translatedSample.low = valueMap->transform( s.low );
            translatedSample.close = valueMap->transform( s.close );

            const int brushIndex = ( s.open < s.close ) 
                ? QwtPlotTradingCurve::Increasing : QwtPlotTradingCurve::Decreasing;

            if ( doAlign )
            {
                translatedSample.time = qRound( translatedSample.time );
                translatedSample.open = qRound( translatedSample.open );
                translatedSample.high = qRound( translatedSample.high );
                translatedSample.low = qRound( translatedSample.low );
                translatedSample.close = qRound( translatedSample.close );
            }

            switch( d_data->symbolStyle )
            {
                case Bar:
                {
                    qwtDrawBar( painter, translatedSample, 
                        orient, d_data->symbolWidth );
                    break;
                }
                case CandleStick:
                {
                    painter->setBrush( d_data->symbolBrush[ brushIndex ] );
                    qwtDrawCandleStick( painter, translatedSample, 
                        orient, d_data->symbolWidth );
                    break;
                }
                default:
                {
                    if ( d_data->symbolStyle >= UserSymbol )
                    {
                        painter->setBrush( d_data->symbolBrush[ brushIndex ] );
                        drawUserSymbol( painter, 
                            d_data->symbolStyle, translatedSample );
                    }
                }
            }
        }
    }
}

void QwtPlotTradingCurve::drawUserSymbol( QPainter *painter, 
    SymbolStyle symbolStyle, const QwtOHLCSample &sample ) const
{
    Q_UNUSED( painter )
    Q_UNUSED( sample )
    Q_UNUSED( symbolStyle )
}

void QwtPlotTradingCurve::drawLegendIdentifier(
    QPainter *painter, const QRectF &rect ) const
{
    const double dim = qMin( rect.width(), rect.height() );

    QSizeF size( dim, dim );

    QRectF r( 0, 0, size.width(), size.height() );
    r.moveCenter( rect.center() );

    Q_UNUSED( painter );
}
