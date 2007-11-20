#include <qstring.h>
#include <qpainter.h>
#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_column_symbol.h>
#include <qwt_scale_map.h>
#include <qwt_plot_histogram.h>

static void drawColumn(QPainter *painter, const QRect &rect)
{
    int pw = painter->pen().width(); 
    if ( pw == 0 )
        pw = 1;

#if QT_VERSION < 0x040000
	QRect r = rect.normalize();
    r.setLeft(r.left() + pw / 2);
    r.setTop(r.top() + pw / 2 + 1);
    r.setRight(r.right() - pw / 2 + 2 - pw % 2);
    r.setBottom(r.bottom() - pw / 2 + 1 - pw % 2 );
#else
	QRect r = rect.normalized();
    r.setLeft(r.left() + pw / 2);
    r.setRight(r.right() + pw / 2 + 1);
    r.setTop(r.top() + pw / 2 + 1);
    r.setBottom(r.bottom() + pw / 2);
#endif
	QwtPainter::drawRect(painter, r);
}

class QwtPlotHistogram::PrivateData
{
public:
    PrivateData():
        reference(0.0),
        curveStyle(NoCurve)
    {
        symbol = new QwtColumnSymbol(QwtColumnSymbol::RaisedBox);
    }

    ~PrivateData()
    {
        delete symbol;
    }

    double reference;

    QPen pen;
    QBrush brush;
	QwtPlotHistogram::CurveStyle curveStyle;
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

void QwtPlotHistogram::setStyle(CurveStyle style)
{
    if ( style != d_data->curveStyle )
    {
        d_data->curveStyle = style;
        itemChanged();
    }
}

QwtPlotHistogram::CurveStyle QwtPlotHistogram::style() const
{
    return d_data->curveStyle;
}

void QwtPlotHistogram::setPen(const QPen &pen)
{
    if ( pen != d_data->pen )
    {
        d_data->pen = pen;
        itemChanged();
    }
}

const QPen &QwtPlotHistogram::pen() const
{
    return d_data->pen;
}

void QwtPlotHistogram::setBrush(const QBrush &brush)
{
    if ( brush != d_data->brush )
    { 
        d_data->brush = brush;
        itemChanged();
    }
}

const QBrush &QwtPlotHistogram::brush() const
{
    return d_data->brush; 
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

QwtDoubleRect QwtPlotHistogram::boundingRect() const
{
    QwtDoubleRect rect = d_series->boundingRect();
    if ( !rect.isValid() ) 
        return rect;

    if ( orientation() == Qt::Vertical )
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

void QwtPlotHistogram::setData(
    const QwtArray<QwtIntervalSample> &data)
{
    QwtPlotSeriesItem<QwtIntervalSample>::setData(
        QwtIntervalSeriesData(data));
}

void QwtPlotHistogram::setData(
    const QwtSeriesData<QwtIntervalSample> &data)
{
    QwtPlotSeriesItem<QwtIntervalSample>::setData(data);
}

void QwtPlotHistogram::draw(QPainter *painter, 
	const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
	const QRect &) const
{
    draw(painter, xMap, yMap, 0, -1);
}

void QwtPlotHistogram::draw(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    int from, int to) const
{
    if ( !painter || dataSize() <= 0 )
        return;

    if (to < 0)
        to = dataSize() - 1;

	painter->save();
    drawCurve(painter, d_data->curveStyle, xMap, yMap, from, to);
	painter->restore();

	if ( d_data->symbol->style() != QwtColumnSymbol::NoSymbol)
	{
        painter->save();
        drawSymbols(painter, *d_data->symbol, xMap, yMap, from, to);
        painter->restore();
	}
}


void QwtPlotHistogram::drawSymbols(QPainter *painter, 
	const QwtColumnSymbol &symbol, const QwtScaleMap &xMap, 
    const QwtScaleMap &yMap, int from, int to) const
{
#if 1
    const QColor color(Qt::darkCyan);
#endif
    painter->setPen(QPen(color));

    const int x0 = xMap.transform(baseline());
    const int y0 = yMap.transform(baseline());

    for ( int i = from; i < to; i++ )
    {
        QwtIntervalSample sample = d_series->sample(i);

        QRect symbolRect;

        if ( orientation() == Qt::Vertical )
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
        else // Horizontal
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

        const QwtColumnSymbol *sym = adjustedSymbol(sample, symbol);
        if ( sym )
        {
            sym->draw(painter, orientation(), symbolRect );
            if ( sym != d_data->symbol )
                delete sym;
        }
    }
}

void QwtPlotHistogram::drawCurve(QPainter *painter, int style,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    int from, int to) const
{
    switch (style)
    {
        case Columns:
			drawColumns(painter, xMap, yMap, from, to);
			break;
        case Lines:
			drawLines(painter, xMap, yMap, from, to);
			break;
        case Caps:
			drawCaps(painter, xMap, yMap, from, to);
			break;
        case NoCurve:
        default:
            break;
	}
}

void QwtPlotHistogram::drawColumns(QPainter *painter,
	const QwtScaleMap &xMap, const QwtScaleMap &yMap,
	int from, int to) const
{
    QRect rect;

    const int x0 = xMap.transform(d_data->reference);
    const int y0 = yMap.transform(d_data->reference);

#if 1
	painter->setPen(d_data->pen);
#else
	QPen pen(Qt::black, 2);
	//pen.setCapStyle(Qt::FlatCap);
	//pen.setJoinStyle(Qt::MiterJoin);
	painter->setPen(pen);
#endif
	painter->setBrush(d_data->brush);

    for ( int i = from; i <= to; i++ )
    {
        QwtIntervalSample sample = d_series->sample(i);
		if ( sample.interval.isNull() )
			continue;

		const QwtDoubleInterval &iv = sample.interval;
		int minOff = 0;
		if ( iv.borderFlags() & QwtDoubleInterval::ExcludeMinimum )
			minOff = 1;

		int maxOff = 0;
		if ( iv.borderFlags() & QwtDoubleInterval::ExcludeMaximum )
			maxOff = 1;

    	if ( orientation() == Qt::Vertical )
		{
			const int x = xMap.transform(sample.value);
			const int y1 = yMap.transform( iv.minValue()) - minOff;
			const int y2 = yMap.transform( iv.maxValue()) + maxOff;

			rect.setRect(x0, y1, x - x0, y2 - y1);
		}
    	else
		{
			const int x1 = xMap.transform( iv.minValue()) + minOff;
			const int x2 = xMap.transform( iv.maxValue()) - maxOff;
			const int y = yMap.transform(sample.value);

			rect.setRect(x1, y0, x2 - x1, y - y0);
		}

		drawColumn(painter, rect);
	}
}

void QwtPlotHistogram::drawLines(QPainter *,
        const QwtScaleMap &, const QwtScaleMap &,
        int , int ) const
{
}

void QwtPlotHistogram::drawCaps(QPainter *,
        const QwtScaleMap &, const QwtScaleMap &,
        int , int ) const
{
}

const QwtColumnSymbol *QwtPlotHistogram::adjustedSymbol(
    const QwtIntervalSample &, const QwtColumnSymbol &defaultSymbol) const
{
    return &defaultSymbol;
}

void QwtPlotHistogram::updateLegend(QwtLegend *) const
{
#if 0
#ifdef __GNUC__
#warning TODO
#endif
#endif
}
