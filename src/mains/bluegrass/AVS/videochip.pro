# =========================================================================
# Last updated on 5/16/10; 5/18/10; 5/19/10
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = videochip.cc \
	  $$WEBDIR/VideoServer.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc 

HEADERS = $$WEBDIR/VideoServer.h \
	  $$WEBDIR/DOMParser.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h 

TARGET = videochip

TEMPLATE_TYPE = app


QT += network thread xml
QT -= gui

CONFIG += qt 
