/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_ABSTRACT_SCALE_H
#define QWT_ABSTRACT_SCALE_H

#include "qwt_global.h"
#include <qwidget.h>

class QwtScaleEngine;
class QwtAbstractScaleDraw;
class QwtScaleDiv;
class QwtScaleMap;
class QwtInterval;

/*!
  \brief An abstract base class for widgets having a scale

  QwtAbstractScale is used to provide classes with a QwtScaleDraw,
  and a QwtScaleDiv. The QwtScaleDiv might be set explicitely
  or calculated by a QwtScaleEngine.
*/

class QWT_EXPORT QwtAbstractScale: public QWidget
{
    Q_OBJECT

    Q_PROPERTY( double lowerBound READ lowerBound WRITE setLowerBound )
    Q_PROPERTY( double upperBound READ upperBound WRITE setUpperBound )

public:
    QwtAbstractScale( QWidget *parent = NULL );
    virtual ~QwtAbstractScale();

    void setScale( double vmin, double vmax, double step = 0.0 );
    void setScale( const QwtInterval &, double step = 0.0 );
    void setScale( const QwtScaleDiv & );

    void setLowerBound( double min );
    double lowerBound() const;

    void setUpperBound( double max );
    double upperBound() const;

    double scaleStepSize() const;
    const QwtScaleDiv& scaleDiv() const;

    void setScaleMaxMajor( int ticks );
    int scaleMaxMinor() const;

    void setScaleMaxMinor( int ticks );
    int scaleMaxMajor() const;

    void setScaleEngine( QwtScaleEngine * );
    const QwtScaleEngine *scaleEngine() const;
    QwtScaleEngine *scaleEngine();

    int transform( double ) const;
    double invTransform( int ) const;

    bool isInverted() const;

    double minimum() const;
    double maximum() const;

	const QwtScaleMap &scaleMap() const;

protected:
    void rescale( double vmin, double vmax, double step = 0.0 );

    void setAbstractScaleDraw( QwtAbstractScaleDraw * );

    const QwtAbstractScaleDraw *abstractScaleDraw() const;
    QwtAbstractScaleDraw *abstractScaleDraw();

    virtual void scaleChange();

private:
    void updateScaleDraw();

    class PrivateData;
    PrivateData *d_data;
};

#endif
