# =========================================================================
# Last updated on 5/5/10; 5/16/10; 5/19/10
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = imageclienttest.cc \
	  $$WEBDIR/ImageServer.cc \
	  $$WEBDIR/ImageClient.cc 

HEADERS = $$WEBDIR/ImageServer.h \
	  $$WEBDIR/ImageClient.h 

TARGET = imageclienttest

TEMPLATE_TYPE = app

QT += network thread xml
QT -= gui

CONFIG += qt 
