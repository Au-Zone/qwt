#include <qwt_series_data.h>
#include <qdatetime.h>

class CurveData: public QwtSeriesData<QwtDoublePoint>
{
public:
    CurveData();

    void append(const QwtDoublePoint &);
    void reset(double min);

    virtual QwtDoublePoint sample(size_t i) const;
    virtual size_t size() const;

    virtual QwtDoubleRect boundingRect() const;

    virtual QwtSeriesData<QwtDoublePoint> *copy() const;

private:
    QwtDoubleRect d_boundingRect; 
    QVector<QwtDoublePoint> d_values;
};
