/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_baritem.h"
#include "qwt_scale_map.h"

static inline double qwtTransformWidth(
    const QwtScaleMap &map, double value, double width )
{
    const double w2 = 0.5 * width;

    const double v1 = map.transform( value - w2 );
    const double v2 = map.transform( value + w2 );

    return qAbs( v2 - v1 );
}

class QwtPlotBarItem::PrivateData
{
public:
    PrivateData():
        layoutPolicy( QwtPlotBarItem::AutoAdjustSamples ),
        layoutHint( 0.5 ),
        spacing( 5 ),
        baseline( 0.0 )
    {
    }

    QwtPlotBarItem::LayoutPolicy layoutPolicy;
    double layoutHint;
    int spacing;
    double baseline;
    ChartAttributes chartAttributes;
};

QwtPlotBarItem::QwtPlotBarItem( const QwtText &title ):
    QwtPlotSeriesItem( title )
{
    d_data = new PrivateData;

    setItemAttribute( QwtPlotItem::Legend, true );
    setItemAttribute( QwtPlotItem::AutoScale, true );
    setItemAttribute( QwtPlotItem::Margins, true );
    setZ( 19.0 );
}

QwtPlotBarItem::~QwtPlotBarItem()
{
    delete d_data;
}

void QwtPlotBarItem::setChartAttribute( ChartAttribute attribute, bool on )
{
    if ( on )
        d_data->chartAttributes |= attribute;
    else
        d_data->chartAttributes &= ~attribute;
}

bool QwtPlotBarItem::testChartAttribute( ChartAttribute attribute ) const
{
    return ( d_data->chartAttributes & attribute );
}

void QwtPlotBarItem::setLayoutPolicy( LayoutPolicy policy )
{
    if ( policy != d_data->layoutPolicy )
    {
        d_data->layoutPolicy = policy;
        itemChanged();
    }
}

QwtPlotBarItem::LayoutPolicy QwtPlotBarItem::layoutPolicy() const
{
    return d_data->layoutPolicy;
}

void QwtPlotBarItem::setLayoutHint( double hint )
{
    hint = qMax( 0.0, hint );
    if ( hint != d_data->layoutHint )
    {
        d_data->layoutHint = hint;
        itemChanged();
    }
}

double QwtPlotBarItem::layoutHint() const
{
    return d_data->layoutHint;
}

void QwtPlotBarItem::setSpacing( int spacing )
{
    spacing = qMax( spacing, 0 );
    if ( spacing != d_data->spacing )
    {
        d_data->spacing = spacing;
        itemChanged();
    }
}

int QwtPlotBarItem::spacing() const
{
    return d_data->spacing;
}

void QwtPlotBarItem::setBaseline( double value )
{
    if ( value != d_data->baseline )
    {
        d_data->baseline = value;
        itemChanged();
    }
}

double QwtPlotBarItem::baseline() const
{
    return d_data->baseline;
}

double QwtPlotBarItem::sampleWidth( const QwtScaleMap &map,
    double canvasSize, double boundingSize, double value ) const
{
    double width;

    switch( d_data->layoutPolicy )
    {
        case ScaleSamplesToAxes:
        {
            width = qwtTransformWidth( map, value, d_data->layoutHint );
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

            width = qwtTransformWidth( map, value, w );
            width -= d_data->spacing;
        }
    }

    return width;
}

void QwtPlotBarItem::getCanvasMarginHint( const QwtScaleMap &xMap, 
    const QwtScaleMap &yMap, const QRectF &canvasRect,
    double &left, double &top, double &right, double &bottom ) const
{
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
    		const size_t numSamples = dataSize();
            if ( numSamples <= 0 )
                break;

            // doesn't work for nonlinear scales

            const QRectF br = dataRect();
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

            const double value = br.left();

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
