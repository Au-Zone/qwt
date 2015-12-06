#include "qwt_bezier.h"
#include <qstack.h>

static inline double qwtMidValue( double v1, double v2 )
{
    return 0.5 * ( v1 + v2 );
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

    inline QwtBezierData subdivided()
    {
        QwtBezierData bz;
        
        const double c1 = qwtMidValue( cx1, cx2 );

        bz.cx1 = qwtMidValue( x1, cx1 );
        cx2 = qwtMidValue( cx2, x2 );
        bz.x1 = x1;
        bz.cx2 = qwtMidValue( bz.cx1, c1 );
        cx1 = qwtMidValue( c1, cx2 );
        bz.x2 = x1 = qwtMidValue( bz.cx2, cx1 );

        const double c2 = qwtMidValue( cy1, cy2 );

        bz.cy1 = qwtMidValue( y1, cy1 );
        cy2 = qwtMidValue( cy2, y2 );
        bz.y1 = y1;
        bz.cy2 = qwtMidValue( bz.cy1, c2 );
        cy1 = qwtMidValue( cy2, c2 );
        bz.y2 = y1 = qwtMidValue( bz.cy2, cy1 );

        return bz;
    }

    double x1, y1;
    double cx1, cy1;
    double cx2, cy2;
    double x2, y2;
};

QPolygonF QwtBezier::toPolygon( double tolerance,
    double x1, double y1, double cx1, double cy1,
    double cx2, double cy2, double x2, double y2,
    bool withLastPoint )
{
    if ( tolerance <= 0.0 )
        return QPolygonF();

    // according to the algo used in QwtBezierData::flatness()
    const double minFlatness = 16 * ( tolerance * tolerance );

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
            bezierStack.push( bezier.subdivided() );
        }
    }
}
