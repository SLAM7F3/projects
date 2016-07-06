CONFIG += debug

unix {
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  UI_DIR = .ui
}
else {
  MOC_DIR = _moc
  OBJECTS_DIR = _obj
  UI_DIR = _ui
}

win32-msvc:QMAKE_CXXFLAGS += /GR
win32-borland:QMAKE_CXXFLAGS += -RT
win32:DEFINES += NOMINMAX _USE_MATH_DEFINES
