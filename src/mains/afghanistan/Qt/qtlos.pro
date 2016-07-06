# =========================================================================
# Last updated on 5/18/10; 5/19/10; 4/9/12
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = qtlos.cc \
	  $$WEBDIR/VideoServer.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/LOSServer.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc \
	  $$WEBDIR/BasicServer.cc 

HEADERS = $$WEBDIR/VideoServer.h \
	  $$WEBDIR/DOMParser.h \
	  $$WEBDIR/LOSServer.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h \
	  $$WEBDIR/BasicServer.h 

TARGET = qtlos

TEMPLATE_TYPE = app

QT += network thread xml
QT += gui

CONFIG += qt 

