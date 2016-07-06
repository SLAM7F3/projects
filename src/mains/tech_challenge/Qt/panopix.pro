# =========================================================================
# Last updated on 8/29/10; 9/4/10
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = qtpanopix.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/AnnotationServer.cc \
	  $$WEBDIR/DataloaderServer.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc \
	  $$WEBDIR/BasicServer.cc 

HEADERS = $$WEBDIR/DOMParser.h \
	  $$WEBDIR/AnnotationServer.h \
	  $$WEBDIR/DataloaderServer.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h \
	  $$WEBDIR/BasicServer.h 

TARGET = qtpanopix

TEMPLATE_TYPE = app

QT += network xml
QT += gui

CONFIG += qt 
