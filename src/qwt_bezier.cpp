#include "qwt_bezier.h"
#include <qstack.h>
#include <QDebug>

static inline double qwtMidValue( double v1, double v2 )
{
    return 0.5 * ( v1 + v2 );
}

class QwtBezierData
{
public:
    inline void setPoints( double p_x1, double p_y1, double p_cx1, double p_cy1,
        double p_cx2, double p_cy2, double p_x2, double p_y2 )
    {
        x1 = p_x1;
        y1 = p_y1;

        cx1 = p_cx1;
        cy1 = p_cy1;

        cx2 = p_cx2;
        cy2 = p_cy2;

        x2 = p_x2;
        y2 = p_y2;
    }

    inline double flatness() const
    {
        // Based on the maximal distance between the curve and the straight line
        // between the endpoints. Algo attributed to Roger Willcocks 
        // ( http://www.rops.org ), but maths behind are quite obvious.

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

    double x1, y1;
    double cx1, cy1;
    double cx2, cy2;
    double x2, y2;
};

QPolygonF QwtBezier::toPolygon( double x1, double y1,
                                double cx1, double cy1, double cx2, double cy2,
                                double x2, double y2, double tolerance )
{
    if ( tolerance <= 0.0 )
        return QPolygonF();

    const double minFlatness = 16 * ( tolerance * tolerance );

    QPolygonF polygon;
    polygon += QPointF( x1, y1 );

    // to avoid deep stacks we convert the recursive algo
    // to something iterative, where the parameters of the
    // recursive calss are pushed to bezierStack instead

    QStack<QwtBezierData> bezierStack;
    bezierStack.push( QwtBezierData() );

    QwtBezierData &bz0 = bezierStack.top();
    bz0.setPoints( x1, y1, cx1, cy1, cx2, cy2, x2, y2 );
    
    while( true )
    {
        QwtBezierData &bezier = bezierStack.top();

        if ( bezier.flatness() <= minFlatness )
        {
            polygon += QPointF( bezier.x2, bezier.y2 );

            bezierStack.pop();
            if ( bezierStack.isEmpty() )
                break;
        }
        else
        {
            bezierStack.push( QwtBezierData() );

            QwtBezierData &bz = bezierStack.top();
            
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
        }
    }

    return polygon;
}
