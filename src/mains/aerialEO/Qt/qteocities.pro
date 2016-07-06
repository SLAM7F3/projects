# =========================================================================
# Last updated on 12/21/09; 5/5/10; 5/19/10
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = qteocities.cc \
	  $$WEBDIR/VideoServer.cc \
	  $$WEBDIR/RTPSButtonServer.cc \
	  $$WEBDIR/SKSClient.cc \
	  $$WEBDIR/SKSDataServerInterfacer.cc \
	  $$WEBDIR/DOMParser.cc \
	  $$WEBDIR/ImageClient.cc \
	  $$WEBDIR/ImageServer.cc \
	  $$WEBDIR/WebServer.cc \
	  $$WEBDIR/WebClient.cc 

HEADERS = $$WEBDIR/VideoServer.h \
	  $$WEBDIR/RTPSButtonServer.h \
	  $$WEBDIR/SKSClient.h \
	  $$WEBDIR/SKSDataServerInterfacer.h \
	  $$WEBDIR/DOMParser.h \
	  $$WEBDIR/ImageClient.h \
	  $$WEBDIR/ImageServer.h \
	  $$WEBDIR/WebServer.h \
	  $$WEBDIR/WebClient.h 

TARGET = qteocities

TEMPLATE_TYPE = app


QT += network thread xml
QT -= gui

CONFIG += qt 
