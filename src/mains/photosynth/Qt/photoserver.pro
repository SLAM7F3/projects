# =========================================================================thr
# Last updated on 9/13/10; 1/14/11; 8/2/11; 8/19/16
# =========================================================================

PREFIX = $(HOME)/programs/c++/git/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = qtphotoserver.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/MessageServer.cc \
	  $$WEBDIR/PhotoServer.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc \
	  $$WEBDIR/BasicServer.cc 

HEADERS = $$WEBDIR/DOMParser.h \
	  $$WEBDIR/MessageServer.h \
	  $$WEBDIR/PhotoServer.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h \
	  $$WEBDIR/BasicServer.h 

TARGET = qtphotoserver

TEMPLATE_TYPE = app

QT += network xml
# QT += gui

CONFIG += qt 
