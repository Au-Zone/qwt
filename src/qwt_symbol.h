/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SYMBOL_H
#define QWT_SYMBOL_H

#include "qwt_global.h"
#include <qbrush.h>
#include <qpen.h>
#include <qsize.h>

class QPainter;
class QRect;

//! A class for drawing symbols
class QWT_EXPORT QwtSymbol
{
public:
    /*!
        Style
        \sa setStyle(), style()
     */
    enum Style 
    { 
        NoSymbol = -1, 

        Ellipse, 
        Rect, 
        Diamond, 
        Triangle, 
        DTriangle,
        UTriangle, 
        LTriangle, 
        RTriangle, 
        Cross, 
        XCross, 
        HLine, 
        VLine, 
        Star1, 
        Star2, 
        Hexagon, 

        StyleCnt 
    };
   
public:
    QwtSymbol(Style = NoSymbol);
    QwtSymbol(Style, const QBrush &, const QPen &, const QSizeF &);
    virtual ~QwtSymbol();
    
    bool operator!=(const QwtSymbol &) const;
    virtual bool operator==(const QwtSymbol &) const;

    void setSize(const QSizeF &);
    void setSize(double width, double height = -1.0);
    const QSizeF& size() const;

    virtual void setColor(const QColor &);

    void setBrush(const QBrush& b);
    const QBrush& brush() const;

    void setPen(const QPen &);
    const QPen& pen() const;

    void setStyle(Style);
    Style style() const;

    void draw(QPainter *p, const QPointF &) const; 
    void draw(QPainter *p, double x, double y) const;

    virtual void draw(QPainter *p, const QRectF &r) const;

private:
    // Disabled copy constructor and operator=
    QwtSymbol( const QwtSymbol & );
    QwtSymbol &operator=( const QwtSymbol & );

    QBrush d_brush;
    QPen d_pen;
    QSizeF d_size;
    Style d_style;
};

//! Return Brush
inline const QBrush& QwtSymbol::brush() const 
{ 
    return d_brush; 
}
    
//! Return Pen
inline const QPen& QwtSymbol::pen() const 
{ 
    return d_pen; 
}
    
//! Return Style
inline QwtSymbol::Style QwtSymbol::style() const 
{
    return d_style; 
}

//! Return Size
inline const QSizeF& QwtSymbol::size() const 
{ 
    return d_size; 
}
    
#endif
