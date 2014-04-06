#include <qapplication.h>
#include <qevent.h>
#include <qwt_plot.h>
#include <qwt_scale_map.h>
#include <qwt_plot_curve.h>
#include "canvaspicker.h"

CanvasPicker::CanvasPicker( QwtPlot *plot ):
    QObject( plot ),
    d_selectedPoint( -1 )
{
    plot->canvas()->installEventFilter( this );
}

QwtPlot *CanvasPicker::plot()
{
    return qobject_cast<QwtPlot *>( parent() );
}

const QwtPlot *CanvasPicker::plot() const
{
    return qobject_cast<const QwtPlot *>( parent() );
}

bool CanvasPicker::eventFilter( QObject *object, QEvent *event )
{
    if ( plot() == NULL || object != plot()->canvas() )
        return false;

    switch( event->type() )
    {
        case QEvent::MouseButtonPress:
        {
            const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
            select( mouseEvent->pos() );
            return true;
        }
        case QEvent::MouseMove:
        {
            const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
            move( mouseEvent->pos() );
            return true;
        }
        default:
            break;
    }

    return QObject::eventFilter( object, event );
}

// Select the point at a position. If there is no point
// deselect the selected point

void CanvasPicker::select( const QPoint &pos )
{
    QwtPlotCurve *curve = NULL;
    double dist = 10e10;
    int index = -1;

    const QwtPlotItemList& itmList = plot()->itemList();
    for ( QwtPlotItemIterator it = itmList.begin();
        it != itmList.end(); ++it )
    {
        if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotCurve )
        {
            QwtPlotCurve *c = static_cast<QwtPlotCurve *>( *it );

            double d;
            int idx = c->closestPoint( pos, &d );
            if ( d < dist )
            {
                curve = c;
                index = idx;
                dist = d;
            }
        }
    }

    d_selectedCurve = NULL;
    d_selectedPoint = -1;

    if ( curve && dist < 10 ) // 10 pixels tolerance
    {
        d_selectedCurve = curve;
        d_selectedPoint = index;
    }
}

// Move the selected point
void CanvasPicker::move( const QPoint &pos )
{
    if ( d_selectedCurve == 0 || d_selectedPoint < 0  )
        return;

    QVector<double> xData( d_selectedCurve->dataSize() );
    QVector<double> yData( d_selectedCurve->dataSize() );

    double dx;
    double dy;

    int numPoints = static_cast<int>( d_selectedCurve->dataSize() );
    for ( int i = 0; i < numPoints; i++ )
    {
        const QPointF sample = d_selectedCurve->sample( i );

        if ( i == d_selectedPoint )
        {
            xData[i] = plot()->invTransform(
                d_selectedCurve->xAxis(), pos.x() );
            yData[i] = plot()->invTransform(
                d_selectedCurve->yAxis(), pos.y() );

            dx = xData[i] - sample.x();
            dy = yData[i] - sample.y();
        }
        else
        {
            xData[i] = sample.x();
            yData[i] = sample.y();
        }
    }
    d_selectedCurve->setSamples( xData, yData );

    QwtPlotItemList curves = plot()->itemList( QwtPlotItem::Rtti_PlotCurve );
    for ( int i = 0; i < curves.size(); i++ )
    {
        QwtPlotCurve *curve = static_cast<QwtPlotCurve *>( curves[i] );
        if ( curve == d_selectedCurve )
            continue;

        xData.resize( curve->dataSize() );
        yData.resize( curve->dataSize() );

        numPoints = static_cast<int>( curve->dataSize() );
        for ( int i = 0; i < numPoints; i++ )
        {   
            const QPointF sample = curve->sample( i );

            xData[i] = sample.x();
            yData[i] = sample.y();

            if ( i == d_selectedPoint )
            {
                xData[i] += dx;
                yData[i] += dy;
            }
        }
        curve->setSamples( xData, yData );
    }

    plot()->replot();
}
