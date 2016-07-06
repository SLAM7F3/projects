# =========================================================================
# Last updated on 10/1/13; 12/2/13; 1/4/14; 11/4/14
# =========================================================================
# Removed libopencv_contrib, libopencv_legacy and libopencv_nonfree which
# are not used for OpenCV 3.0.0
# =========================================================================

INCLUDEDIR=$$PREFIX/include
LIBDIR=$$PREFIX/lib

INCLUDEPATH += $$INCLUDEDIR

INCLUDEPATH += . \
        /usr/local/axiscpp_deploy/include \
	/usr/local/OpenCV/include/ \
	/usr/include/freetype2/ \
	/usr/local/include/ceres/ \
	/usr/local/include/glog/ \
	/usr/local/include/graphchi/ \
	/usr/local/include/eigen3/ \
	/usr/local/include/OpenSURF/ \
        $(OSG_OP_OT_PATH)/include \
        /usr/local/include /usr/local/pgsql/include \
	/usr/include/gdal \
	/opt/gdal/include/ \
	/opt/geos/include/ \
	/usr/local/include/dlib \
	/usr/include/ImageMagick \
	/usr/include/postgresql \
	$$INCLUDEDIR/eyeglass/ \
   	/usr/local/include/xsens \
   	/usr/local/include/exiv2 \
   	/usr/include/exiv2 \
	/usr/local/include/ANN \
	/usr/local/include/trianglepp \
	/usr/local/include/libtdp \
	/usr/local/include/activemq-cpp \
	/usr/include/glib-2.0 /usr/lib/glib-2.0/include 


LIBS =   $$LIBDIR/libspace.a \
	 $$LIBDIR/libclassification.a \
	 $$LIBDIR/libbundler.a \
	 $$LIBDIR/liboptimum.a \
	 $$LIBDIR/librobots.a \
	 $$LIBDIR/libosgClipping.a \
	 $$LIBDIR/libosgOrganization.a \
	 $$LIBDIR/libosgRTPS.a \
	 $$LIBDIR/libosgFusion.a \
	 $$LIBDIR/libosgFeatures.a \
	 $$LIBDIR/libosgPanoramas.a \
	 $$LIBDIR/libosgModels.a \
	 $$LIBDIR/libosgAnnotators.a \
	 $$LIBDIR/libosgEarth.a \
	 $$LIBDIR/libosgSpace.a \
	 $$LIBDIR/libgearth.a \
	 $$LIBDIR/libosg3D.a \
	 $$LIBDIR/libosgOperations.a \
	 $$LIBDIR/libosg2D.a \
	 $$LIBDIR/libosgTiles.a \
	 $$LIBDIR/libosgGrid.a \
	 $$LIBDIR/libosgGIS.a \
	 $$LIBDIR/libosgRegions.a \
	 $$LIBDIR/libosgGeometry.a \
	 $$LIBDIR/liburban.a \
	 $$LIBDIR/libladar.a \
	 $$LIBDIR/libvideo.a \
	 $$LIBDIR/libosgGraphicals.a \
	 $$LIBDIR/libosgSceneGraph.a \
	 $$LIBDIR/libthreeDgraphics.a \
	 $$LIBDIR/libisds.a \
	 $$LIBDIR/libtrack.a \
	 $$LIBDIR/libstructmotion.a \
	 $$LIBDIR/libeyeglasslodtree.a \
	 $$LIBDIR/libeyeglassio.a \
	 $$LIBDIR/libeyeglassviewer.a \
	 $$LIBDIR/libeyeglassutil.a \
	 $$LIBDIR/libeyeglassmodel.a \
	 $$LIBDIR/libosgWindow.a \
	 $$LIBDIR/libosg.a \
	 $$LIBDIR/libcoincidence.a \
	 $$LIBDIR/libdistancetransform.a \
	 $$LIBDIR/libimage.a \
	 $$LIBDIR/libgraph.a \
	 $$LIBDIR/libpostgres.a \
	 $$LIBDIR/libpasses.a \
	 $$LIBDIR/libastrogeo.a \
	 $$LIBDIR/libmodels.a \
	 $$LIBDIR/libgeometry.a \
	 $$LIBDIR/libdelaunay.a \
	 $$LIBDIR/libnetwork.a \
	 $$LIBDIR/libplot.a \
	 $$LIBDIR/libwaveform.a \
	 $$LIBDIR/libsocket.a \
	 $$LIBDIR/libfilter.a \
	 $$LIBDIR/libnumerical.a \
	 $$LIBDIR/libxml.a \
	 $$LIBDIR/libkdtree.a \
	 $$LIBDIR/libcluster.a \
	 $$LIBDIR/libmachinelearning.a \
	 $$LIBDIR/libmath.a \
	 $$LIBDIR/libcolor.a \
	 $$LIBDIR/libweb.a \
	 $$LIBDIR/libmessenger.a \
	 $$LIBDIR/libtime.a \
	 $$LIBDIR/libgen.a \
	 $$LIBDIR/libdatastructures.a \
	 $$LIBDIR/libnewmat.a \
	 $$LIBDIR/libnumrec.a \
	 $$LIBDIR/libbayes.a \
	 $$LIBDIR/libklt.a \
	 $$LIBDIR/libkht.a \
	 $$LIBDIR/libalglib.a \
	 $$LIBDIR/libdavis.a \
	 $$LIBDIR/libffmpeg.a 


LIBS += -L/usr/local/lib \
        -L/usr/X11R6/lib \
	-L/usr/local/OpenCV/lib \
	-L/usr/lib/x86_64-linux-gnu \
	-L/usr/local/pgsql/lib \
        -L/usr/lib/atlas \
        -L/usr/local/lib/bundler \
        -L/opt/gdal/lib \
        -L/opt/geos/lib \
	-lLBP \
	-lceres -lglog -lgflags -lcholmod -lcxsparse -lamd -lcolamd \
	-lsuitesparseconfig \
	-lconnexe \
	-lfastann -ltrimesh \
	-lpngwriter \
	-lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_video \
	-lopencv_features2d -lopencv_ml -lopencv_highgui \
	-lopencv_objdetect \
        -ldl -lfftw -lrfftw -lnetcdf -lm -lcurses -lgdal \
        -lpq -lgeos_c -latlas \
        -losgUtil -losgText -losg  \
        -losgSim -losgProducer -losgParticle -losgGA -losgFX -losgDB \
	-lProducer -lOpenThreads \
	-lcurl -ljaula -lidn -lssl -lcrypto \
        /usr/local/lib/libavdevice.a \
        /usr/local/lib/libavfilter.a \
        /usr/local/lib/libavformat.a \
        /usr/local/lib/libavcodec.a \
        /usr/local/lib/libavutil.a \
        /usr/local/lib/libswscale.a \
	-lgssapi_krb5 -lkrb5 -lcom_err -lk5crypto -lresolv \
	-lglib-2.0 -lpthread -lgthread-2.0 -lexiv2 \
	-Wl,--export-dynamic -lgmodule-2.0 -ldl \
	/usr/local/lib/libactivemq-cpp.a \
        -luuid -lX11 \
	/usr/local/lib/libANN.a \
        -llevmar -lsba -llapack -lf2c -llmmin \
    	-lCg -lCgGL -lGLEW -lglut -lIL \
    	-lOGDF -lsqlite3 -lcmt -ldel \
	-lgomp -lGL -lhdf5 -lblas -lfreetype \
    	-lserial -lMagick++ -lz -lbz2 \
        -lfann \
        -l5point -lcblas -lf2c -limage -lmatrix -lminpack \
        -lsba.v1.5 -lsfmdrv \
        -lpng -ljpeg -lX11 \
	/usr/local/lib/libtdp.a

