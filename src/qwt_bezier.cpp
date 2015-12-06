#include "qwt_bezier.h"
#include <qstack.h>

class QwtBezierData
{
public:
    inline QwtBezierData()
    {
        // default constructor with unitialized points
    }

    inline QwtBezierData( const QPointF &p1, const QPointF &cp1,
            const QPointF &cp2, const QPointF &p2 ):
        d_x1( p1.x() ),
        d_y1( p1.y() ),
        d_cx1( cp1.x() ),
        d_cy1( cp1.y() ),
        d_cx2( cp2.x() ),
        d_cy2( cp2.y() ),
        d_x2( p2.x() ),
        d_y2( p2.y() )
    {
    }

    inline double flatness() const
    {
        // algo by Roger Willcocks ( http://www.rops.org )

        const double ux = 3.0 * d_cx1 - 2.0 * d_x1 - d_x2; 
        const double uy = 3.0 * d_cy1 - 2.0 * d_y1 - d_y2; 
        const double vx = 3.0 * d_cx2 - 2.0 * d_x2 - d_x1;
        const double vy = 3.0 * d_cy2 - 2.0 * d_y2 - d_y1;

        const double ux2 = ux * ux;
        const double uy2 = uy * uy;

        const double vx2 = vx * vx;
        const double vy2 = vy * vy;

        return qMax( ux2, vx2 ) + qMax( uy2, vy2 );
    }

    static inline double minFlatness( double tolerance )
    {
        return 16 * ( tolerance * tolerance );
    }

    inline QwtBezierData subdivided()
    {
        QwtBezierData bz;
        
        const double c1 = midValue( d_cx1, d_cx2 );

        bz.d_cx1 = midValue( d_x1, d_cx1 );
        d_cx2 = midValue( d_cx2, d_x2 );
        bz.d_x1 = d_x1;
        bz.d_cx2 = midValue( bz.d_cx1, c1 );
        d_cx1 = midValue( c1, d_cx2 );
        bz.d_x2 = d_x1 = midValue( bz.d_cx2, d_cx1 );

        const double c2 = midValue( d_cy1, d_cy2 );

        bz.d_cy1 = midValue( d_y1, d_cy1 );
        d_cy2 = midValue( d_cy2, d_y2 );
        bz.d_y1 = d_y1;
        bz.d_cy2 = midValue( bz.d_cy1, c2 );
        d_cy1 = midValue( d_cy2, c2 );
        bz.d_y2 = d_y1 = midValue( bz.d_cy2, d_cy1 );

        return bz;
    }

    inline QPointF p2() const
    {
        return QPointF( d_x2, d_y2 );
    }

private:
    inline double midValue( double v1, double v2 )
    {
        return 0.5 * ( v1 + v2 );
    }

    double d_x1, d_y1;
    double d_cx1, d_cy1;
    double d_cx2, d_cy2;
    double d_x2, d_y2;
};

void QwtBezier::toPolygon( double tolerance,
    const QPointF &p1, const QPointF &cp1,
    const QPointF &cp2, const QPointF &p2,
    QPolygonF &polygon )
{
    const double minFlatness = QwtBezierData::minFlatness( tolerance );

    polygon += p1;

    // to avoid deep stacks we convert the recursive algo
    // to something iterative, where the parameters of the
    // recursive calss are pushed to bezierStack instead

    QStack<QwtBezierData> bezierStack;
    bezierStack.push( QwtBezierData( p1, cp1, cp2, p2 ) );

    while( true )
    {
        QwtBezierData &bz = bezierStack.top();

        if ( bz.flatness() < minFlatness )
        {
            if ( bezierStack.size() == 1 )
                return; 

            polygon += bz.p2();
            bezierStack.pop();
        }
        else
        {
            bezierStack.push( bz.subdivided() );
        }
    }
}
