/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_RADIAL_PLOT_H
#define QWT_RADIAL_PLOT_H 1

#include <qwidget.h>
#include <qregion.h>
#include "qwt_global.h"
#include "qwt_scale_map.h"

class QwtRoundScaleDraw;
class QwtScaleEngine;
class QwtScaleDiv;

class QWT_EXPORT QwtRadialPlot: public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QBrush canvasBackground 
        READ canvasBackground WRITE setCanvasBackground);

public:
	enum Scale
	{
		AngleScale1,
		AngleScale2,
		AngleScale3,
		AngleScale4,

		DistanceScale1,
		DistanceScale2,
		DistanceScale3,
		DistanceScale4,
	
		ScaleCount
	};

    explicit QwtRadialPlot( QWidget *parent = NULL);
#if QT_VERSION < 0x040000
    explicit QwtRadialPlot( QWidget *parent, const char *name);
#endif

    virtual ~QwtRadialPlot();

    QRegion::RegionType shape() const;
    void setShape(QRegion::RegionType);

    void setCanvasBackground(const QBrush&);
    const QBrush &canvasBackground() const;

    void setAutoReplot(bool tf = true); 
    bool autoReplot() const;

	// angle scale only
    void showAngleScale(int scaleId, bool enable = true);
    bool isAngleScaleVisible(int scaleId) const;

    void setAngleScaleDraw(int scaleId, QwtRoundScaleDraw *);
    const QwtRoundScaleDraw *angleScaleDraw(int scaleId) const;
    QwtRoundScaleDraw *angleScaleDraw(int scaleId);

    double origin(int scaleId) const;

    // all scales
	void setAutoScale(int scaleId, bool enable);
	bool autoScale(int scaleId) const;

    void setScaleMaxMinor(int scaleId, int maxMinor);
    int scaleMaxMinor(int scaleId) const;

    int scaleMaxMajor(int scaleId) const;
    void setScaleMaxMajor(int scaleId, int maxMajor);

    QwtScaleEngine *scaleEngine(int scaleId);
    const QwtScaleEngine *scaleEngine(int scaleId) const;
    void setScaleEngine(int scaleId, QwtScaleEngine *);

    void setScale(int scaleId, double min, double max, double step = 0);

    void setScaleDiv(int scaleId, const QwtScaleDiv &);
    const QwtScaleDiv *scaleDiv(int scaleId) const;
    QwtScaleDiv *scaleDiv(int scaleId);

	QwtScaleMap scaleMap(int scaleId) const;

    QRect boundingRect() const;
    QRect canvasRect() const;

    virtual void polish();
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

public slots:
    void setOrigin(int scaleId, double);

    virtual void replot();
    void autoRefresh();

protected:
	bool event(QEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);

    virtual void updateMask();

	virtual void drawAngleScale(QPainter *, int scaleId, const QRect &) const;
	virtual void drawCanvas(QPainter *, const QRect &) const;

	void updateScale(int scaleId);

private:
    void initPlot();

	bool isValidScale(int scaleId) const;
	bool isAngleScale(int scaleId) const;
	bool isDistanceScale(int scaleId) const;
	int scaleExtent() const;

    class ScaleData;
    class PrivateData;
    PrivateData *d_data;
};

#endif
