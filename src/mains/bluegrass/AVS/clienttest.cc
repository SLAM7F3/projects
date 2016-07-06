// ==========================================================================
// Program CLIENTTEST
// ==========================================================================
// Last updated on 4/29/08
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osg/ArgumentParser>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <QtCore/QtCore>

#include "Qt/web/BluegrassClient.h"
#include "passes/PassesGroup.h"
#include "general/sysfuncs.h"
#include "track/tracklist.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Initialize Qt objects:

   QCoreApplication app(argc,argv);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input data files:
   
   PassesGroup passes_group(&arguments);

   int pass_ID=passes_group.get_n_passes()-1;
   string BluegrassServer_URL=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_WebServer_URL();
   cout << "BluegrassServer_URL = " << BluegrassServer_URL << endl;

// Instantiate BluegrassClient which uses Qt http functionality:

   BluegrassClient* BluegrassClient_ptr=new BluegrassClient(
      BluegrassServer_URL);

   string curr_query="/vehicle_tracks/?";
   curr_query += "min_longitude=-101.97&min_latitude=33.48";
   curr_query += "&max_longitude=-101.91&max_latitude=33.53";
   curr_query += "&t_start=1190908800&t_stop=1190912400";

   BluegrassClient_ptr->query_BluegrassServer(curr_query);
   TrackList* tracklist_ptr=
      BluegrassClient_ptr->generate_tracks_from_parsed_XML(
         BluegrassClient_ptr->get_returned_output());

// ========================================================================


   cout << "Before starting infinite Qt event loop" << endl;
   return app.exec();

}
