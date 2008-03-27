/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <cfloat>
#include <cmath>
#include <qpainter.h>
#include <qpen.h>
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_scale_map.h"
#include "qwt_scale_div.h"
#include "qwt_scale_draw.h"
#include "qwt_round_scale_draw.h"
#include "qwt_radial_plot_grid.h"
#if 1
#include <QDebug>
#endif

static inline bool isClose(double value1, double value2 )
{
	return qwtAbs(value1 - value2) < DBL_EPSILON;
}

class QwtRadialPlotGrid::AxisData
{
public:
	AxisData():
		isVisible(false),
		scaleDraw(NULL)
	{
	}
	~AxisData()
	{
		delete scaleDraw;
	}

	bool isVisible;
	mutable QwtAbstractScaleDraw *scaleDraw;
    QPen pen;
    QFont font;
};

class QwtRadialPlotGrid::GridData
{
public:
	GridData():
		isVisible(true),
		isMinorVisible(false)
	{
	}

	bool isVisible;
	bool isMinorVisible;
    QwtScaleDiv scaleDiv;

    QPen majorPen;
    QPen minorPen;
};

class QwtRadialPlotGrid::PrivateData
{
public:
	GridData gridData[QwtRadialPlot::ScaleCount];
	AxisData axisData[QwtRadialPlotGrid::AxesCount];
	int displayFlags;
};

QwtRadialPlotGrid::QwtRadialPlotGrid():
    QwtRadialPlotItem(QwtText("Grid"))
{
    d_data = new PrivateData;

	for ( int axisId = 0; axisId < AxesCount; axisId++ )
	{
		AxisData &axis = d_data->axisData[axisId];
		switch(axisId)
		{
			case AngleAxis:
			{
				axis.scaleDraw = new QwtRoundScaleDraw;
				axis.scaleDraw->setTickLength(QwtScaleDiv::MinorTick, 2);
				axis.scaleDraw->setTickLength(QwtScaleDiv::MediumTick, 2);
				axis.scaleDraw->setTickLength(QwtScaleDiv::MajorTick, 4);
				axis.isVisible = true;
				break;
			}
			case LeftAxis:
			{
				QwtScaleDraw *scaleDraw = new QwtScaleDraw;
				scaleDraw->setAlignment(QwtScaleDraw::BottomScale);

				axis.scaleDraw = scaleDraw;
				axis.isVisible = false;
				break;
			}
			case RightAxis:
			{
				QwtScaleDraw *scaleDraw = new QwtScaleDraw;
				scaleDraw->setAlignment(QwtScaleDraw::BottomScale);

				axis.scaleDraw = scaleDraw;
				axis.isVisible = true;
				break;
			}
			case TopAxis:
			{
				QwtScaleDraw *scaleDraw = new QwtScaleDraw;
				scaleDraw->setAlignment(QwtScaleDraw::LeftScale);

				axis.scaleDraw = scaleDraw;
				axis.isVisible = false;
				break;
			}
			case BottomAxis:
			{
				QwtScaleDraw *scaleDraw = new QwtScaleDraw;
				scaleDraw->setAlignment(QwtScaleDraw::LeftScale);

				axis.scaleDraw = scaleDraw;
				axis.isVisible = true;
				break;
			}
			default:;
		}
	}
	d_data->displayFlags = 0;
	d_data->displayFlags |= SmartOriginLabel;
	d_data->displayFlags |= HideMaxDistanceValue;
	d_data->displayFlags |= ClipAxisBackground;
	d_data->displayFlags |= SmartScaleDraw;

    setZ(10.0);
#if QT_VERSION >= 0x040000
    setRenderHint(RenderAntialiased, true);
#endif
}

QwtRadialPlotGrid::~QwtRadialPlotGrid()
{
    delete d_data;
}

int QwtRadialPlotGrid::rtti() const
{
    return QwtRadialPlotItem::Rtti_RadialPlotGrid;
}

void QwtRadialPlotGrid::setDisplayFlag(DisplayFlag flag, bool on)
{
    if ( ((d_data->displayFlags & flag) != 0) != on )
    {
    	if ( on )
        	d_data->displayFlags |= flag;
    	else
        	d_data->displayFlags &= ~flag;

		itemChanged();
	}
}

bool QwtRadialPlotGrid::testDisplayFlag(DisplayFlag flag) const
{
    return (d_data->displayFlags & flag);
}

void QwtRadialPlotGrid::showGrid(int scaleId, bool show)
{
    if ( scaleId < 0 || scaleId >= QwtRadialPlot::ScaleCount )
		return;

	GridData &grid = d_data->gridData[scaleId];
    if ( grid.isVisible != show )
    {
        grid.isVisible = show;
        itemChanged();
    }
}

bool QwtRadialPlotGrid::isGridVisible(int scaleId) const
{ 
    if ( scaleId < 0 || scaleId >= QwtRadialPlot::ScaleCount )
        return false;

    return d_data->gridData[scaleId].isVisible;
}

void QwtRadialPlotGrid::showMinorGrid(int scaleId, bool show)
{
    if ( scaleId < 0 || scaleId >= QwtRadialPlot::ScaleCount )
		return;

	GridData &grid = d_data->gridData[scaleId];
    if ( grid.isMinorVisible != show )
    {
        grid.isMinorVisible = show;
        itemChanged();
    }
}

bool QwtRadialPlotGrid::isMinorGridVisible(int scaleId) const
{ 
    if ( scaleId < 0 || scaleId >= QwtRadialPlot::ScaleCount )
        return false;

    return d_data->gridData[scaleId].isMinorVisible;
}

void QwtRadialPlotGrid::showAxis(int axisId, bool show)
{
    if ( axisId < 0 || axisId >= AxesCount )
		return;

	AxisData &axisData = d_data->axisData[axisId];
	if ( axisData.isVisible != show )
	{
		axisData.isVisible = show;
		itemChanged();
	}
}

bool QwtRadialPlotGrid::isAxisVisible(int axisId) const
{
    if ( axisId < 0 || axisId >= AxesCount )
		return false;

	return d_data->axisData[axisId].isVisible;
}

void QwtRadialPlotGrid::setScaleDiv(int scaleId, const QwtScaleDiv &scaleDiv)
{
    if ( scaleId < 0 || scaleId >= QwtRadialPlot::ScaleCount )
		return;

	GridData &grid = d_data->gridData[scaleId];
    if ( grid.scaleDiv != scaleDiv )
    {
        grid.scaleDiv = scaleDiv;
        itemChanged();
    }
}

QwtScaleDiv QwtRadialPlotGrid::scaleDiv(int scaleId) const 
{ 
    if ( scaleId < 0 || scaleId >= QwtRadialPlot::ScaleCount )
		return QwtScaleDiv();

	return d_data->gridData[scaleId].scaleDiv;
}

void QwtRadialPlotGrid::setPen(const QPen &pen)
{
	bool isChanged = false;

	for ( int scaleId = 0; scaleId < QwtRadialPlot::ScaleCount; scaleId++ )
	{
		GridData &grid = d_data->gridData[scaleId];
    	if ( grid.majorPen != pen || grid.minorPen != pen )
		{
			grid.majorPen = pen;
			grid.minorPen = pen;
			isChanged = true;
		}
	}
	if ( isChanged )
		itemChanged();
}

void QwtRadialPlotGrid::setMajorGridPen(const QPen &pen)
{
	bool isChanged = false;

	for ( int scaleId = 0; scaleId < QwtRadialPlot::ScaleCount; scaleId++ )
	{
		GridData &grid = d_data->gridData[scaleId];
    	if ( grid.majorPen != pen )
		{
			grid.majorPen = pen;
			isChanged = true;
		}
	}
	if ( isChanged )
		itemChanged();
}

void QwtRadialPlotGrid::setMajorGridPen(int scaleId, const QPen &pen)
{
    if ( scaleId < 0 || scaleId >= QwtRadialPlot::ScaleCount )
        return;

    GridData &grid = d_data->gridData[scaleId];
	if ( grid.majorPen != pen )
    {
    	grid.majorPen = pen;
		itemChanged();
	}
}

QPen QwtRadialPlotGrid::majorGridPen(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= QwtRadialPlot::ScaleCount )
        return QPen();
    
    const GridData &grid = d_data->gridData[scaleId];
    return grid.majorPen;
}   

void QwtRadialPlotGrid::setMinorGridPen(const QPen &pen)
{
    bool isChanged = false;

    for ( int scaleId = 0; scaleId < QwtRadialPlot::ScaleCount; scaleId++ )
    {
        GridData &grid = d_data->gridData[scaleId];
        if ( grid.minorPen != pen )
        {
            grid.minorPen = pen;
            isChanged = true;
        }   
    }   
    if ( isChanged )
        itemChanged();
}

void QwtRadialPlotGrid::setMinorGridPen(int scaleId, const QPen &pen)
{
    if ( scaleId < 0 || scaleId >= QwtRadialPlot::ScaleCount )
        return;

    GridData &grid = d_data->gridData[scaleId];
    if ( grid.minorPen != pen )
    {
        grid.minorPen = pen;
        itemChanged();
    }
}

QPen QwtRadialPlotGrid::minorGridPen(int scaleId) const
{ 
    if ( scaleId < 0 || scaleId >= QwtRadialPlot::ScaleCount )
        return QPen();

    const GridData &grid = d_data->gridData[scaleId];
    return grid.minorPen;
}

void QwtRadialPlotGrid::draw(QPainter *painter, 
    const QwtScaleMap &distanceMap, const QwtScaleMap &angleMap,
    const QRect &canvasRect) const
{
	updateScaleDraws(distanceMap, angleMap, canvasRect);

	painter->save();

    const QPoint center = canvasRect.center();
    const int radius = qRound(qAbs(distanceMap.p2() - distanceMap.p1()));

	if ( testDisplayFlag(ClipAxisBackground) )
	{
		QRegion clipRegion(canvasRect);
		for ( int axisId = 0; axisId < AxesCount; axisId++ )
		{
			const AxisData &axis = d_data->axisData[axisId];
			if ( axisId != AngleAxis && axis.isVisible )
			{
				QwtScaleDraw *scaleDraw = (QwtScaleDraw *)axis.scaleDraw;
				if ( scaleDraw->hasComponent(QwtScaleDraw::Labels) )
				{
					const QwtValueList &ticks = 
						scaleDraw->scaleDiv().ticks(QwtScaleDiv::MajorTick);
					for ( int i = 0; i < ticks.size(); i++ )
					{
						QRect labelRect =
							scaleDraw->boundingLabelRect(axis.font, ticks[i]);

						const int margin = 2;
						labelRect.setRect(
							labelRect.x() - margin,
							labelRect.y() - margin,
							labelRect.width() + 2 * margin,
							labelRect.height() + 2 * margin
						);
							
						if ( labelRect.isValid() )
							clipRegion -= QRegion(labelRect);
					}
				}
			}
		}
		painter->setClipRegion(clipRegion);
	}

    //  draw distance grid
    
	const GridData &distanceGrid = 
		d_data->gridData[QwtRadialPlot::DistanceScale];

    if (distanceGrid.isVisible && distanceGrid.isMinorVisible)
    {
    	painter->setPen(distanceGrid.minorPen);
		
        drawCircles(painter, center, distanceMap, 
			distanceGrid.scaleDiv.ticks(QwtScaleDiv::MinorTick) );
        drawCircles(painter, center, distanceMap, 
			distanceGrid.scaleDiv.ticks(QwtScaleDiv::MediumTick) );
    }
    if (distanceGrid.isVisible)
	{
    	painter->setPen(distanceGrid.majorPen);

        drawCircles(painter, center, distanceMap, 
			distanceGrid.scaleDiv.ticks(QwtScaleDiv::MajorTick) );
	}

    // draw angle grid

	const GridData &angleGrid = 
		d_data->gridData[QwtRadialPlot::AngleScale];

    if (angleGrid.isVisible && angleGrid.isMinorVisible)
    {
    	painter->setPen(angleGrid.minorPen);

        drawLines(painter, center, radius, angleMap, 
            angleGrid.scaleDiv.ticks(QwtScaleDiv::MinorTick));
        drawLines(painter, center, radius, angleMap, 
            angleGrid.scaleDiv.ticks(QwtScaleDiv::MediumTick));
    }
    if (angleGrid.isVisible)
    {   
        painter->setPen(angleGrid.majorPen);

        drawLines(painter, center, radius, angleMap,
            angleGrid.scaleDiv.ticks(QwtScaleDiv::MajorTick));
    }
	painter->restore();

	for ( int axisId = 0; axisId < AxesCount; axisId++ )
	{
		const AxisData &axis = d_data->axisData[axisId];
		if ( axis.isVisible )
		{
			painter->save();
			drawAxis(painter, axisId);
			painter->restore();
		}
	}
}

void QwtRadialPlotGrid::drawLines(QPainter *painter, 
    const QPoint &center, int radius,
    const QwtScaleMap &angleMap, const QwtValueList &values) const
{
    for ( int i = 0; i < values.size(); i++ )
    {
        double angle = angleMap.xTransform(values[i]);
		angle = ::fmod(angle, 2 * M_PI);

		bool skipLine = false;
        if ( testDisplayFlag(SmartScaleDraw) )
		{
			const QwtAbstractScaleDraw::ScaleComponent bone = 
				QwtAbstractScaleDraw::Backbone;
			if ( isClose(angle, 0.0) )
			{
				const AxisData &axis = d_data->axisData[RightAxis];
				if ( axis.isVisible && axis.scaleDraw->hasComponent(bone) )
					skipLine = true;
			}
			else if ( isClose(angle, M_PI / 2) )
			{
				const AxisData &axis = d_data->axisData[TopAxis];
				if ( axis.isVisible && axis.scaleDraw->hasComponent(bone) )
					skipLine = true;
			}
			else if ( isClose(angle, M_PI) )
			{
				const AxisData &axis = d_data->axisData[LeftAxis];
				if ( axis.isVisible && axis.scaleDraw->hasComponent(bone) )
					skipLine = true;
			}
			else if ( isClose(angle, 3 * M_PI / 2.0) )
			{
				const AxisData &axis = d_data->axisData[BottomAxis];
				if ( axis.isVisible && axis.scaleDraw->hasComponent(bone) )
					skipLine = true;
			}
		}
		if ( !skipLine )
		{
        	const QPoint pos = qwtPolar2Pos(center, radius, angle);
        	painter->drawLine(center, pos);
		}
    }
}

void QwtRadialPlotGrid::drawCircles(QPainter *painter, const QPoint &center,
    const QwtScaleMap &distanceMap, const QwtValueList &values) const
{
    for ( int i = 0; i < values.size(); i++ )
    {
		const double val = values[i];

		const GridData &gridData = 
			d_data->gridData[QwtRadialPlot::DistanceScale];

		bool skipLine = false;
        if ( testDisplayFlag(SmartScaleDraw) )
		{
			const AxisData &axis = d_data->axisData[AngleAxis];
			if ( axis.isVisible &&
				axis.scaleDraw->hasComponent(QwtAbstractScaleDraw::Backbone) )
			{
				if ( isClose(val, gridData.scaleDiv.hBound()) )
					skipLine = true;
			}
		}

		if ( isClose(val, gridData.scaleDiv.lBound()) )
			skipLine = true;

		if ( !skipLine )
		{
			const int radius = distanceMap.transform(val) - center.x();
			const QRect r(center.x() - radius, center.y() - radius, 
				2 * radius, 2 * radius);
			painter->drawEllipse(r);
		}
	}
}

void QwtRadialPlotGrid::drawAxis(QPainter *painter, int axisId) const
{
	if ( axisId < 0 || axisId >= AxesCount )
		return;

	AxisData &axis = d_data->axisData[axisId];

	painter->setPen(axis.pen);
    painter->setFont(axis.font);

#if QT_VERSION < 0x040000
    QColorGroup cg;
    cg.setColor(QColorGroup::Foreground, axis.pen.color());
    cg.setColor(QColorGroup::Text, axis.pen.color());

    axis.scaleDraw->draw(painter, cg);
#else
    QPalette pal;
    pal.setColor(QPalette::Foreground, axis.pen.color());
    pal.setColor(QPalette::Text, axis.pen.color());

    axis.scaleDraw->draw(painter, pal);
#endif
}

void QwtRadialPlotGrid::updateScaleDraws(const QwtScaleMap &distanceMap, 
	const QwtScaleMap &angleMap, const QRect &rect) const
{
    const QPoint center = rect.center();
    const int radius = rect.width() / 2;

	for ( int axisId = 0; axisId < AxesCount; axisId++ )
	{
    	AxisData &axis = d_data->axisData[axisId];

		if ( axisId == AngleAxis )
		{
    		QwtRoundScaleDraw *scaleDraw = (QwtRoundScaleDraw *)axis.scaleDraw;

    		scaleDraw->setRadius(radius);
    		scaleDraw->moveCenter(center);

    		scaleDraw->setAngleRange(90.0, -270.0);
    		scaleDraw->setTransformation(angleMap.transformation()->copy());
		}
		else
		{
			QwtScaleDraw *scaleDraw = (QwtScaleDraw *)axis.scaleDraw;
			switch(axisId)
			{
				case LeftAxis:
				{
					scaleDraw->move(center.x(), center.y());
					scaleDraw->setLength(-radius);
					break;
				}
				case RightAxis:
				{
					scaleDraw->move(center.x(), center.y());
					scaleDraw->setLength(radius);
					break;
				}
				case TopAxis:
				{
					scaleDraw->move(center.x(), rect.top());
					scaleDraw->setLength(radius);
					break;
				}
				case BottomAxis:
				{
					scaleDraw->move(center.x(), rect.bottom());
					scaleDraw->setLength(-radius);
					break;
				}
			}
			scaleDraw->setTransformation(distanceMap.transformation()->copy());
		}
	}
}

void QwtRadialPlotGrid::updateScaleDiv(const QwtScaleDiv &distanceScaleDiv,
    const QwtScaleDiv &angleScaleDiv)
{
    GridData &distanceGrid = d_data->gridData[QwtRadialPlot::DistanceScale];
    if ( distanceGrid.scaleDiv != distanceScaleDiv )
    {
        distanceGrid.scaleDiv = distanceScaleDiv;
    }

    GridData &angleGrid = d_data->gridData[QwtRadialPlot::AngleScale];
    if ( angleGrid.scaleDiv != angleScaleDiv )
    {
        angleGrid.scaleDiv = angleScaleDiv;
    }

	bool hasOrigin = false;
	for ( int axisId = 0; axisId < AxesCount; axisId++ )
	{
	    AxisData &axis = d_data->axisData[axisId];
		if ( axis.isVisible && axis.scaleDraw )
		{
			if ( axisId == AngleAxis )
			{
				axis.scaleDraw->setScaleDiv(angleScaleDiv);
				if ( testDisplayFlag(SmartScaleDraw) )
				{
					axis.scaleDraw->enableComponent(
						QwtAbstractScaleDraw::Ticks, !angleGrid.isVisible);
				}
			}
			else
			{
				QwtScaleDiv sd = distanceScaleDiv;

				QwtValueList &ticks = 
						(QwtValueList &)sd.ticks(QwtScaleDiv::MajorTick);

				if ( testDisplayFlag(SmartOriginLabel) )
				{
					bool skipOrigin = hasOrigin;
					if ( !skipOrigin )
					{
						if ( axisId == LeftAxis || axisId == RightAxis )
						{
							if ( d_data->axisData[BottomAxis].isVisible )
								skipOrigin = true;
						}
						else
						{
							if ( d_data->axisData[LeftAxis].isVisible )
								skipOrigin = true;
						}
					}
					if ( ticks.size() > 0 && ticks.first() == sd.lBound() )
					{
						if ( skipOrigin )
							ticks.removeFirst();
						else
							hasOrigin = true;
					}
				}

				if ( testDisplayFlag(HideMaxDistanceValue) )
				{
					if ( ticks.size() > 0 && ticks.last() == sd.hBound() )
						ticks.removeLast();
				}

				axis.scaleDraw->setScaleDiv(sd);

                if ( testDisplayFlag(SmartScaleDraw) )
				{
                    axis.scaleDraw->enableComponent(
                        QwtAbstractScaleDraw::Ticks, !distanceGrid.isVisible);
				}

			}
		}
	}

#if 1
	itemChanged();
#endif
}

QRect QwtRadialPlotGrid::canvasLayoutHint(const QRect &rect) const
{
    const AxisData &axis = d_data->axisData[AngleAxis];
	if ( axis.isVisible )
	{
		const int extent = axis.scaleDraw->extent(axis.pen, axis.font);
		if ( extent > 0 )
		{
		    int w = qwtMin(rect.width(), rect.height());
			w -= 2 * extent;
			if ( w < 0 )
				w = 0;
			
			QRect r(0, 0, w, w);
			r.moveCenter(rect.center());
			return r;
		}
	}
	
    return rect;
}

