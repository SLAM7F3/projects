SOURCES = dataparser.cc DOMParser.cc SKSDataServerInterfacer.cc

HEADERS = DOMParser.h SKSDataServerInterfacer.h

TARGET = dataparser

TEMPLATE_TYPE = app

PREFIX = /home/cho/programs/c++/svn/projects
INCLUDEDIR=$$PREFIX/include
LIBDIR=$$PREFIX/lib

INCLUDEPATH += $$INCLUDEDIR

INCLUDEPATH += . \
        /usr/local/axiscpp_deploy/include \
        $(OSG_OP_OT_PATH)/include \
        /usr/local/include /usr/local/pgsql/include \
	/usr/include/postgresql \
	$$INCLUDEDIR/eyeglass/ \
   	/opt/intel/ipp41/ia32_itanium/include/ \
   	/usr/local/kakadu/include \
   	/usr/local/include/exiv2 \
   	/usr/include/exiv2 \
	/usr/local/include/activemq-cpp-2.0.1 \
	/usr/include/glib-2.0 /usr/lib/glib-2.0/include \

LIBS = $$LIBDIR/libspace.a \
	 $$LIBDIR/librobots.a \
	 $$LIBDIR/libosgOrganization.a \
	 $$LIBDIR/libosgFusion.a \
	 $$LIBDIR/libosgFeatures.a \
	 $$LIBDIR/libosgModels.a \
	 $$LIBDIR/libosgAnnotators.a \
	 $$LIBDIR/libosgEarth.a \
	 $$LIBDIR/libosgSpace.a \
	 $$LIBDIR/libosg3D.a \
	 $$LIBDIR/libosg2D.a \
	 $$LIBDIR/libosgGrid.a \
	 $$LIBDIR/libosgGIS.a \
	 $$LIBDIR/libosgGeometry.a \
	 $$LIBDIR/liburban.a \
	 $$LIBDIR/libladar.a \
	 $$LIBDIR/libthreeDgraphics.a \
	 $$LIBDIR/libvideo.a \
	 $$LIBDIR/libimage.a \
	 $$LIBDIR/libosgGraphicals.a \
	 $$LIBDIR/libosgSceneGraph.a \
	 $$LIBDIR/libisds.a \
	 $$LIBDIR/libfusion.a \
	 $$LIBDIR/libkdtree.a \
	 $$LIBDIR/libtrack.a \
	 $$LIBDIR/libstructmotion.a \
	 $$LIBDIR/libeyeglasslodtree.a \
	 $$LIBDIR/libeyeglassio.a \
	 $$LIBDIR/libeyeglassviewer.a \
	 $$LIBDIR/libeyeglassutil.a \
	 $$LIBDIR/libeyeglassmodel.a \
	 $$LIBDIR/libgearth.a \
	 $$LIBDIR/libosg.a \
	 $$LIBDIR/libastrogeo.a \
	 $$LIBDIR/libgeometry.a \
	 $$LIBDIR/libdelaunay.a \
	 $$LIBDIR/libnetwork.a \
	 $$LIBDIR/libplot.a \
	 $$LIBDIR/libwaveform.a \
	 $$LIBDIR/libsocket.a \
	 $$LIBDIR/libfilter.a \
	 $$LIBDIR/libnumerical.a \
	 $$LIBDIR/libpostgres.a \
	 $$LIBDIR/libmath.a \
	 $$LIBDIR/libcolor.a \
	 $$LIBDIR/libweb.a \
	 $$LIBDIR/libgen.a \
	 $$LIBDIR/libdatastructures.a \
	 $$LIBDIR/libgraph.a \
	 $$LIBDIR/libnewmat.a \
	 $$LIBDIR/libnumrec.a \
	 $$LIBDIR/libbayes.a \
	 $$LIBDIR/libklt.a \
	 $$LIBDIR/libkakadu.a \
	 $$LIBDIR/libffmpeg.a \
	 $$LIBDIR/libwebservices.a \
	 $$LIBDIR/libmessenger.a \
	 $$LIBDIR/libtime.a 

LIBS +=  -lpng -lz -ljpeg -L/usr/X11R6/lib \
        -ldl -lfftw -lrfftw -lnetcdf -lm -lcurses -lgdal \
	-L/usr/local/pgsql/lib -lpq -lgeos_c \
        -L$(OSG_OP_OT_PATH)/lib \
        -losgUtil -losgText -losg  \
        -losgSim -losgProducer -losgParticle -losgGA -losgFX -losgDB \
	-lProducer -lOpenThreads \
	-L/opt/intel/ipp41/ia32_itanium/sharedlib \
	-L/opt/intel/ipp41/ia32_itanium/sharedlib/linux32 -lippi \
	-lcurl -ljaula -lidn -lssl -lcrypto \
	-lavformat -lavcodec -lavutil \
	-lcurl -lidn -lssl -lcrypto \
	-lgssapi_krb5 -lkrb5 -lcom_err -lk5crypto -lresolv \
	-lglib-2.0 -pthread -luuid -lgthread-2.0 -lexiv2 \
	-Wl,--export-dynamic -lgmodule-2.0 -ldl \
	-L/usr/local/kakadu/lib \
 	-lkdu_v52R -lkdu_jni -lkdu_a52R \
	/usr/local/lib/libactivemq-cpp.a \
	/usr/local/lib/libtdp.a 

QT += network thread xml
QT -= gui

CONFIG += qt 