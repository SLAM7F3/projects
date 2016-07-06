# =========================================================================
# Last updated on 7/10/10
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = qtviewmovie.cc \
	  $$WEBDIR/VideoServer.cc \
	  $$WEBDIR/MovieServer.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc \
	  $$WEBDIR/BasicServer.cc 

HEADERS = $$WEBDIR/VideoServer.h \
	  $$WEBDIR/DOMParser.h \
	  $$WEBDIR/MovieServer.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h \
	  $$WEBDIR/BasicServer.h 

TARGET = qtviewmovie

TEMPLATE_TYPE = app

QT += network thread xml
QT += gui

CONFIG += qt 
