# =========================================================================
# Last updated on 5/16/10; 5/18/10; 5/19/10
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = qtcities.cc \
	  $$WEBDIR/BluegrassServer.cc \
	  $$WEBDIR/BluegrassClient.cc \
	  $$WEBDIR/VideoServer.cc \
	  $$WEBDIR/OSGButtonServer.cc \
	  $$WEBDIR/SKSClient.cc \
	  $$WEBDIR/SKSDataServerInterfacer.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc 

HEADERS = $$WEBDIR/BluegrassServer.h \
	  $$WEBDIR/BluegrassClient.h \
	  $$WEBDIR/VideoServer.h \
	  $$WEBDIR/OSGButtonServer.h \
	  $$WEBDIR/SKSClient.h \
	  $$WEBDIR/SKSDataServerInterfacer.h \
	  $$WEBDIR/DOMParser.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h 

TARGET = qtcities

TEMPLATE_TYPE = app

QT += network thread xml
QT -= gui

CONFIG += qt 
