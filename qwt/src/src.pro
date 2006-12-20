# -*- mode: sh -*- ###########################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
##############################################

# pro file for building the qwt libraries
#
# Please read the instructions in INSTALL, if you don´t know
# how to use it.
#
# HEADERS/SOURCES are separated into plot
# relevant files and others. If you are 
# interested in QwtPlot only, you can remove
# the lists at the end of this file.

TARGET            = qwt
TEMPLATE          = lib

VERSION      = 5.0.0

# Only one of the following flags is allowed !
# For Qtopia Core you need qt!

CONFIG           += qt
#CONFIG           += qtopia

CONFIG           += warn_on 
CONFIG           += release
CONFIG           += thread

# Only one of the following flags is allowed !
CONFIG           += dll 
# CONFIG           += staticlib

# DEFINES          += QT_NO_CAST_ASCII
# DEFINES          += QT_NO_CAST_TO_ASCII

MOC_DIR           = moc
OBJECTS_DIR       = obj
DESTDIR           = ../lib

win32:dll:DEFINES    += QT_DLL QWT_DLL QWT_MAKEDLL

# debug:QMAKE_CXXFLAGS += -O0

HEADERS = \
    qwt.h \
    qwt_abstract_scale_draw.h \
    qwt_array.h \
    qwt_color_map.h \
    qwt_curve_fitter.h \
    qwt_data.h \
    qwt_double_interval.h \
    qwt_double_rect.h \
    qwt_dyngrid_layout.h \
    qwt_event_pattern.h \
    qwt_global.h \
    qwt_interval_data.h \
    qwt_layout_metrics.h \
    qwt_legend.h \
    qwt_legend_item.h \
    qwt_math.h \
    qwt_painter.h \
    qwt_paint_buffer.h \
    qwt_panner.h \
    qwt_picker.h \
    qwt_picker_machine.h \
    qwt_plot.h \
    qwt_plot_curve.h \
    qwt_plot_dict.h \
    qwt_plot_grid.h \
    qwt_plot_item.h \
    qwt_plot_layout.h \
    qwt_plot_marker.h \
    qwt_plot_printfilter.h \
    qwt_plot_rasteritem.h \
    qwt_plot_spectrogram.h \
    qwt_plot_canvas.h \
    qwt_plot_panner.h \
    qwt_plot_picker.h \
    qwt_plot_zoomer.h \
    qwt_plot_magnifier.h \
    qwt_polygon.h \
    qwt_raster_data.h \
    qwt_rect.h \
    qwt_round_scale_draw.h \
    qwt_scale_widget.h \
    qwt_scale_div.h \
    qwt_scale_draw.h \
    qwt_scale_engine.h \
    qwt_scale_map.h \
    qwt_spline.h \
    qwt_symbol.h \
    qwt_text_engine.h \
    qwt_text_label.h \
    qwt_text.h \
    qwt_valuelist.h

SOURCES = \
    qwt_abstract_scale_draw.cpp \
    qwt_data.cpp \
    qwt_color_map.cpp \
    qwt_curve_fitter.cpp \
    qwt_double_interval.cpp \
    qwt_double_rect.cpp \
    qwt_dyngrid_layout.cpp \
    qwt_event_pattern.cpp \
    qwt_interval_data.cpp \
    qwt_layout_metrics.cpp \
    qwt_legend.cpp \
    qwt_legend_item.cpp \
    qwt_math.cpp \
    qwt_painter.cpp \
    qwt_paint_buffer.cpp \
    qwt_panner.cpp \
    qwt_picker.cpp \
    qwt_picker_machine.cpp \
    qwt_plot.cpp \
    qwt_plot_print.cpp \
    qwt_plot_xml.cpp \
    qwt_plot_axis.cpp \
    qwt_plot_curve.cpp \
    qwt_plot_dict.cpp \
    qwt_plot_grid.cpp \
    qwt_plot_item.cpp \
    qwt_plot_spectrogram.cpp \
    qwt_plot_marker.cpp \
    qwt_plot_layout.cpp \
    qwt_plot_printfilter.cpp \
    qwt_plot_canvas.cpp \
    qwt_plot_panner.cpp \
    qwt_plot_rasteritem.cpp \
    qwt_plot_picker.cpp \
    qwt_plot_zoomer.cpp \
    qwt_plot_magnifier.cpp \
    qwt_raster_data.cpp \
    qwt_rect.cpp \
    qwt_round_scale_draw.cpp \
    qwt_scale_widget.cpp \
    qwt_scale_div.cpp \
    qwt_scale_draw.cpp \
    qwt_scale_engine.cpp \
    qwt_scale_map.cpp \
    qwt_spline.cpp \
    qwt_symbol.cpp \
    qwt_text_engine.cpp \
    qwt_text_label.cpp \
    qwt_text.cpp 
 
##############################################
# If you don't need to display svg images
# on the plot canvas, remove the following 
# lines
##############################################

# QT += svg
# HEADERS += qwt_plot_svgitem.h
# SOURCES += qwt_plot_svgitem.cpp 

##############################################
# If you are interested in the plot widget
# only, you can remove the following 
# HEADERS/SOURCES lists.
##############################################

HEADERS += \
    qwt_abstract_slider.h \
    qwt_abstract_scale.h \
    qwt_arrow_button.h \
    qwt_analog_clock.h \
    qwt_compass.h \
    qwt_compass_rose.h \
    qwt_counter.h \
    qwt_dial.h \
    qwt_dial_needle.h \
    qwt_double_range.h \
    qwt_knob.h \
    qwt_slider.h \
    qwt_thermo.h \
    qwt_wheel.h

SOURCES += \
    qwt_abstract_slider.cpp \
    qwt_abstract_scale.cpp \
    qwt_arrow_button.cpp \
    qwt_analog_clock.cpp \
    qwt_compass.cpp \
    qwt_compass_rose.cpp \
    qwt_counter.cpp \
    qwt_dial.cpp \
    qwt_dial_needle.cpp \
    qwt_double_range.cpp \
    qwt_knob.cpp \
    qwt_slider.cpp \
    qwt_thermo.cpp \
    qwt_wheel.cpp

unix {
    INSTALLBASE    = /usr/local/qwt
    target.path    = $$INSTALLBASE/lib
    headers.path   = $$INSTALLBASE/include
    headers.files  = $$HEADERS
    doc.path       = $$INSTALLBASE/doc
    doc.files      = ../doc/html ../doc/man
    INSTALLS       = target headers doc
}
