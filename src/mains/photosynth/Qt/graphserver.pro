# =========================================================================
# Last updated on 3/17/12; 8/19/16
# =========================================================================

PREFIX = $(HOME)/programs/c++/git/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = qtgraphserver.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/MessageServer.cc \
	  $$WEBDIR/GraphServer.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc \
	  $$WEBDIR/BasicServer.cc 

HEADERS = $$WEBDIR/DOMParser.h \
	  $$WEBDIR/MessageServer.h \
	  $$WEBDIR/GraphServer.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h \
	  $$WEBDIR/BasicServer.h 

TARGET = qtgraphserver

TEMPLATE_TYPE = app

QT += network xml
# QT += gui

CONFIG += qt 
