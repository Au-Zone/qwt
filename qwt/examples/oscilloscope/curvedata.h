#include <qwt_series_data.h>
#include <qdatetime.h>

class CurveData: public QwtSeriesData<QwtDoublePoint>
{
public:
	CurveData(double interval);

	void append(double value);
	void reset();

    virtual QwtDoublePoint sample(size_t i) const;
    virtual size_t size() const;

	virtual QwtDoubleRect boundingRect() const;

    virtual QwtSeriesData<QwtDoublePoint> *copy() const;

private:
	QTime d_startTime;
	const double d_interval; 

	QwtDoubleRect d_boundingRect; 
	QVector<QwtDoublePoint> d_values;
};
