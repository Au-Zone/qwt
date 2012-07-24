################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

# Copied and modified from qtAddLibrary: Qt5 redefined the meaning
# of qtAddLibrary to be usable for modules only

defineTest(qwtAddLibrary) {

    LIB_NAME = $$1

    unset(LINKAGE)

    if(!debug_and_release|build_pass):CONFIG(debug, debug|release) {
	   win32:LINKAGE = -l$${LIB_NAME}$${QT_LIBINFIX}d
	   mac:LINKAGE = -l$${LIB_NAME}$${QT_LIBINFIX}_debug
    }

   	isEmpty(LINKAGE):LINKAGE = -l$${LIB_NAME}$${QT_LIBINFIX}

	!isEmpty(QMAKE_LSB) {
        QMAKE_LFLAGS *= --lsb-shared-libs=$${LIB_NAME}$${QT_LIBINFIX}
    }

    LIBS += $$LINKAGE
    export(LIBS)
    export(QMAKE_LFLAGS)

    return(true)
}
