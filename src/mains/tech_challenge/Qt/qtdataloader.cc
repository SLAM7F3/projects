// ========================================================================
// Program QTDATALOADER
// ========================================================================
// Last updated on 8/9/10; 8/10/10; 9/6/10
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
#include "astro_geo/Clock.h"
#include "Qt/web/DataloaderServer.h"
#include "postgres/gis_databases_group.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgWindow/MyViewerEventHandler.h"
#include "passes/PassesGroup.h"
#include "track/tracks_group.h"
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

   Clock clock;
   int UTM_zone=19;	// Boston, MA
   clock.compute_UTM_zone_time_offset(UTM_zone);
   clock.set_daylight_savings_flag(true);

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
//   cout << "gis_database_ptr = " << gis_database_ptr << endl;

   tracks_group gps_tracks_group;

// Instantiate DataloaderServer on LOCALHOST (and *NOT* remote BEAST
// machine) which receives get calls from web page buttons.  This
// local server either services the calls itself and/or
// transmits/receives information from the BEAST database:

   string DataloaderServer_hostname="127.0.0.1";
   int DataloaderServer_portnumber=4043;
   string DataloaderServer_URL;
   if (DataloaderServer_URL.size() > 0)
   {
      DataloaderServer_hostname=stringfunc::get_hostname_from_URL(
         DataloaderServer_URL);
      DataloaderServer_portnumber=stringfunc::get_portnumber_from_URL(
         DataloaderServer_URL);
   }
   cout << "DataloaderServer_hostname = " << DataloaderServer_hostname
        << " DataloaderServer_portnumber = " << DataloaderServer_portnumber
        << endl;
   DataloaderServer Dataloader_server(
      DataloaderServer_hostname,DataloaderServer_portnumber);
// Instantiate ActiveMQ messengers:

   string broker_URL="tcp://127.0.0.1:61616";
//   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
   string message_sender_ID="QTDATALOADER";

   string message_queue_channel_name="viewer_update";
   Messenger viewer_messenger( 
      broker_URL,message_queue_channel_name,message_sender_ID);

   Dataloader_server.set_clock_ptr(&clock);
   Dataloader_server.set_gis_database_ptr(gis_database_ptr);
   Dataloader_server.set_gps_tracksgroup_ptr(&gps_tracks_group);
   Dataloader_server.set_viewer_messenger_ptr(&viewer_messenger);

   string command="SEND_THICKCLIENT_READY_FOR_USER_INPUT";
   viewer_messenger.broadcast_subpacket(command);

// ========================================================================

   while( true )
   {
      app.processEvents();
   }

}
