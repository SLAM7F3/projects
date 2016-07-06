# =========================================================================
# Last updated on 5/1/11
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = qttoc11.cc \
	  $$WEBDIR/BluegrassServer.cc \
	  $$WEBDIR/BluegrassClient.cc \
	  $$WEBDIR/VideoServer.cc \
	  $$WEBDIR/OSGButtonServer.cc \
	  $$WEBDIR/SKSClient.cc \
	  $$WEBDIR/SKSDataServerInterfacer.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/LOSServer.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc \
	  $$WEBDIR/BasicServer.cc 

HEADERS = $$WEBDIR/BluegrassServer.h \
	  $$WEBDIR/BluegrassClient.h \
	  $$WEBDIR/VideoServer.h \
	  $$WEBDIR/OSGButtonServer.h \
	  $$WEBDIR/SKSClient.h \
	  $$WEBDIR/SKSDataServerInterfacer.h \
	  $$WEBDIR/DOMParser.h \
	  $$WEBDIR/LOSServer.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h \
	  $$WEBDIR/BasicServer.h 

TARGET = qttoc11

TEMPLATE_TYPE = app

QT += network thread xml
QT -= gui

CONFIG += qt 
