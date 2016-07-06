// ========================================================================
// Program QTPHOTOSERVER starts up a Photo Server on the current
// machine which can handle requests from multiple thin clients.

//	      qtphotoserver --GIS_layer ./packages/TOC_metadata.pkg

// ========================================================================
// Last updated on 8/26/11; 2/20/13; 8/1/13; 11/12/15
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <osgDB/FileUtils>

#include <QtCore/QtCore>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>

#include "Qt/web/PhotoServer.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
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

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_database_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);

// Instantiate PhotoServer on BEAST so that it can respond to
// calls from multiple thin clients:

   string PhotoServer_hostname=postgis_database_ptr->get_hostname();
   int PhotoServer_portnumber=4046;
   cout << "PhotoServer_hostname = " << PhotoServer_hostname
        << " PhotoServer_portnumber = " << PhotoServer_portnumber
        << endl;
   PhotoServer Photo_server(PhotoServer_hostname,PhotoServer_portnumber);

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=false;
   bool display_movie_number=false;
   bool hide_Mode_HUD_flag=false;
   Operations operations(
      ndims,NULL,passes_group,
      display_movie_state,display_movie_number,hide_Mode_HUD_flag);

   string tomcat_subdir="/usr/local/apache-tomcat/webapps/photo/";
   
// Photo server variable settings:

   string home_dir = getenv("HOME");
   Photo_server.set_JSON_filename(home_dir+"/graph.json");
   Photo_server.set_Operations_ptr(&operations);
   Photo_server.set_postgis_database_ptr(postgis_database_ptr);
   Photo_server.set_tomcat_subdir(tomcat_subdir);

// ========================================================================

   while( true )
   {
      app.processEvents();
      usleep(100);
   }

}


