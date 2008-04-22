# -*- mode: sh -*- ################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
###################################################################

QWT_ROOT = ../..

include( $${QWT_ROOT}/qwtconfig.pri )

TEMPLATE     = app

MOC_DIR      = moc
OBJECTS_DIR  = obj
INCLUDEPATH += $${QWT_ROOT}/src
DEPENDPATH  += $${QWT_ROOT}/src

win32 {
	contains(CONFIG, QwtDll) {
		DEFINES    += QT_DLL QWT_DLL
		QWTLIB      = qwt$${VER_MAJ}
	}
	else {
		QWTLIB      = qwt
	}

    msvc:LIBS  += $${QWT_ROOT}/lib/$${QWTLIB}.lib
    msvc.net:LIBS  += $${QWT_ROOT}/lib/{QWTLIB}.lib
    msvc2005:LIBS += $${QWT_ROOT}/lib/{QWTLIB}.lib
    g++:LIBS   += -L$${QWT_ROOT}/lib -lqwt
}
else {
	LIBS        += -L$${QWT_ROOT}/lib -lqwt
}

contains(CONFIG, QwtDll) {
