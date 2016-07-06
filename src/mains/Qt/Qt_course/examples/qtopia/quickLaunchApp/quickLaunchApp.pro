qtopia_project(qtopia app)
CONFIG+=qtopia_main
CONFIG-=no_quicklaunch
TARGET = quickLaunchApp
SOURCES = main.cpp 

unix:MOC_DIR = .moc
unix:OBJECTS_DIR = .obj

desktop.files = $$PWD/quickLaunchApp.desktop
desktop.path=/apps/Applications
INSTALLS += desktop

pics.files=$$PWD/quickLaunchApp.png
pics.path=/pics/quickLaunchApp
PICS_INSTALLS+=pics
