#include <qstring.h>
#include <qpainter.h>
#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_column_symbol.h>
#include <qwt_scale_map.h>
#include "histogram_item.h"

class HistogramItem::PrivateData
{
public:
    PrivateData():
        attributes(HistogramItem::Auto),
        reference(0.0)
    {
        symbol = new QwtColumnSymbol(QwtColumnSymbol::RaisedBox);
    }

    ~PrivateData()
    {
        delete symbol;
    }

    int attributes;
    double reference;

    QColor color;
    QwtColumnSymbol *symbol;
};

HistogramItem::HistogramItem(const QwtText &title):
    QwtPlotSeriesItem<QwtIntervalSample>(title)
{
    init();
}

HistogramItem::HistogramItem(const QString &title):
    QwtPlotSeriesItem<QwtIntervalSample>(title)
{
    init();
}

HistogramItem::~HistogramItem()
{
    delete d_data;
}

void HistogramItem::init()
{
    d_data = new PrivateData();
    d_series = new QwtIntervalSeriesData();

    setItemAttribute(QwtPlotItem::AutoScale, true);
    setItemAttribute(QwtPlotItem::Legend, true);

    setZ(20.0);
}

void HistogramItem::setSymbol(const QwtColumnSymbol &symbol)
{
    delete d_data->symbol;
    d_data->symbol = symbol.clone();
}

const QwtColumnSymbol &HistogramItem::symbol() const
{
    return *d_data->symbol;
}

void HistogramItem::setBaseline(double reference)
{
    if ( d_data->reference != reference )
    {
        d_data->reference = reference;
        itemChanged();
    }
}

double HistogramItem::baseline() const
{
    return d_data->reference;
}

void HistogramItem::setColor(const QColor &color)
{
    if ( d_data->color != color )
    {
        d_data->color = color;
        itemChanged();
    }
}

QColor HistogramItem::color() const
{
    return d_data->color;
}

QwtDoubleRect HistogramItem::boundingRect() const
{
    QwtDoubleRect rect = d_series->boundingRect();
    if ( !rect.isValid() ) 
        return rect;

    if ( d_data->attributes & Xfy ) 
    {
        rect = QwtDoubleRect( rect.y(), rect.x(), 
            rect.height(), rect.width() );

        if ( rect.left() > d_data->reference ) 
            rect.setLeft( d_data->reference );
        else if ( rect.right() < d_data->reference ) 
            rect.setRight( d_data->reference );
    } 
    else 
    {
        if ( rect.bottom() < d_data->reference ) 
            rect.setBottom( d_data->reference );
        else if ( rect.top() > d_data->reference ) 
            rect.setTop( d_data->reference );
    }

    return rect;
}


int HistogramItem::rtti() const
{
    return QwtPlotItem::Rtti_PlotHistogram;
}

void HistogramItem::setHistogramAttribute(HistogramAttribute attribute, bool on)
{
    if ( bool(d_data->attributes & attribute) == on )
        return;

    if ( on )
        d_data->attributes |= attribute;
    else
        d_data->attributes &= ~attribute;

    itemChanged();
}

bool HistogramItem::testHistogramAttribute(HistogramAttribute attribute) const
{
    return d_data->attributes & attribute;
}

void HistogramItem::draw(QPainter *painter, const QwtScaleMap &xMap, 
    const QwtScaleMap &yMap, const QRect &) const
{
    painter->setPen(QPen(d_data->color));

    const int x0 = xMap.transform(baseline());
    const int y0 = yMap.transform(baseline());

    for ( int i = 0; i < (int)d_series->size(); i++ )
    {
        QwtIntervalSample sample = d_series->sample(i);

        if ( d_data->attributes & HistogramItem::Xfy )
        {
            const int x2 = xMap.transform(sample.value);
            if ( x2 == x0 )
                continue;


            int y1 = yMap.transform( sample.interval.minValue());
            int y2 = yMap.transform( sample.interval.maxValue());
            if ( y1 > y2 )
                qSwap(y1, y2);

            if ( i < (int)d_series->size() - 2 )
            {
                sample = d_series->sample(i+1);

                const int yy1 = yMap.transform(sample.interval.minValue());
                const int yy2 = yMap.transform(sample.interval.maxValue());

                if ( y2 == qwtMin(yy1, yy2) )
                {
                    const int xx2 = xMap.transform(sample.interval.minValue());
                    if ( xx2 != x0 && ( (xx2 < x0 && x2 < x0) ||
                                          (xx2 > x0 && x2 > x0) ) )
                    {
                       // One pixel distance between neighboured bars
                       y2++;
                    }
                }
            }

            d_data->symbol->draw(painter, Qt::Horizontal,
                QRect(x0, y1, x2 - x0, y2 - y1));
        }
        else
        {
            const int y2 = yMap.transform(sample.value);
            if ( y2 == y0 )
                continue;

            int x1 = xMap.transform(sample.interval.minValue());
            int x2 = xMap.transform(sample.interval.maxValue());
            if ( x1 > x2 )
                qSwap(x1, x2);

            if ( i < (int)d_series->size() - 2 )
            {
                sample = d_series->sample(i+1);

                const int xx1 = xMap.transform(sample.interval.minValue());
                const int xx2 = xMap.transform(sample.interval.maxValue());

                if ( x2 == qwtMin(xx1, xx2) )
                {
                    const int yy2 = yMap.transform(sample.value);
                    if ( yy2 != y0 && ( (yy2 < y0 && y2 < y0) ||
                                    (yy2 > y0 && y2 > y0) ) )
                    {
                        // One pixel distance between neighboured bars
                        x2--;
                    }
                }
            }
            d_data->symbol->draw(painter, Qt::Vertical,
                QRect(x1, y0, x2 - x1, y2 - y0) );
        }
    }
}
