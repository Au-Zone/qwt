#include <qapplication.h>
#include "plot.h"

#ifndef QWT_NO_OPENGL
#if QT_VERSION > 0x040600 && QT_VERSION < 0x050000
#define USE_OPENGL 1
#endif
#endif

#if USE_OPENGL
#include <qgl.h>
#include <qwt_plot_glcanvas.h>
#endif

int main ( int argc, char **argv )
{
    QApplication a( argc, argv );

    Plot plot;

#if USE_OPENGL
    QwtPlotGLCanvas *canvas = new QwtPlotGLCanvas();
    canvas->setFrameStyle( QwtPlotGLCanvas::NoFrame );
    plot.setCanvas( canvas );
#endif

    plot.setCanvasBackground( QColor( 30, 30, 50 ) );

    plot.resize( 400, 400 );
    plot.show();

    return a.exec();
}
