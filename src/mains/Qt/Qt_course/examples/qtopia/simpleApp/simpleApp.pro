qtopia_project(qtopia app)
TARGET=simpleApp
SOURCES = main.cpp 
CONFIG += qtopia_main
CONFIG += no_quicklaunch

unix:MOC_DIR = .moc
unix:OBJECTS_DIR = .obj
