################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

!exists( qtmmlwidget.cpp ) {
	error( "qtmmlwidget.cpp is missing, see http://qt.nokia.com/products/appdev/add-on-products/catalog/4/Widgets/qtmmlwidget" )
}

include( ../textengines.pri )

TARGET    = $$qtLibraryTarget(qwtmathml)
VERSION   = 1.0.0
QT       += xml

HEADERS = \
	qwt_mathml_text_engine.h

SOURCES = \
	qwt_mathml_text_engine.cpp

# The files below can be found in the MathML addon of the Qt Solution 
# package http://qt.nokia.com/products/appdev/add-on-products/catalog/4/Widgets/qtmmlwidget/
# Copy them here, or modify the pro file for your installation.

HEADERS += qtmmlwidget.h
SOURCES += qtmmlwidget.cpp

