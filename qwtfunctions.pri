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

    INCLUDEPATH -= $$QMAKE_INCDIR_QT/$$1
    INCLUDEPATH = $$QMAKE_INCDIR_QT/$$1 $$INCLUDEPATH

    LIB_NAME = $$1
    unset(LINKAGE)
    mac {
       CONFIG(qt_framework, qt_framework|qt_no_framework) { #forced
          QMAKE_FRAMEWORKPATH *= $${QMAKE_LIBDIR_QT}
          FRAMEWORK_INCLUDE = $$QMAKE_LIBDIR_QT/$${LIB_NAME}.framework/Headers
          !qt_no_framework_direct_includes:exists($$FRAMEWORK_INCLUDE) {
             INCLUDEPATH -= $$FRAMEWORK_INCLUDE
             INCLUDEPATH = $$FRAMEWORK_INCLUDE $$INCLUDEPATH
           }
           LINKAGE = -framework $${LIB_NAME}$${QT_LIBINFIX}
        } else:!qt_no_framework { #detection
           for(frmwrk_dir, $$list($$QMAKE_LIBDIR_QT $$QMAKE_LIBDIR $$(DYLD_FRAMEWORK_PATH) /Library/Frameworks)) {
              exists($${frmwrk_dir}/$${LIB_NAME}.framework) {
                QMAKE_FRAMEWORKPATH *= $${frmwrk_dir}
                FRAMEWORK_INCLUDE = $$frmwrk_dir/$${LIB_NAME}.framework/Headers
                !qt_no_framework_direct_includes:exists($$FRAMEWORK_INCLUDE) {
                  INCLUDEPATH -= $$FRAMEWORK_INCLUDE
                  INCLUDEPATH = $$FRAMEWORK_INCLUDE $$INCLUDEPATH
                }
                LINKAGE = -framework $${LIB_NAME}
                break()
              }
           }
       }
    }
    symbian {
        export(TARGET.EPOCHEAPSIZE)
        export(TARGET.CAPABILITY)
    }
    isEmpty(LINKAGE) {
       if(!debug_and_release|build_pass):CONFIG(debug, debug|release) {
           win32:LINKAGE = -l$${LIB_NAME}$${QT_LIBINFIX}d
           mac:LINKAGE = -l$${LIB_NAME}$${QT_LIBINFIX}_debug
       }
       isEmpty(LINKAGE):LINKAGE = -l$${LIB_NAME}$${QT_LIBINFIX}
    }
    !isEmpty(QMAKE_LSB) {
        QMAKE_LFLAGS *= --lsb-libpath=$$$$QMAKE_LIBDIR_QT
        QMAKE_LFLAGS *= -L/opt/lsb/lib
        QMAKE_LFLAGS *= --lsb-shared-libs=$${LIB_NAME}$${QT_LIBINFIX}
    }
    LIBS += $$LINKAGE
    export(LIBS)
    export(INCLUDEPATH)
    export(QMAKE_FRAMEWORKPATH)
    export(QMAKE_LFLAGS)
    return(true)
}
