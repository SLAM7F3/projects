TEMPLATE= lib
TARGET = teststyle
CONFIG += plugin
SOURCES = teststyle.cpp teststyleplugin.cpp
HEADERS = teststyle.h
unix:MOC_DIR = .moc
unix:OBJECTS_DIR = .obj

INSTALLS += target
target.path = $$[QT_INSTALL_PREFIX]/plugins/styles
unix:target.files = libteststyle.so
