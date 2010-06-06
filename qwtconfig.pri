################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

VER_MAJ      = 6
VER_MIN      = 0
VER_PAT      = 0
VERSION      = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}

######################################################################
# Install paths
######################################################################

QWT_INSTALL_PREFIX = $$[QT_INSTALL_PREFIX]

unix {
    QWT_INSTALL_PREFIX    = /usr/local/qwt-$$VERSION-svn
}

win32 {
    QWT_INSTALL_PREFIX    = C:/Qwt-$$VERSION-svn
}

QWT_INSTALL_DOCS      = $${QWT_INSTALL_PREFIX}/doc
QWT_INSTALL_HEADERS   = $${QWT_INSTALL_PREFIX}/include
QWT_INSTALL_LIBS      = $${QWT_INSTALL_PREFIX}/lib
QWT_INSTALL_PLUGINS   = $${QWT_INSTALL_PREFIX}/plugins/designer
QWT_INSTALL_FEATURES  = $${QWT_INSTALL_PREFIX}/features

######################################################################
# qmake internal options
######################################################################

CONFIG           += qt     
CONFIG           += warn_on
CONFIG           += thread
CONFIG           += no_keywords
CONFIG           += silent

######################################################################
# release/debug mode
######################################################################

win32 {
	# On Windows you can't mix release and debug libraries.
	# The designer is built in release mode. If you like to use it
	# you need a release version. For your own application development you
	# might need a debug version. 
	# Enable debug_and_release + build_all if you want to build both.

	CONFIG           += debug_and_release
	CONFIG           += build_all
}
else {

	CONFIG           += debug
}

linux-g++ {
	# CONFIG           += separate_debug_info
}

######################################################################
# paths for building qwt
######################################################################

MOC_DIR      = moc
RCC_DIR      = resources
!debug_and_release {
	OBJECTS_DIR       = obj
}

######################################################################
# Build the static/shared libraries.
# If QwtDll is enabled, a shared library is built, otherwise
# it will be a static library.
######################################################################

CONFIG           += QwtDll

######################################################################
# QwtPlot enables all classes, that are needed to use the QwtPlot 
# widget. 
######################################################################

CONFIG       += QwtPlot

######################################################################
# QwtWidgets enables all classes, that are needed to use the all other
# widgets (sliders, dials, ...), beside QwtPlot. 
######################################################################

CONFIG     += QwtWidgets

######################################################################
# If you want to display svg images on the plot canvas, enable the 
# line below. Note that Qwt needs the svg+xml, when enabling 
# QwtSVGItem.
######################################################################

CONFIG     += QwtSVGItem

######################################################################
# You can use the MathML renderer of the Qt solutions package to 
# enable MathML support in Qwt.  # If you want this, copy 
# qtmmlwidget.h + qtmmlwidget.cpp to # textengines/mathml and enable 
# the line below.
######################################################################

#CONFIG     += QwtMathML

######################################################################
# If you want to build the Qwt designer plugin, 
# enable the line below.
# Otherwise you have to build it from the designer directory.
######################################################################

CONFIG     += QwtDesigner

######################################################################
# If you want to auto build the examples, enable the line below
# Otherwise you have to build them from the examples directory.
######################################################################

#CONFIG     += QwtExamples
