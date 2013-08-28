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
    painter->save();
    painter->translate( docRect.topLeft() );
    doc.paint( painter, QPoint( 0, 0 ) );
    painter->restore();
#else
    doc.paint( painter, docRect.topLeft().toPoint() );
#endif
}
