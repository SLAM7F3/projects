# =========================================================================thr
# Last updated on 9/13/10
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = qttocserv.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/TOCServer.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc \
	  $$WEBDIR/BasicServer.cc 

HEADERS = $$WEBDIR/DOMParser.h \
	  $$WEBDIR/TOCServer.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h \
	  $$WEBDIR/BasicServer.h 

TARGET = qttocserv

TEMPLATE_TYPE = app

QT += network xml
QT += gui

CONFIG += qt 
