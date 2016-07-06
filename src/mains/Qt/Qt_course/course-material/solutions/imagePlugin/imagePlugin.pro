TEMPLATE = lib

DESTDIR = $(QTDIR)/plugins/imageformats

CONFIG += plugin console

# Input
HEADERS += SimpleImageIOHandler.h SimpleImagePlugin.h
SOURCES += SimpleImageIOHandler.cpp SimpleImagePlugin.cpp
include(../solutions.pri)
