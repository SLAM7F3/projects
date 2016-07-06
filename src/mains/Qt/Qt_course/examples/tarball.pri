include(subdirs.pro)
# this file creates examples/examples.tar with the subdirs that match
# slides/setup.tex's config

unix { # works only for unix

# Import setup.tex config into CONFIG:
SETUP_TEX=$$PWD/../slides/setup.tex
include( ../../common/setup-scan.pri )

# disable subdirs that don't match config:
!part-qprocess:SUBDIRS -= findTool
!part-qcanvas:SUBDIRS -= kdab
!part-xml:SUBDIRS -= qdom qdom-write
!part-sql:SUBDIRS -= sql
!part-network-programming:SUBDIRS -= sockets udpClient udpServer
!part-internationalization:SUBDIRS -= codec
!part-activeqt:SUBDIRS -= activeqtwebbrowser activeqt
!part-multithreading:SUBDIRS -= fibonacci
!part-model-view:SUBDIRS -= model-view
!part-opengl:SUBDIRS -= opengl
!part-qsa:SUBDIRS -= qsa
!part-qscrollview:SUBDIRS -= qscrollarea
!part-qtestlib:SUBDIRS -= unit-test
SUBDIRS -= custom-signals qt-embedded qtopia # currently not used.



# and create the examples.tar:
system( ../../common/tar_add.sh -C .. -p examples -o examples.tar $$SUBDIRS examples.pri)
else:error( Could not create examples/examples.tar )

}
