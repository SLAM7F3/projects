# =========================================================================thr
# Last updated on 9/12/10
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = qtanotserv.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/AnnotationServer.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc \
	  $$WEBDIR/BasicServer.cc 

HEADERS = $$WEBDIR/DOMParser.h \
	  $$WEBDIR/AnnotationServer.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h \
	  $$WEBDIR/BasicServer.h 

TARGET = qtanotserv

TEMPLATE_TYPE = app

QT += network xml
QT += gui

CONFIG += qt 
