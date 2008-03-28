# -*- mode: sh -*- ###########################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
##############################################

TARGET            = qwtpolar
TEMPLATE          = lib

VERSION      = 0.1.0

MOC_DIR           = moc
OBJECTS_DIR       = obj
DESTDIR           = ../lib

CONFIG           += qt     # Also for Qtopia Core!
CONFIG           += warn_on
CONFIG           += thread
CONFIG           += debug
CONFIG           += dll

INCLUDEPATH += ../../../src
DEPENDPATH  += ../../../src

HEADERS += \
	qwt_polar.h \
	qwt_polar_item.h \
	qwt_polar_grid.h \
	qwt_polar_curve.h \
	qwt_polar_itemdict.h \
    qwt_polar_plot.h

SOURCES += \
	qwt_polar_item.cpp \
	qwt_polar_grid.cpp \
	qwt_polar_curve.cpp \
	qwt_polar_itemdict.cpp \
    qwt_polar_plot.cpp
