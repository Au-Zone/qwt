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

class QwtPlotTradingCurve::PrivateData
{
public:
    PrivateData():
        symbolStyle( QwtPlotTradingCurve::CandleStick )
    {
    }

    QwtPlotTradingCurve::SymbolStyle symbolStyle;
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
    const QRectF tr = QwtScaleMap::invTransform( xMap, yMap, canvasRect );

#if 0
    const double xMin = tr.left();
    const double xMax = tr.right();
    const double yMin = tr.top();
    const double yMax = tr.bottom();

    const bool doClip = true;
#endif

    for ( int i = from; i <= to; i++ )
    {
        const QwtOHLCSample s = sample( i );

        if ( orientation() == Qt::Vertical )
        {
            QwtOHLCSample trSample;

            trSample.time = xMap.transform( s.time );
            trSample.open = yMap.transform( s.open );
            trSample.high = yMap.transform( s.high );
            trSample.low = yMap.transform( s.low );
            trSample.close = yMap.transform( s.close );
        }
        else
        {
            QwtOHLCSample trSample;

            trSample.time = yMap.transform( s.time );
            trSample.open = xMap.transform( s.open );
            trSample.high = xMap.transform( s.high );
            trSample.low = xMap.transform( s.low );
            trSample.close = xMap.transform( s.close );
        }
    }

    Q_UNUSED( painter );
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
