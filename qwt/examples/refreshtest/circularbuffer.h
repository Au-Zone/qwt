#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_

#include <qwt_series_data.h>
#include <qwt_array.h>

class CircularBuffer: public QwtSeriesData<QwtDoublePoint>
{
public:
    CircularBuffer(double interval = 10.0, size_t numPoints = 1000);
    void fill(double interval, size_t numPoints);

    void setReferenceTime(double);
    double referenceTime() const;

    virtual size_t size() const;
    virtual QwtDoublePoint sample(size_t i) const;

    virtual QwtDoubleRect boundingRect() const;
    virtual QwtSeriesData<QwtDoublePoint> *copy() const;

private:
    double value(double x) const;

    double d_referenceTime;
    double d_interval;
    QwtArray<double> d_values;
};

#endif
