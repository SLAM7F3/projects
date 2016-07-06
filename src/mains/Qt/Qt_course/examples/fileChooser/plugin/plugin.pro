SOURCES  += plugin.cpp ../widget/filechooser.cpp
HEADERS  += plugin.h ../widget/filechooser.h
DESTDIR   = $(QTDIR)/plugins/designer
TARGET    = filechooser

TEMPLATE     = lib
CONFIG      += qt warn_on release plugin
unix:MOC_DIR = .moc
unix:OBJECTS_DIR = .obj
