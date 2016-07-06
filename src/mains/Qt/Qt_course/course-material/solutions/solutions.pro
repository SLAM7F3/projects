TEMPLATE=subdirs

exists( activeqtscribble ):SUBDIRS += activeqtscribble
exists( colorTester ):SUBDIRS += colorTester
exists( compasswidget ):SUBDIRS += compasswidget
exists( drag-and-drop ):SUBDIRS += drag-and-drop
exists( firstApplication ):SUBDIRS += firstApplication
exists( helpbrowser ):SUBDIRS += helpbrowser
exists( imagePlugin ):SUBDIRS += imagePlugin
exists( kdab ):SUBDIRS += kdab
exists( layoutbuttons ):SUBDIRS += layoutbuttons
exists( object-browser ):SUBDIRS += object-browser
exists( paintProgram ):SUBDIRS += paintProgram
#exists( paintProgram-plugins ):SUBDIRS += paintProgram-plugins
#exists( paintProgram-qtopia ):SUBDIRS += paintProgram-qtopia
#exists( paintProgram-with-qsa ):SUBDIRS += paintProgram-with-qsa
exists( tcp-server ):SUBDIRS += tcp-server
exists( testtool ):SUBDIRS += testtool
exists( texteditor ):SUBDIRS += texteditor
exists( texteditor-advanced ):SUBDIRS += texteditor-advanced
exists( threads ):SUBDIRS += threads
exists( widgetStyle ):SUBDIRS += widgetStyle
exists( sql ):SUBDIRS += sql

win32:SUBDIRS -= sql
!win32|!exists( $$QTDIR/lib/qaxcontainer.lib ):SUBDIRS -= activeqtscribble
