/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qpainter.h>
#include <qpen.h>
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_scale_map.h"
#include "qwt_scale_div.h"
#include "qwt_radial_plot_grid.h"
#if 1
#include <QDebug>
#endif

class QwtRadialPlotGrid::PrivateData
{
public:
    PrivateData():
        xEnabled(true),
        yEnabled(true),
        xMinEnabled(false),
        yMinEnabled(false)
    {
    }

    bool xEnabled;
    bool yEnabled;
    bool xMinEnabled;
    bool yMinEnabled;

    QwtScaleDiv sdx;
    QwtScaleDiv sdy;

    QPen majPen;
    QPen minPen;
};

//! Enables major grid, disables minor grid
QwtRadialPlotGrid::QwtRadialPlotGrid():
    QwtRadialPlotItem(QwtText("Grid"))
{
    d_data = new PrivateData;
    setZ(10.0);
}

//! dtor
QwtRadialPlotGrid::~QwtRadialPlotGrid()
{
    delete d_data;
}

int QwtRadialPlotGrid::rtti() const
{
    return QwtRadialPlotItem::Rtti_RadialPlotGrid;
}

/*!
  \brief Enable or disable vertical gridlines
  \param tf Enable (true) or disable

  \sa Minor gridlines can be enabled or disabled with
      enableXMin()
*/
void QwtRadialPlotGrid::enableX(bool tf)
{
    if ( d_data->xEnabled != tf )
    {
        d_data->xEnabled = tf;
        itemChanged();
    }
}

/*!
  \brief Enable or disable horizontal gridlines
  \param tf Enable (true) or disable
  \sa Minor gridlines can be enabled or disabled with enableYMin()
*/
void QwtRadialPlotGrid::enableY(bool tf)
{
    if ( d_data->yEnabled != tf )
    {
        d_data->yEnabled = tf;  
        itemChanged();
    }
}

/*!
  \brief Enable or disable  minor vertical gridlines.
  \param tf Enable (true) or disable
  \sa enableX()
*/
void QwtRadialPlotGrid::enableXMin(bool tf)
{
    if ( d_data->xMinEnabled != tf )
    {
        d_data->xMinEnabled = tf;
        itemChanged();
    }
}

/*!
  \brief Enable or disable minor horizontal gridlines
  \param tf Enable (true) or disable
  \sa enableY()
*/
void QwtRadialPlotGrid::enableYMin(bool tf)
{
    if ( d_data->yMinEnabled != tf )
    {
        d_data->yMinEnabled = tf;
        itemChanged();
    }
}

/*!
  \brief Assign an x axis scale division
  \param sx Scale division
  \warning QwtRadialPlotGrid uses implicit sharing (see Qt Manual) for
  the scale divisions.
*/
void QwtRadialPlotGrid::setXDiv(const QwtScaleDiv &sx)
{
    if ( d_data->sdx != sx )
    {
        d_data->sdx = sx;
        itemChanged();
    }
}

/*!
  \brief Assign a y axis division
  \param sy Scale division
  \warning QwtRadialPlotGrid uses implicit sharing (see Qt Manual) for
  the scale divisions.
*/
void QwtRadialPlotGrid::setYDiv(const QwtScaleDiv &sy)
{
    if ( d_data->sdy != sy )
    {
        d_data->sdy = sy;    
        itemChanged();
    }
}

/*!
  \brief Assign a pen for both major and minor gridlines
  \param p Pen
  \sa setMajPen(), setMinPen()
*/
void QwtRadialPlotGrid::setPen(const QPen &p)
{
    if ( d_data->majPen != p || d_data->minPen != p )
    {
        d_data->majPen = p;
        d_data->minPen = p;
        itemChanged();
    }
}

/*!
  \brief Assign a pen for the major gridlines
  \param p Pen
  \sa majPen(), setMinPen(), setPen()
*/
void QwtRadialPlotGrid::setMajPen(const QPen &p)
{
    if ( d_data->majPen != p )
    {
        d_data->majPen = p;
        itemChanged();
    }
}

/*!
  \brief Assign a pen for the minor gridlines
  \param p Pen
*/
void QwtRadialPlotGrid::setMinPen(const QPen &p)
{
    if ( d_data->minPen != p )
    {
        d_data->minPen = p;  
        itemChanged();
    }
}

/*!
  \brief Draw the grid
  
  The grid is drawn into the bounding rectangle such that 
  gridlines begin and end at the rectangle's borders. The X and Y
  maps are used to map the scale divisions into the drawing region
  screen.
  \param painter  Painter
  \param mx X axis map
  \param my Y axis 
  \param r Contents rect of the plot canvas
*/
void QwtRadialPlotGrid::draw(QPainter *painter, 
    const QwtScaleMap &distanceMap, const QwtScaleMap &angleMap,
    const QRect &canvasRect) const
{
qDebug() << "draw grid";
#if 0
    //  draw minor gridlines
    painter->setPen(d_data->minPen);
    
    if (d_data->xEnabled && d_data->xMinEnabled)
    {
        drawLines(painter, r, Qt::Vertical, mx, 
            d_data->sdx.ticks(QwtScaleDiv::MinorTick));
        drawLines(painter, r, Qt::Vertical, mx, 
            d_data->sdx.ticks(QwtScaleDiv::MediumTick));
    }

    if (d_data->yEnabled && d_data->yMinEnabled)
    {
        drawLines(painter, r, Qt::Horizontal, my, 
            d_data->sdy.ticks(QwtScaleDiv::MinorTick));
        drawLines(painter, r, Qt::Horizontal, my, 
            d_data->sdy.ticks(QwtScaleDiv::MediumTick));
    }

    //  draw major gridlines
    painter->setPen(d_data->majPen);
    
    if (d_data->xEnabled)
    {
        drawLines(painter, r, Qt::Vertical, mx,
            d_data->sdx.ticks(QwtScaleDiv::MajorTick));
    }

    if (d_data->yEnabled)
    {
        drawLines(painter, r, Qt::Horizontal, my,
            d_data->sdy.ticks(QwtScaleDiv::MajorTick));
    }
#endif
}

/*!
  \return the pen for the major gridlines
  \sa setMajPen(), setMinPen(), setPen()
*/
const QPen &QwtRadialPlotGrid::majPen() const 
{ 
    return d_data->majPen; 
}

/*!
  \return the pen for the minor gridlines
  \sa setMinPen(), setMajPen(), setPen()
*/
const QPen &QwtRadialPlotGrid::minPen() const 
{ 
    return d_data->minPen; 
}
  
/*!
  \return true if vertical gridlines are enabled
  \sa enableX()
*/
bool QwtRadialPlotGrid::xEnabled() const
{ 
    return d_data->xEnabled; 
}

/*!
  \return true if minor vertical gridlines are enabled
  \sa enableXMin()
*/
bool QwtRadialPlotGrid::xMinEnabled() const 
{ 
    return d_data->xMinEnabled; 
}

/*!
  \return true if horizontal gridlines are enabled
  \sa enableY()
*/
bool QwtRadialPlotGrid::yEnabled() const 
{ 
    return d_data->yEnabled; 
}

/*!
  \return true if minor horizontal gridlines are enabled
  \sa enableYMin()
*/
bool QwtRadialPlotGrid::yMinEnabled() const 
{
    return d_data->yMinEnabled; 
}

  
/*! \return the scale division of the x axis */
const QwtScaleDiv &QwtRadialPlotGrid::xScaleDiv() const 
{ 
    return d_data->sdx; 
}

/*! \return the scale division of the y axis */
const QwtScaleDiv &QwtRadialPlotGrid::yScaleDiv() const 
{ 
    return d_data->sdy; 
}
 
void QwtRadialPlotGrid::updateScaleDiv(const QwtScaleDiv& xDiv,
    const QwtScaleDiv& yDiv)
{
    setXDiv(xDiv);
    setYDiv(yDiv);
}
