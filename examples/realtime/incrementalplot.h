#ifndef _INCREMENTALPLOT_H_
#define _INCREMENTALPLOT_H_ 1

#include <qwt_plot.h>
#include <qvector.h>

class QwtPlotCurve;
class QwtPlotDirectPainter;

class CurveData
{
    // A container class for growing data
public:

    CurveData();

    void append(double *x, double *y, int count);

    int count() const;
    int size() const;
    const double *x() const;
    const double *y() const;

private:
    int d_count;
    QVector<double> d_x;
    QVector<double> d_y;
};

class IncrementalPlot : public QwtPlot
{
    Q_OBJECT
public:
    IncrementalPlot(QWidget *parent = NULL);
    virtual ~IncrementalPlot();

    void appendData(double x, double y);
    void appendData(double *x, double *y, int size);

    void removeData();

private:
    CurveData *d_data;
    QwtPlotCurve *d_curve;
    QwtPlotDirectPainter *d_directPainter;
};

#endif // _INCREMENTALPLOT_H_
