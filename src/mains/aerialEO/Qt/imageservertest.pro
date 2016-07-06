# =========================================================================
# Last updated on 5/5/10; 5/16/10; 5/18/10
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = imageservertest.cc \
	  $$WEBDIR/ImageServer.cc \
	  $$WEBDIR/ImageClient.cc 

HEADERS = $$WEBDIR/ImageServer.h \
	  $$WEBDIR/ImageClient.h 

TARGET = imageservertest

TEMPLATE_TYPE = app

QT += network thread xml 
QT -= gui

CONFIG += qt 
