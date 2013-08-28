#ifndef _FORMULA_VIEW_H_
#define _FORMULA_VIEW_H_

#include <qwidget.h>

class QPainter;

class FormulaView: public QWidget
{
    Q_OBJECT

public:
    FormulaView( QWidget *parent = NULL );
    
    QString formula() const;

public Q_SLOTS:
    void setFormula( const QString & );

protected:
    virtual void paintEvent( QPaintEvent * );

private:
    void renderFormula( QPainter * ) const;

private:
    QString d_formula;
};

#endif
