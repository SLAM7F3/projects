# =========================================================================
# Last updated on 9/8/09; 5/5/10; 5/19/10
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = cars.cc \
	  $$WEBDIR/BluegrassClient.cc \
	  $$WEBDIR/VideoServer.cc \
	  $$WEBDIR/SKSClient.cc \
	  $$WEBDIR/SKSDataServerInterfacer.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/ImageServer.cc \
	  $$WEBDIR/ImageClient.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc 

HEADERS = $$WEBDIR/VideoServer.h \
	  $$WEBDIR/BluegrassClient.h \
	  $$WEBDIR/SKSClient.h \
	  $$WEBDIR/SKSDataServerInterfacer.h \
	  $$WEBDIR/DOMParser.h \
	  $$WEBDIR/ImageServer.h \
	  $$WEBDIR/ImageClient.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h 

TARGET = cars

TEMPLATE_TYPE = app

QT += network thread xml
QT -= gui

CONFIG += qt 
