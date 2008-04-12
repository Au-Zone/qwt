# -*- mode: sh -*- ###########################
# QwtPolar Widget Library
# Copyright (C) 2008   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GPL License, Version 2.0
##############################################

include( ../../../qwtconfig.pri )

TEMPLATE     = app

MOC_DIR      = moc
OBJECTS_DIR  = obj
INCLUDEPATH += ../../src
DEPENDPATH  += ../../src
INCLUDEPATH += ../../../../src
DEPENDPATH  += ../../../../src


unix:LIBS  += -L../../lib -lqwtpolar
unix:LIBS  += -L../../../../lib -lqwt 

win32:QwtDll {
    DEFINES    += QT_DLL QWT_DLL
}

contains(CONFIG, QwtDll) {
    win32-msvc:LIBS  += ../../../lib/qwt5.lib
    win32-msvc:LIBS  += ../../lib/qwtpolar0.lib
    win32-msvc.net:LIBS  += ../../../lib/qwt5.lib
    win32-msvc.net:LIBS  += ../../lib/qwtpolar0.lib
    win32-msvc2005:LIBS += ../../../lib/qwt5.lib
    win32-msvc2005:LIBS += ../../lib/qwtpolar0.lib
    win32-g++:LIBS   += -L../../../lib -lqwt
    win32-g++:LIBS   += -L../../lib -lqwtpolar
} else {
    win32-msvc:LIBS  += ../../../lib/qwt.lib
    win32-msvc:LIBS  += ../../lib/qwtpolar.lib
    win32-msvc.net:LIBS  += ../../../lib/qwt.lib
    win32-msvc.net:LIBS  += ../../lib/qwtpolar.lib
    win32-msvc2005:LIBS += ../../../lib/qwt.lib
    win32-msvc2005:LIBS += ../../lib/qwtpolar.lib
    win32-g++:LIBS   += -L../../../lib -lqwt
    win32-g++:LIBS   += -L../../lib -lqwtpolar
}

