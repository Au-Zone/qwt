# -*- mode: sh -*- ###########################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
##############################################

QWT_ROOT = ..

include ( $${QWT_ROOT}/qwtconfig.pri )

contains(CONFIG, QwtDesigner) {

	CONFIG    += warn_on

	SUFFIX_STR =

	CONFIG(debug, debug|release) {
		SUFFIX_STR = $${DEBUG_SUFFIX}
	}
	else {
		SUFFIX_STR = $${RELEASE_SUFFIX}
	}

	TEMPLATE        = lib
	MOC_DIR         = moc
	OBJECTS_DIR     = obj$${SUFFIX_STR}
	DESTDIR         = plugins/designer
	INCLUDEPATH    += $${QWT_ROOT}/src 
	DEPENDPATH     += $${QWT_ROOT}/src 

    LIBNAME         = qwt$${SUFFIX_STR}
	contains(CONFIG, QwtDll) {
		win32 {
			DEFINES += QT_DLL QWT_DLL
			LIBNAME = $${LIBNAME}$${VER_MAJ}
		}
	}

	!contains(CONFIG, QwtPlot) {
		DEFINES += NO_QWT_PLOT
	}

	!contains(CONFIG, QwtWidgets) {
		DEFINES += NO_QWT_WIDGETS
	}

	unix:LIBS      += -L$${QWT_ROOT}/lib -l$${LIBNAME}
	win32-msvc:LIBS  += $${QWT_ROOT}/lib/$${LIBNAME}.lib
	win32-msvc.net:LIBS  += $${QWT_ROOT}/lib/$${LIBNAME}.lib
	win32-msvc2002:LIBS += $${QWT_ROOT}/lib/$${LIBNAME}.lib
	win32-msvc2003:LIBS += $${QWT_ROOT}/lib/$${LIBNAME}.lib
	win32-msvc2005:LIBS += $${QWT_ROOT}/lib/$${LIBNAME}.lib
	win32-msvc2008:LIBS += $${QWT_ROOT}/lib/$${LIBNAME}.lib
	win32-g++:LIBS   += -L$${QWT_ROOT}/lib -l$${LIBNAME}

	TARGET    = qwt_designer_plugin$${SUFFIX_STR}
	CONFIG    += qt designer plugin 

	RCC_DIR   = resources

	HEADERS += \
		qwt_designer_plugin.h

	SOURCES += \
		qwt_designer_plugin.cpp

	contains(CONFIG, QwtPlot) {

		HEADERS += \
			qwt_designer_plotdialog.h

		SOURCES += \
			qwt_designer_plotdialog.cpp
	}

	RESOURCES += \
		qwt_designer_plugin.qrc

	target.path = $$[QT_INSTALL_PLUGINS]/designer
	INSTALLS += target
}
else {
	TEMPLATE        = subdirs # do nothing
}
