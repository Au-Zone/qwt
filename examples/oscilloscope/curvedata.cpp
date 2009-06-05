#include "curvedata.h"

CurveData::CurveData(double interval):
	d_interval(interval)
{
	reset();
}
	
void CurveData::reset()
{
	d_startTime.start();
	d_boundingRect = QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid

	d_values.clear();
	d_values.reserve(1000);
}

void CurveData::append(double value)
{
	const QTime currentTime = QTime::currentTime();
	const int elapsed = d_startTime.elapsed();

	if ( elapsed >= d_interval * 1000 )
	{
		d_startTime.start();
		d_boundingRect = QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid

		const int numPoints = d_values.size();
		d_values.clear();
		d_values.reserve(numPoints);
	}

	const double time = d_startTime.elapsed() / 1000.0;
	d_values += QwtDoublePoint(time, value);

	// adjust the bounding rectangle 

	if ( !d_boundingRect.isValid() )
		d_boundingRect = QwtDoubleRect(time, value, 0.0, 0.0);
	else
	{
		d_boundingRect.setRight(time);
		if ( value > d_boundingRect.bottom() )
			d_boundingRect.setBottom(value);
		if ( value < d_boundingRect.top() )
			d_boundingRect.setBottom(value);
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
	CurveData *other = new CurveData(d_interval);
	other->d_startTime = d_startTime;
	other->d_values = d_values;
	other->d_boundingRect = d_boundingRect;

	return other;
}

QwtDoubleRect CurveData::boundingRect() const
{
	return d_boundingRect;
}

