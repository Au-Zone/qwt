/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_INTERVAL_BAR_H
#define QWT_INTERVAL_BAR_H

#include <qpen.h>
#include <qsize.h>
#include "qwt_global.h"

class QPainter;
class QRect;

//! A drawing primitive for bars
class QWT_EXPORT QwtBar
{
public:
    /*!
        Style
        \sa setStyle(), style()
     */
    enum Style 
    { 
        NoBar = -1, 

        ErrorBar, 
        Box, 

        StyleCnt 
    };
   
public:
    QwtBar(Style = NoBar);
    virtual ~QwtBar();
    
    bool operator!=(const QwtBar &) const;
    virtual bool operator==(const QwtBar &) const;

    virtual QwtBar *clone() const;

    void setWidth(int);
    int width() const;

    void setBrush(const QBrush& b);
    const QBrush& brush() const;

    void setPen(const QPen &);
    const QPen& pen() const; 

    void setStyle(Style);
    Style style() const;
    
    virtual void draw(QPainter *, Qt::Orientation, 
        const QPoint& from, const QPoint& to, int width) const;

private:
    class PrivateData;
    PrivateData* d_data;
};

#endif
