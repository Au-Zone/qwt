################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

include( $${PWD}/../examples.pri )

QT += xml

INCLUDEPATH += $${QWT_ROOT}/textengines/mathml
DEPENDPATH += $${QWT_ROOT}/textengines/mathml

TARGET   = mathml
SOURCES = main.cpp

qwtAddLibrary($${QWT_OUT_ROOT}/lib, qwtmathml)

