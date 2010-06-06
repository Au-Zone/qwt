################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

include( ../qwtconfig.pri )

TEMPLATE = subdirs

PRFFILE = qwt.prf

EOC = $$escape_expand(\n\t)

GENSPECS =
GENSPECS += @echo "INCLUDEPATH *= $${QWT_INSTALL_HEADERS}" > $$PRFFILE 
GENSPECS += $$EOC@echo "LIBS *= -L$${QWT_INSTALL_LIBS}" >> $$PRFFILE 
GENSPECS += $$EOC@echo \"qtAddLibrary(qwt)\" >> $$PRFFILE 

QwtDll {
	GENSPECS += $$EOC@echo "DEFINES *= QWT_DLL" >> $$PRFFILE 
}

qwtspecs.target = $$PRFFILE
qwtspecs.files = $$qwtspecs.target
qwtspecs.commands = $$GENSPECS

QMAKE_EXTRA_TARGETS += qwtspecs
QMAKE_CLEAN += $$qwtspecs.target

qwtspecs.path  = $${QWT_INSTALL_FEATURES}
INSTALLS += qwtspecs

