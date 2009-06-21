#include <qwt_series_data.h>
#include <qpointer.h>

class SignalData;

class CurveData: public QwtSeriesData<QwtDoublePoint>
{
public:
    const SignalData &values() const;
    SignalData &values();

    virtual QwtDoublePoint sample(size_t i) const;
    virtual size_t size() const;

    virtual QwtDoubleRect boundingRect() const;

    virtual QwtSeriesData<QwtDoublePoint> *copy() const;
};
