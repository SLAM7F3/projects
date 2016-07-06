// ==========================================================================
// Program QT_VIDEO_ON_DEMAND

// ==========================================================================
// Last updated on 10/20/11
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include <QtCore/QtCore>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>

#include "Qt/web/VideoOnDemandServer.h"
#include "postgres/databasefuncs.h"
#include "general/filefuncs.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

int main (int argc, char * argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Initialize Qt application:

   QApplication app(argc,argv);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

// Instantiate gis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_database_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);

// Instantiate VideoOnDemandServer which receives get calls from web page
// buttons:

   string VideoOnDemandServer_hostname="127.0.0.1";
   int VideoOnDemandServer_portnumber=4060;
   string VideoOnDemandServer_URL;
   if (VideoOnDemandServer_URL.size() > 0)
   {
      VideoOnDemandServer_hostname=stringfunc::get_hostname_from_URL(
         VideoOnDemandServer_URL);
      VideoOnDemandServer_portnumber=stringfunc::get_portnumber_from_URL(
         VideoOnDemandServer_URL);
   }
   cout << "VideoOnDemandServer_hostname = " << VideoOnDemandServer_hostname
        << " VideoOnDemandServer_portnumber = " 
        << VideoOnDemandServer_portnumber
        << endl;
   VideoOnDemandServer VideoOnDemand_server(
      VideoOnDemandServer_hostname,VideoOnDemandServer_portnumber);
   VideoOnDemand_server.set_postgis_database_ptr(postgis_database_ptr);

// ========================================================================

   while(true)
   {
      app.processEvents();
   }
   
   delete postgis_databases_group_ptr;
} 

