/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_POLAR_H
#define QWT_POLAR_H 1

#include "qwt_global.h"

namespace QwtPolar
{
	enum Coordinate
	{
		Azimuth,
		Radius
	};

	enum Axis
	{
		AxisAzimuth,

		AxisLeft,
		AxisRight,
		AxisTop,
		AxisBottom,

		AxesCount
	};

	enum Scale
	{
		ScaleAzimuth = Azimuth,
		ScaleRadius = Radius,

		ScaleCount 
	};

};

#endif
