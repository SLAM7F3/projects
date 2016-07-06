# =========================================================================
# Last updated on 10/20/11
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = qtVideoOnDemand.cc \
	  $$WEBDIR/VideoOnDemandServer.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/MessageServer.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc \
	  $$WEBDIR/BasicServer.cc 

HEADERS = $$WEBDIR/VideoOnDemandServer.h \
	  $$WEBDIR/DOMParser.h \
	  $$WEBDIR/MessageServer.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h \
	  $$WEBDIR/BasicServer.h 

TARGET = qtVideoOnDemand

TEMPLATE_TYPE = app

QT += network thread xml
QT += gui

CONFIG += qt 
