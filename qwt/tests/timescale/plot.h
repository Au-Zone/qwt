#ifndef _PLOT_H_
#define _PLOT_H_

#include <qwt_plot.h>

class Plot: public QwtPlot
{
public:
	Plot( QWidget *parent = NULL );
};
#endif
