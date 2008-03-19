/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_radial_plot.h"
#include "qwt_radial_plot_item.h"

class QwtRadialPlotItem::PrivateData
{
public:
    PrivateData():
        plot(NULL),
        isVisible(true),
        attributes(0),
#if QT_VERSION >= 0x040000
        renderHints(0),
#endif
        z(0.0),
        distanceAxis(QwtRadialPlot::DistanceScale1),
        angleAxis(QwtRadialPlot::AngleScale1)
    {
    }

    mutable QwtRadialPlot *plot;

    bool isVisible;
    int attributes;
#if QT_VERSION >= 0x040000
    int renderHints;
#endif
    double z;

    int distanceAxis;
    int angleAxis;

    QwtText title;
};

//! Constructor
QwtRadialPlotItem::QwtRadialPlotItem(const QwtText &title)
{
    d_data = new PrivateData;
    d_data->title = title;
}

//! Destroy the QwtRadialPlotItem
QwtRadialPlotItem::~QwtRadialPlotItem()
{
    attach(NULL);
    delete d_data;
}

/*! 
  \brief Attach the item to a plot.

  This method will attach a QwtRadialPlotItem to the QwtRadialPlot argument. It will first
  detach the QwtRadialPlotItem from any plot from a previous call to attach (if
  necessary). If a NULL argument is passed, it will detach from any QwtRadialPlot it
  was attached to.

  \sa QwtRadialPlotItem::detach()
*/
void QwtRadialPlotItem::attach(QwtRadialPlot *plot)
{
    if ( plot == d_data->plot )
        return;

    // remove the item from the previous plot

    if ( d_data->plot )
    {
        d_data->plot->attachItem(this, false);

        if ( d_data->plot->autoReplot() )
            d_data->plot->update();
    }

    d_data->plot = plot;

    if ( d_data->plot )
    {
        // insert the item into the current plot

        d_data->plot->attachItem(this, true);
        itemChanged();
    }
}

/*! 
   Return rtti for the specific class represented. QwtRadialPlotItem is simply
   a virtual interface class, and base classes will implement this method
   with specific rtti values so a user can differentiate them.

   The rtti value is useful for environments, where the 
   runtime type information is disabled and it is not possible
   to do a dynamic_cast<...>.
   
   \return rtti value
   \sa RttiValues
*/
int QwtRadialPlotItem::rtti() const
{
    return Rtti_RadialPlotItem;
}

//! Return attached plot
QwtRadialPlot *QwtRadialPlotItem::plot() const 
{ 
    return d_data->plot; 
}

/*!
   Plot items are painted in increasing z-order.

   \return setZ(), QwtRadialPlotDict::itemList()
*/
double QwtRadialPlotItem::z() const 
{ 
    return d_data->z; 
}

/*!
   \brief Set the z value

   Plot items are painted in increasing z-order.

   \param z Z-value
   \sa z(), QwtRadialPlotDict::itemList()
*/
void QwtRadialPlotItem::setZ(double z) 
{ 
    if ( d_data->z != z )
    {
        d_data->z = z; 
        if ( d_data->plot )
        {
            // update the z order
            d_data->plot->attachItem(this, false);
            d_data->plot->attachItem(this, true);
        }
        itemChanged();
    }
}

/*! 
   Set a new title

   \param title Title
   \sa title() 
*/  
void QwtRadialPlotItem::setTitle(const QString &title)
{
    setTitle(QwtText(title));
}

/*! 
   Set a new title

   \param title Title
   \sa title() 
*/  
void QwtRadialPlotItem::setTitle(const QwtText &title)
{
    if ( d_data->title != title )
    {
        d_data->title = title; 
        itemChanged();
    }
}

/*!
   \return Title of the item
   \sa setTitle()
*/
const QwtText &QwtRadialPlotItem::title() const
{
    return d_data->title;
}

/*!
   Toggle an item attribute
 
   \param attribute Attribute type
   \param on true/false

   \sa testItemAttribute(), ItemAttribute
*/
void QwtRadialPlotItem::setItemAttribute(ItemAttribute attribute, bool on)
{
    if ( bool(d_data->attributes & attribute) != on )
    {
        if ( on )
            d_data->attributes |= attribute;
        else
            d_data->attributes &= ~attribute;

        itemChanged();
    }
}

/*!
   Test an item attribute

   \param ItemAttribute Attribute type
   \return true/false
   \sa setItemAttribute(), ItemAttribute
*/
bool QwtRadialPlotItem::testItemAttribute(ItemAttribute attribute) const
{
    return d_data->attributes & attribute;
}

#if QT_VERSION >= 0x040000

/*!
   Toggle an render hint
 
   \param hint Render hint
   \param on true/false

   \sa testRenderHint(), RenderHint
*/
void QwtRadialPlotItem::setRenderHint(RenderHint hint, bool on)
{
    if ( ((d_data->renderHints & hint) != 0) != on )
    {
        if ( on )
            d_data->renderHints |= hint;
        else
            d_data->renderHints &= ~hint;

        itemChanged();
    }
}

/*!
   Test a render hint

   \param hint Render hint
   \return true/false
   \sa setRenderHint(), RenderHint
*/
bool QwtRadialPlotItem::testRenderHint(RenderHint hint) const
{
    return (d_data->renderHints & hint);
}

#endif

void QwtRadialPlotItem::show()
{
    setVisible(true);
}

void QwtRadialPlotItem::hide()
{
    setVisible(false);
}

/*! 
    Show/Hide the item

    \param on Show if true, otherwise hide
    \sa isVisible(), show(), hide()
*/
void QwtRadialPlotItem::setVisible(bool on) 
{ 
    if ( on != d_data->isVisible )
    {
        d_data->isVisible = on; 
        itemChanged(); 
    }
}

/*! 
    \return true if visible
    \sa setVisible(), show(), hide()
*/
bool QwtRadialPlotItem::isVisible() const
{ 
    return d_data->isVisible; 
}

/*! 
   Update the legend and call QwtRadialPlot::autoRefresh for the 
   parent plot.

   \sa updateLegend()
*/
void QwtRadialPlotItem::itemChanged()
{
    if ( d_data->plot )
        d_data->plot->autoRefresh();
}

/*!  
   Set X and Y axis

   The item will painted according to the coordinates its Axes.

   \param distanceAxis X Axis
   \param angleAxis Y Axis

   \sa setDistanceAxis(), setAngleAxis(), distanceAxis(), angleAxis()
*/
void QwtRadialPlotItem::setAxis(int distanceAxis, int angleAxis)
{
    if (QwtRadialPlot::isDistanceScale(distanceAxis))
       d_data->distanceAxis = distanceAxis;

    if (QwtRadialPlot::isAngleScale(angleAxis))
       d_data->angleAxis = angleAxis;

    itemChanged();    
}

/*!  
   Set the X axis

   The item will painted according to the coordinates its Axes.

   \param axis Distance Axis
   \sa setAxis(), setAngleAxis(), distanceAxis()
*/
void QwtRadialPlotItem::setDistanceAxis(int axis)
{
    if (QwtRadialPlot::isDistanceScale(axis))
    {
       d_data->distanceAxis = axis;
       itemChanged();    
    }
}

/*!  
   Set the Y axis

   The item will painted according to the coordinates its Axes.

   \param axis Angle Axis
   \sa setAxis(), setDistanceAxis(), angleAxis()
*/
void QwtRadialPlotItem::setAngleAxis(int axis)
{
    if (QwtRadialPlot::isAngleScale(axis))
    {
       d_data->angleAxis = axis;
       itemChanged();   
    }
}

//! Return distanceAxis
int QwtRadialPlotItem::distanceAxis() const 
{ 
    return d_data->distanceAxis; 
}

//! Return angleAxis
int QwtRadialPlotItem::angleAxis() const 
{ 
    return d_data->angleAxis; 
}

/*!
   \return An invalid bounding rect: QwtDoubleRect(1.0, 1.0, -2.0, -2.0)
*/
QwtDoubleRect QwtRadialPlotItem::boundingRect() const
{
    return QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid
}

/*!
   \brief Update the item to changes of the axes scale division

   Update the item, when the axes of plot have changed.
   The default implementation does nothing, but items that depend
   on the scale division (like QwtRadialPlotGrid()) have to reimplement
   updateScaleDiv()

   \param xScaleDiv Scale division of the x-axis
   \param yScaleDiv Scale division of the y-axis

   \sa QwtRadialPlot::updateAxes()
*/
void QwtRadialPlotItem::updateScaleDiv(const QwtScaleDiv &,
    const QwtScaleDiv &) 
{ 
}
