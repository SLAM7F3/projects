ACTIVEQTDIR=$(QTDIR)/extensions/activeqt

TEMPLATE	= app
CONFIG		+= qt warn_on release qaxserver
DEF_FILE    = $$ACTIVEQTDIR/control/qaxserver.def
RC_FILE     = $$ACTIVEQTDIR/control/qaxserver.rc
HEADERS		+= hello.h
SOURCES		+= hello.cpp \
		       main.cpp
TARGET		= helloserver


#DEPENDPATH+=$$ACTIVEQTDIR/control
#INCLUDEPATH+=$$ACTIVEQTDIR/control

