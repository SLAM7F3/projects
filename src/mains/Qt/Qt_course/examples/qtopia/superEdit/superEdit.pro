qtopia_project(qtopia app)
TARGET=superEdit
CONFIG+=qtopia_main
CONFIG+=no_quicklaunch
DEFINES += QTOPIA_PHONE

HEADERS = edit.h
SOURCES = main.cpp edit.cpp

desktop.files=$$PWD/superEdit.desktop
desktop.path=/apps/Applications
INSTALLS+=desktop

help.files=$$PWD/superEdit.html
help.path=/help/html
INSTALLS+=help

pics.files=$$PWD/superEdit.png
pics.path=/pics/superEdit
PICS_INSTALLS+=pics

