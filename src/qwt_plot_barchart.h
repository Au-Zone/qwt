/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_BAR_CHART_H
#define QWT_PLOT_BAR_CHART_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"
#include "qwt_series_data.h"

class QwtColumnRect;
class QwtColumnSymbol;

class QWT_EXPORT QwtPlotBarChart: public QwtPlotSeriesItem<QwtSetSample>
{
public:
    enum ChartAttribute
    {
        ShowLabels = 0x01
    };

    typedef QFlags<ChartAttribute> ChartAttributes;

    enum ChartStyle
    {
        Stacked,
        Grouped
    };

    /*!
        \brief Mode how to calculate the bar width

        setLayoutPolicy(), setLayoutHint()
     */
    enum LayoutPolicy
    {
        /*!
          The sample width is calculated by deviding the bounding rectangle
          by the number of samples.

          \sa boundingRectangle()
          \note The layoutHint() is ignored
         */
        AutoAdjustSamples,

        /*!
          The barWidthHint() defines an interval in axis coordinates
         */
        ScaleSamplesToAxes,

        /*!
          The bar width is calculated by multiplying the barWidthHint()
          with the height or width of the canvas.

          \sa boundingRectangle()
         */
        ScaleSampleToCanvas,

        /*!
          The barWidthHint() defines a fixed width in paint device coordinates.
         */
        FixedSampleSize
    };

    explicit QwtPlotBarChart( const QString &title = QString::null );
    explicit QwtPlotBarChart( const QwtText &title );

    virtual ~QwtPlotBarChart();

    virtual int rtti() const;

    void setChartAttribute( ChartAttribute, bool on = true );
    bool testChartAttribute( ChartAttribute ) const;

    void setSamples( const QVector<QwtSetSample> & );
    void setSamples( const QVector< QVector<double> > & );
    void setSamples( const QVector<double> & );

    void setStyle( ChartStyle style );
    ChartStyle style() const;

    void setSymbol( QwtColumnSymbol * );
    const QwtColumnSymbol *symbol() const;


    void setLayoutPolicy( LayoutPolicy );
    LayoutPolicy layoutPolicy() const;

    void setLayoutHint( double );
    double layoutHint() const;

    void setSpacing( int );
    int spacing() const;

    void setBaseline( double );
    double baseline() const;

    virtual void drawSeries( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to ) const;

    virtual QRectF boundingRect() const;
    virtual void drawLegendIdentifier( QPainter *, const QRectF & ) const;

    virtual void getCanvasMarginHint( 
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect,
        double &left, double &top, double &right, double &bottom) const;

protected:
    virtual void drawSample( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, const QwtInterval &boundingInterval,
        int index, const QwtSetSample& sample ) const;

    virtual void drawBar( QPainter *, int sampleIndex,
        int barIndex, const QwtColumnRect & ) const;

    virtual void drawLabel( QPainter *, int sampleIndex,
        int barIndex, const QwtColumnRect &, const QwtText & ) const;

    virtual QwtText label( int sampleIndex, int barIndex,
        const QwtSetSample& ) const;

    void drawStackedBars( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int index,
        double sampleWidth, const QwtSetSample& sample ) const;

    void drawGroupedBars( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int index,
        double sampleWidth, const QwtSetSample& sample ) const;

private:
    void init();

    double sampleWidth( const QwtScaleMap &map,
        double canvasSize, double dataSize,
        const QwtSetSample& ) const;

    class PrivateData;
    PrivateData *d_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotBarChart::ChartAttributes )

#endif
