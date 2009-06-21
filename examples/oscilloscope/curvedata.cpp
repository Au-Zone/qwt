#include "curvedata.h"
#include "signaldata.h"

const SignalData &CurveData::values() const
{
    return SignalData::instance();
}

SignalData &CurveData::values() 
{
    return SignalData::instance();
}

QwtDoublePoint CurveData::sample(size_t i) const
{
    return SignalData::instance().value(i);
}

size_t CurveData::size() const
{
    return SignalData::instance().size();
}

QwtSeriesData<QwtDoublePoint> *CurveData::copy() const
{
    return new CurveData();
}

QwtDoubleRect CurveData::boundingRect() const
{
    return SignalData::instance().boundingRect();
}
