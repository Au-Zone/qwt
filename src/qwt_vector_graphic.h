/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_VECTOR_GRAPHIC_H
#define QWT_VECTOR_GRAPHIC_H

#include "qwt_global.h"
#include <qpicture.h>
#include <qmetatype.h>

class QWT_EXPORT QwtVectorGraphic: public QPicture
{
public:
    QwtVectorGraphic();
    virtual ~QwtVectorGraphic();

    QwtVectorGraphic& operator=(const QwtVectorGraphic &p);

    QRectF boundingRectF() const;

    bool operator==(const QwtVectorGraphic &) const;
    bool operator!=(const QwtVectorGraphic &) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

Q_DECLARE_METATYPE( QwtVectorGraphic )

#endif
