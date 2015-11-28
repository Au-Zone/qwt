#include "qwt_bezier.h"
#include <qstack.h>
#include <QDebug>

static inline double qwtMidValue( double v1, double v2 )
{
    return 0.5 * ( v1 + v2 );
}

static inline double qwtManhattanLength(
    double x1, double y1, double x2, double y2 )
{
    return qAbs( x2 - x1 ) + qAbs( y2 - y1 );
}

struct QwtBezierData
{
    inline double flatness() const
    {
        // algo by Roger Willcocks ( http://www.rops.org )

        const double ux = 3.0 * cx1 - 2.0 * x1 - x2; 
        const double uy = 3.0 * cy1 - 2.0 * y1 - y2; 
        const double vx = 3.0 * cx2 - 2.0 * x2 - x1;
        const double vy = 3.0 * cy2 - 2.0 * y2 - y1;

        const double ux2 = ux * ux;
        const double uy2 = uy * uy;

        const double vx2 = vx * vx;
        const double vy2 = vy * vy;

        return qMax( ux2, vx2 ) + qMax( uy2, vy2 );
    }

    inline double flatnessML() const 
    {
        const double l = qwtManhattanLength( x1, y1, x2, y2 );
        if ( l > 1.0 )
        {   
            const double dx = x2 - x1;
            const double dy = y2 - y1;

            const double x3 = dx * (y1 - cy1);
            const double y3 = dx * (y1 - cy2);

            const double x4 = dy * (x1 - cx1);
            const double y4 = dy * (x1 - cx2);

            return qwtManhattanLength( x3, y3, x4, y4 ) / l;
        }
        else
        {   
            return qwtManhattanLength( x1, y1, cx1, cy1 ) 
                + qwtManhattanLength( x1, y1, cx2, cy2 );
        }
    }

    inline double flatness2() const
    {
        double dd2x = x1 + cx2 - 2 * cx1;
        double dd2y = y1 + cy2 - 2 * cy1;

        double dd3x = cx1 + x2 - 2 * cx2;
        double dd3y = cy1 + y2 - 2 * cy2;

        const double l1 = qAbs(dd2x) + qAbs(dd2y);
        const double l2 = qAbs(dd3x) + qAbs(dd3y);

        return l1 + l2;
    }

    inline double flatness3() const
    {
        double td2x = 2 * x1 + x2 - 3 * cx1;
        double td2y = 2 * y1 + y2 - 3 * cy1;
        double td3x = x1 + 2 * x2 - 3 * cx2;
        double td3y = y1 + 2 * y2 - 3 * cy2;

        const double l1 = qAbs(td2x) + qAbs(td2y);
        const double l2 = qAbs(td3x) + qAbs(td3y);

        return l1 + l2;
    }

    double x1, y1;
    double cx1, cy1;
    double cx2, cy2;
    double x2, y2;
};

QPolygonF QwtBezier::toPolygon( double maxDeviation,
    double x1, double y1, double cx1, double cy1,
    double cx2, double cy2, double x2, double y2,
    bool withLastPoint )
{
    if ( maxDeviation <= 0.0 )
        return QPolygonF();

    // according to the algo used in QwtBezierData::flatness()
    const double minFlatness = 16 * ( maxDeviation * maxDeviation );

    QPolygonF polygon;
    polygon += QPointF( x1, y1 );

    // to avoid deep stacks we convert the recursive algo
    // to something iterative, where the parameters of the
    // recursive calss are pushed to bezierStack instead

    QStack<QwtBezierData> bezierStack;
    bezierStack.push( QwtBezierData() );

    QwtBezierData &bz0 = bezierStack.top();
    bz0.x1 = x1;
    bz0.y1 = y1;
    bz0.cx1 = cx1;
    bz0.cy1 = cy1;
    bz0.cx2 = cx2;
    bz0.cy2 = cy2;
    bz0.x2 = x2;
    bz0.y2 = y2;
    
    while( true )
    {
        QwtBezierData &bezier = bezierStack.top();

        if ( bezier.flatness() < minFlatness )
        {
            if ( bezierStack.size() == 1 )
            {
                if ( withLastPoint )
                    polygon += QPointF( bezier.x2, bezier.y2 );

                return polygon;
            }

            polygon += QPointF( bezier.x2, bezier.y2 );
            bezierStack.pop();
        }
        else
        {
            QwtBezierData bz;
            
            const double c1 = qwtMidValue( bezier.cx1, bezier.cx2 );

            bz.cx1 = qwtMidValue( bezier.x1, bezier.cx1 );
            bezier.cx2 = qwtMidValue( bezier.cx2, bezier.x2 );
            bz.x1 = bezier.x1;
            bz.cx2 = qwtMidValue( bz.cx1, c1 );
            bezier.cx1 = qwtMidValue( c1, bezier.cx2 );
            bz.x2 = bezier.x1 = qwtMidValue( bz.cx2, bezier.cx1 );

            const double c2 = qwtMidValue( bezier.cy1, bezier.cy2 );

            bz.cy1 = qwtMidValue( bezier.y1, bezier.cy1 );
            bezier.cy2 = qwtMidValue( bezier.cy2, bezier.y2 );
            bz.y1 = bezier.y1;
            bz.cy2 = qwtMidValue( bz.cy1, c2 );
            bezier.cy1 = qwtMidValue( bezier.cy2, c2 );
            bz.y2 = bezier.y1 = qwtMidValue( bz.cy2, bezier.cy1 );

            bezierStack.push( bz );
        }
    }
}
