# =========================================================================
# Last updated on 7/8/10; 7/9/10; 7/25/10
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = qtviewladar.cc \
	  $$WEBDIR/VideoServer.cc \
	  $$WEBDIR/LadarServer.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc \
	  $$WEBDIR/MovieServer.cc \
	  $$WEBDIR/BasicServer.cc 

HEADERS = $$WEBDIR/VideoServer.h \
	  $$WEBDIR/DOMParser.h \
	  $$WEBDIR/LadarServer.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h \
	  $$WEBDIR/MovieServer.h \
	  $$WEBDIR/BasicServer.h 

TARGET = qtviewladar

TEMPLATE_TYPE = app

QT += network thread xml
QT += gui

CONFIG += qt 
