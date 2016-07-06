# =========================================================================
# Last updated on 5/19/10; 1/14/12; 7/10/12
# =========================================================================

PREFIX = $(HOME)/programs/c++/svn/projects
CONFIGDIR = $$PREFIX/config
WEBDIR = $$PREFIX/src/Qt/web/
include ($$CONFIGDIR/common.pro)

SOURCES = qtLOSTserver.cc \
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

TARGET = qtLOSTserver

TEMPLATE_TYPE = app

QT += network thread xml
QT += gui

CONFIG += qt 

# On 7/10/12, Dave Ceddia taught us that we cannot replace the "QT += gui"
# line with "QT -= gui" even though the LOSTserver is supposed to run with
# no GUI front-end.  Recall the BasicServer class has a QWidget* window_ptr
# member.  QWidgets are basically the foundation of all QT GUI objects.
# So we would have to eliminate the dependence of the LOSTserver upon the
# BasicServer class in order to eliminate the dependence of this .pro file
# upon gui.  

# As of July 2012, we don't want to have to redo the entire LOSServer class
# just to make this .pro cleaner.  So we're willing to live with this ugly
# hack for now...
