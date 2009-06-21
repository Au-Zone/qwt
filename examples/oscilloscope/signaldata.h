#ifndef _SIGNAL_DATA_H_
#define _SIGNAL_DATA_H_ 1

#include <qwt_double_rect.h>

class SignalData
{
public:
    static SignalData &instance();

    void append(const QwtDoublePoint &pos);
    void clearStaleValues(double min);

    int size() const;
    QwtDoublePoint value(int index) const;

    QwtDoubleRect boundingRect() const;

    void lock();
    void unlock();
    
private:
    SignalData();
    SignalData(const SignalData &);
    SignalData &operator=( const SignalData & );

    virtual ~SignalData();

    class PrivateData;
    PrivateData *d_data;
};

#endif
