/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_OVERLAY_H
#define QWT_PLOT_OVERLAY_H

#include <qwidget.h>

class QPainter;

class QwtPlotOverlay: public QWidget
{
public:
    enum MaskMode
    {
        NoMask,
		Mask,
		AlphaMask
    };

	enum RenderMode
	{
		AutoRenderMode,
		CopyAlphaMask,
		DrawOverlay
	};

    QwtPlotOverlay( QWidget* canvas );
    virtual ~QwtPlotOverlay();

	void updateOverlay();

    virtual bool eventFilter( QObject *, QEvent *);

protected:
    virtual void paintEvent( QPaintEvent* event );
    virtual void resizeEvent( QResizeEvent* event );

    virtual void drawOverlay( QPainter * ) const = 0;
    virtual QRegion maskHint() const;

    void updateMask();

private:
    void draw( QPainter * ) const;

private:
    class PrivateData;
    PrivateData *d_data;

    uchar* m_rgbaBuffer;
};

#endif
