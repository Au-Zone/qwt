#include "formulaview.h"
#include <qevent.h>
#include <qpainter.h>
#include <qwt_mml_document.h>

FormulaView::FormulaView( QWidget *parent ):
    QWidget( parent )
{
}

QString FormulaView::formula() const
{
    return d_formula;
}

void FormulaView::setFormula( const QString &formula )
{
    d_formula = formula;
    update();
}

void FormulaView::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    renderFormula( &painter );
}

void FormulaView::renderFormula( QPainter *painter ) const
{
    QwtMathMLDocument doc;
    doc.setContent( d_formula );
    doc.setBaseFontPointSize( painter->font().pointSize() );

    QRectF docRect;
    docRect.setSize( doc.size() );
    docRect.moveCenter( rect().center() );

#if 1
    const double scaleF = 2.0;

    painter->save();

    painter->translate( docRect.center() );
    painter->scale( scaleF, scaleF );
    painter->translate( docRect.topLeft() - docRect.center() );
    doc.paint( painter, QPointF( 0, 0 ) );

    painter->restore();
#endif

#if 0
    doc.paint( painter, docRect.topLeft().toPoint() );
#endif
}
