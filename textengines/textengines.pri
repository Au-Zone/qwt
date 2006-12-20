# -*- mode: sh -*- ###########################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
##############################################

TEMPLATE  = lib
CONFIG    += qt lib
CONFIG    += warn_on
CONFIG    += release
CONFIG    += thread

MOC_DIR         = moc
OBJECTS_DIR     = obj
DESTDIR         = ../../lib
INCLUDEPATH    += ../../src
DEPENDPATH     += ../../src

win32::DEFINES += QT_DLL QWT_DLL QWT_MAKEDLL

unix:LIBS      += -L../../lib -lqwt
win32-msvc:LIBS  += ../../lib/qwt5.lib
win32-msvc.net:LIBS  += ../../lib/qwt5.lib
win32-msvc2005:LIBS += ../../lib/qwt5.lib
win32-g++:LIBS   += -L../../lib -lqwt5

unix {
    INSTALLBASE    = /usr/local/qwt
    target.path    = $$INSTALLBASE/lib
    headers.path   = $$INSTALLBASE/include
    headers.files  = $$HEADERS
    INSTALLS       = target headers 
}
