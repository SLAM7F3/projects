// ========================================================================
// Program QTANOTSERV asks for an input IP address for the Annotation
// Server which should generally equal the Beast's.  It then starts up
// an Annotation Server on the current machine which can handle
// requests from multiple thin clients.

//	      qtanotserv --GIS_layer ./packages/TOC_metadata.pkg

// ========================================================================
// Last updated on 9/12/10; 9/13/10
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <osgDB/FileUtils>

#include <QtCore/QtCore>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>

#include "Qt/web/AnnotationServer.h"
#include "postgres/gis_databases_group.h"
#include "passes/PassesGroup.h"
#include "osg/osgWindow/ViewerManager.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Initialize Qt application:

   QApplication app(argc,argv);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   int ndims=3;
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

// Instantiate gis database objects to send data to and retrieve
// data from external Postgres database:

   gis_databases_group* gis_databases_group_ptr=new gis_databases_group;
   gis_database* gis_database_ptr=gis_databases_group_ptr->
      generate_gis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);

// Instantiate AnotationServer on BEAST so that it can respond to
// calls from multiple thin clients:

   string AnnotationServer_hostname=gis_database_ptr->get_hostname();
   
/*
   string AnnotationServer_hostname="127.0.0.1";
   string banner="Enter IP address for AnnotationServer:";
   outputfunc::write_big_banner(banner);
   cout << "AnnotationServer IP =  127.0.0.1 for development purposes or Beast's address."
        << endl;
   cout << "IP should agree with tracker.annotationServer's in"
        << endl;
   cout << "/usr/local/apache-tomcat/webapps/bluetracker/script/global.js !"
        << endl << endl;
   cin >> AnnotationServer_hostname;
*/

   int AnnotationServer_portnumber=4041;
   cout << "AnnotationServer_hostname = " << AnnotationServer_hostname
        << " AnnotationServer_portnumber = " << AnnotationServer_portnumber
        << endl;
   AnnotationServer Annotation_server(
      AnnotationServer_hostname,AnnotationServer_portnumber);

// Instantiate ActiveMQ messengers for sending messages from this
// thick client to the BLUETRACKER thin client:

   string broker_URL="tcp://127.0.0.1:61616";
//   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
   string message_sender_ID="ANNOTATIONSERVER";

   string message_queue_channel_name="viewer_update";
   Messenger viewer_messenger( 
      broker_URL,message_queue_channel_name,message_sender_ID);

   string tomcat_subdir="/usr/local/apache-tomcat/webapps/bluetracker/";
   
   Annotation_server.set_gis_database_ptr(gis_database_ptr);
   Annotation_server.set_tomcat_subdir(tomcat_subdir);
   Annotation_server.set_viewer_messenger_ptr(&viewer_messenger);

// ========================================================================

   while( true )
   {
      app.processEvents();
   }

}


