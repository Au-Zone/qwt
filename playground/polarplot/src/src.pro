# -*- mode: sh -*- ###########################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
##############################################

TARGET            = polarplot
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
    qwt_radial_plot.h

SOURCES += \
    qwt_radial_plot.cpp 
