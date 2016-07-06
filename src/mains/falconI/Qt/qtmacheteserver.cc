// ========================================================================
// Program QTMACHETESERVER starts up a Machete Server on the current
// machine which can handle requests from multiple thin clients.

//	qtmacheteserver --GIS_layer ./packages/machete_metadata.pkg

// ========================================================================
// Last updated on 4/4/12; 4/5/12
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <QtCore/QtCore>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>

#include "astro_geo/Clock.h"
#include "Qt/web/MacheteServer.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"

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

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_database_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);

// FAKE FAKE:  Thurs Apr 5, 2012 
// Temporarily hardwire in local UTM zoneumber and daylight savings flag

   Clock clock;
   int local_UTM_zonenumber=19;	// Boston
   bool daylight_savings_flag=true;
   clock.set_time_based_on_local_computer_clock(
      local_UTM_zonenumber,daylight_savings_flag);

// Instantiate MacheteServer on BEAST so that it can respond to
// calls from multiple thin clients:

   string MacheteServer_hostname=postgis_database_ptr->get_hostname();
   int MacheteServer_portnumber=4046;
   cout << "MacheteServer_hostname = " << MacheteServer_hostname
        << " MacheteServer_portnumber = " << MacheteServer_portnumber
        << endl;
   MacheteServer Machete_server(
      MacheteServer_hostname,MacheteServer_portnumber);

   string tomcat_subdir="/usr/local/apache-tomcat/webapps/machete/";
   
// Machete server variable settings:

   Machete_server.set_clock_ptr(&clock);
   Machete_server.set_postgis_database_ptr(postgis_database_ptr);
   Machete_server.set_tomcat_subdir(tomcat_subdir);



// ========================================================================

   while( true )
   {
      app.processEvents();
   }

}


