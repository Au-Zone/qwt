################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

QWT_ROOT = ../..
include( $${QWT_ROOT}/qwtconfig.pri )

TEMPLATE  = lib

DESTDIR         = $${QWT_ROOT}/lib
INCLUDEPATH    += $${QWT_ROOT}/src
DEPENDPATH     += $${QWT_ROOT}/src

QwtDll {
    CONFIG += dll
    win32|symbian: DEFINES += QT_DLL QWT_DLL QWT_MAKEDLL
}   
else {
    CONFIG += staticlib
}

LIBS      += -L$${QWT_ROOT}/lib
qtAddLibrary(qwt)

target.path    = $${QWT_INSTALL_LIBS}
headers.path   = $${QWT_INSTALL_HEADERS}
doc.path       = $${QWT_INSTALL_DOCS}

headers.files  = $$HEADERS
INSTALLS       = target headers
