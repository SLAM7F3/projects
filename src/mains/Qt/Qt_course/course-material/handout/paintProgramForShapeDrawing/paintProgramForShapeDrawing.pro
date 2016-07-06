TEMPLATE = app
HEADERS = scribbleArea.h paintwindow.h configDialog.h
SOURCES  = scribbleArea.cpp main.cpp paintwindow.cpp configDialog.cpp
unix:MOC_DIR = .moc
unix:OBJECTS_DIR = .obj
include( "../handout.pri" )
