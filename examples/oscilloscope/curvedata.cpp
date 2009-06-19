#include "curvedata.h"

CurveData::CurveData()
{
    d_boundingRect = QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid
    d_values.reserve(10000);
}
    
void CurveData::reset(double min)
{
    d_boundingRect = QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid

    QVector<QwtDoublePoint> values = d_values;

    d_values.clear();
    d_values.reserve(values.size());

    int index;
    for ( index = values.size() - 1; index >= 0; index-- )
    {
        if ( values[index].x() < min )
            break;
    }

    if ( index > 0 )
        append(values[index++]);

    while ( index < values.size() - 1 )
        append(values[index++]);
}

void CurveData::append(const QwtDoublePoint &sample)
{
    d_values += sample;

    // adjust the bounding rectangle 

    if ( !d_boundingRect.isValid() )
        d_boundingRect = QwtDoubleRect(sample.x(), sample.y(), 0.0, 0.0);
    else
    {
        d_boundingRect.setRight(sample.x());
        if ( sample.y() > d_boundingRect.bottom() )
            d_boundingRect.setBottom(sample.y());
        if ( sample.y() < d_boundingRect.top() )
            d_boundingRect.setBottom(sample.y());
    }
}

QwtDoublePoint CurveData::sample(size_t i) const
{
    if ( i < (size_t) d_values.size() )
        return d_values[i];

    return QwtDoublePoint();
}

size_t CurveData::size() const
{
    return d_values.size();
}

QwtSeriesData<QwtDoublePoint> *CurveData::copy() const
{
    CurveData *other = new CurveData();
    other->d_values = d_values;
    other->d_boundingRect = d_boundingRect;

    return other;
}

QwtDoubleRect CurveData::boundingRect() const
{
    return d_boundingRect;
}

