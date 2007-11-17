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
class QPalette;
class QRect;
class QwtText;

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
        RaisedBox, 

        StyleCnt 
    };
   
public:
    QwtColumnSymbol(Style = NoSymbol);
    virtual ~QwtColumnSymbol();
    
    bool operator!=(const QwtColumnSymbol &) const;
    virtual bool operator==(const QwtColumnSymbol &) const;

    virtual QwtColumnSymbol *clone() const;

    void setPalette(const QPalette &);
    const QPalette &palette() const;

    void setStyle(Style);
    Style style() const;
    
    void setLabel(const QwtText&);
    const QwtText &label() const;

    virtual void draw(QPainter *, Qt::Orientation, const QRect&) const;

protected:
    void drawBox(QPainter *, Qt::Orientation, const QRect&) const;
    void drawRaisedBox(QPainter *, Qt::Orientation, const QRect&) const;

private:
    class PrivateData;
    PrivateData* d_data;
};

#endif
