#include <qstring.h>
#include <qpainter.h>
#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_column_symbol.h>
#include <qwt_scale_map.h>
#include <qwt_plot_histogram.h>

class QwtPlotHistogram::PrivateData
{
public:
    PrivateData():
        attributes(QwtPlotHistogram::Auto),
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

QwtPlotHistogram::QwtPlotHistogram(const QwtText &title):
    QwtPlotSeriesItem<QwtIntervalSample>(title)
{
    init();
}

QwtPlotHistogram::QwtPlotHistogram(const QString &title):
    QwtPlotSeriesItem<QwtIntervalSample>(title)
{
    init();
}

QwtPlotHistogram::~QwtPlotHistogram()
{
    delete d_data;
}

void QwtPlotHistogram::init()
{
    d_data = new PrivateData();
    d_series = new QwtIntervalSeriesData();

    setItemAttribute(QwtPlotItem::AutoScale, true);
    setItemAttribute(QwtPlotItem::Legend, true);

    setZ(20.0);
}

void QwtPlotHistogram::setSymbol(const QwtColumnSymbol &symbol)
{
    delete d_data->symbol;
    d_data->symbol = symbol.clone();
}

const QwtColumnSymbol &QwtPlotHistogram::symbol() const
{
    return *d_data->symbol;
}

void QwtPlotHistogram::setBaseline(double reference)
{
    if ( d_data->reference != reference )
    {
        d_data->reference = reference;
        itemChanged();
    }
}

double QwtPlotHistogram::baseline() const
{
    return d_data->reference;
}

void QwtPlotHistogram::setColor(const QColor &color)
{
    if ( d_data->color != color )
    {
        d_data->color = color;
        itemChanged();
    }
}

QColor QwtPlotHistogram::color() const
{
    return d_data->color;
}

QwtDoubleRect QwtPlotHistogram::boundingRect() const
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


int QwtPlotHistogram::rtti() const
{
    return QwtPlotItem::Rtti_PlotHistogram;
}

void QwtPlotHistogram::setHistogramAttribute(HistogramAttribute attribute, bool on)
{
    if ( bool(d_data->attributes & attribute) == on )
        return;

    if ( on )
        d_data->attributes |= attribute;
    else
        d_data->attributes &= ~attribute;

    itemChanged();
}

bool QwtPlotHistogram::testHistogramAttribute(HistogramAttribute attribute) const
{
    return d_data->attributes & attribute;
}

void QwtPlotHistogram::draw(QPainter *painter, const QwtScaleMap &xMap, 
    const QwtScaleMap &yMap, const QRect &) const
{
    painter->setPen(QPen(d_data->color));

    const int x0 = xMap.transform(baseline());
    const int y0 = yMap.transform(baseline());

    const Qt::Orientation orientation = (d_data->attributes & Xfy) 
        ? Qt::Horizontal : Qt::Vertical;

    for ( int i = 0; i < (int)d_series->size(); i++ )
    {
        QwtIntervalSample sample = d_series->sample(i);

        QRect symbolRect;

        if ( d_data->attributes & QwtPlotHistogram::Xfy )
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

            symbolRect.setRect(x0, y1, x2 - x0, y2 - y1);
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
            symbolRect.setRect(x1, y0, x2 - x1, y2 - y0);
        }

        const QwtColumnSymbol *symbol = adjustedSymbol(sample, *d_data->symbol);
        if ( symbol )
        {
            symbol->draw(painter, orientation, symbolRect );
            if ( symbol != d_data->symbol )
                delete symbol;
        }
    }
}

const QwtColumnSymbol *QwtPlotHistogram::adjustedSymbol(
    const QwtIntervalSample &, const QwtColumnSymbol &defaultSymbol) const
{
    return &defaultSymbol;
}
