# =========================================================================
# Last updated on 4/4/12; 4/6/12
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = qtmacheteserver.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/LOSTClient.cc \
	  $$WEBDIR/MessageServer.cc \
	  $$WEBDIR/MacheteServer.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc \
	  $$WEBDIR/BasicServer.cc 

HEADERS = $$WEBDIR/DOMParser.h \
	  $$WEBDIR/LOSTClient.h \
	  $$WEBDIR/MessageServer.h \
	  $$WEBDIR/MacheteServer.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h \
	  $$WEBDIR/BasicServer.h 

TARGET = qtmacheteserver

TEMPLATE_TYPE = app

QT += network xml
# QT += gui

CONFIG += qt 
