/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_RADIAL_PLOT_ITEM_H
#define QWT_RADIAL_PLOT_ITEM_H

#include "qwt_global.h"
#include "qwt_text.h"
#include "qwt_double_rect.h"

class QString;
class QRect;
class QPainter;
class QwtRadialPlot;
class QwtScaleMap;
class QwtScaleDiv;

/*!
  \brief Base class for items on the plot canvas
*/

class QWT_EXPORT QwtRadialPlotItem
{
public:
    enum RttiValues
    { 
        Rtti_RadialPlotItem = 0,

        Rtti_RadialPlotGrid,
        Rtti_RadialPlotScale,
        Rtti_RadialPlotMarker,
        Rtti_RadialPlotCurve,

        Rtti_RadialPlotUserItem = 1000
    };

    enum ItemAttribute
    {
        AutoScale = 1
    };

#if QT_VERSION >= 0x040000
    enum RenderHint
    {
        RenderAntialiased = 1
    };
#endif

    explicit QwtRadialPlotItem(const QwtText &title = QwtText());
    virtual ~QwtRadialPlotItem();

    void attach(QwtRadialPlot *plot);

    /*!
       \brief This method detaches a QwtRadialPlotItem from any QwtRadialPlot is has been
              associated with.

       detach() is equivalent to calling attach( NULL )
       \sa attach( QwtRadialPlot* plot )
    */
    void detach() { attach(NULL); }

    QwtRadialPlot *plot() const;
    
    void setTitle(const QString &title);
    void setTitle(const QwtText &title);
    const QwtText &title() const;

    virtual int rtti() const;

    void setItemAttribute(ItemAttribute, bool on = true);
    bool testItemAttribute(ItemAttribute) const;

#if QT_VERSION >= 0x040000
    void setRenderHint(RenderHint, bool on = true);
    bool testRenderHint(RenderHint) const;
#endif

    double z() const; 
    void setZ(double z);

    void show();
    void hide();
    virtual void setVisible(bool);
    bool isVisible () const;

    virtual void itemChanged();

    /*!
      \brief Draw the item

      \param painter Painter
      \param distanceMap Maps distance values into pixel coordinates.
      \param angleMap Maps angle values into pixel coordinates.
      \param canvasRect Contents rect of the canvas in painter coordinates
    */
    virtual void draw(QPainter *painter, 
        const QwtScaleMap &distanceMap, const QwtScaleMap &angleMap,
        const QRect &canvasRect) const = 0;

    virtual QwtDoubleRect boundingRect() const;

    virtual void updateScaleDiv(const QwtScaleDiv&,
        const QwtScaleDiv&);

	virtual QRect canvasLayoutHint(const QRect &) const;

private:
    // Disabled copy constructor and operator=
    QwtRadialPlotItem( const QwtRadialPlotItem & );
    QwtRadialPlotItem &operator=( const QwtRadialPlotItem & );

    class PrivateData;
    PrivateData *d_data;
};
            
#endif
