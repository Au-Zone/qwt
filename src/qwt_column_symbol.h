/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_COLUMN_SYMBOL_H
#define QWT_COLUMN_SYMBOL_H

#include <qpen.h>
#include <qsize.h>
#include "qwt_global.h"

class QPainter;
class QRect;

//! A drawing primitive for columns
class QWT_EXPORT QwtColumnSymbol
{
public:
    /*!
        Style
        \sa setStyle(), style()
     */
    enum Style 
    { 
        NoSymbol = -1, 

        Box, 

        StyleCnt 
    };
   
public:
    QwtColumnSymbol(Style = NoSymbol);
    virtual ~QwtColumnSymbol();
    
    bool operator!=(const QwtColumnSymbol &) const;
    virtual bool operator==(const QwtColumnSymbol &) const;

    virtual QwtColumnSymbol *clone() const;

    void setBrush(const QBrush& b);
    const QBrush& brush() const;

    void setPen(const QPen &);
    const QPen& pen() const; 

    void setStyle(Style);
    Style style() const;
    
    virtual void draw(QPainter *, Qt::Orientation, const QRect&) const;

private:
    class PrivateData;
    PrivateData* d_data;
};

#endif
