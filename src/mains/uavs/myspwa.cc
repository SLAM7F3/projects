// ========================================================================
// Program MYSPWA is a test playground for parsing a polyline KML file
// generated from within GoogleEarth.  

//	      myspwa --ActiveMQ_hostname tcp://155.34.125.216:61616

// ========================================================================
// Last updated on 3/2/08; 3/4/08; 3/10/08
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osg/ArgumentParser>
#include "general/filefuncs.h"
#include "gearth/kml_parser.h"
#include "messenger/Messenger.h"
#include "passes/PassesGroup.h"
#include "robots/SAMs_group.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/Terrain_Manipulator.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input data files:
   
   PassesGroup passes_group(&arguments);

// Instantiate SAM messenger:

   int pass_ID=passes_group.get_n_passes()-1;
   string broker_URL=passes_group.get_pass_ptr(pass_ID)->
      get_PassInfo_ptr()->get_ActiveMQ_hostname();
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
//   string broker_URL="tcp://155.34.125.216:61616";	// family day
//   string broker_URL="tcp://155.34.135.239:61616";	// G104 conf room

   string SAM_message_queue_channel_name="SAM";
   Messenger SAM_messenger( 
      broker_URL, SAM_message_queue_channel_name );

// Initialize SAM sites from input text file information:

   SAMs_group SAMs(&SAM_messenger);

   string SAM_sites_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/uavs/sam_sites/";
//   string SAM_list_filename=SAM_sites_subdir+"single.sams";
//   string SAM_list_filename=SAM_sites_subdir+"triple.sams";
//   string SAM_list_filename=SAM_sites_subdir+"north_korea.sams";
//   string SAM_list_filename=SAM_sites_subdir+"iran.sams";
//   string SAM_list_filename=SAM_sites_subdir+"china.sams";
   string SAM_list_filename=SAM_sites_subdir+"total.sams";
   SAMs.generate_all_SAMs(SAM_list_filename);

// Parse candidate flight paths entered via Google Earth as polyline
// KML files:

//   string subdir=
//      "/home/cho/programs/c++/svn/projects/src/mains/uavs/kml_files/";
   string subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/uavs/flight_paths/";
   string untitled_filename=subdir+"Untitled\ Path.kml";
   string untitled_prefix=subdir+"Untitled*Path.kml";

   kml_parser polyline_kml_parser;
   int path_number=0;

// Infinite event loop:

   while (true)
   {
      if (filefunc::fileexist(untitled_filename))
      {

// Look for new KML file entitled "Untitled filename.kml" within
// kml_files subdirectory.  If such a file is found, rename it as
// "path_n.kml".  Then parse its contents and generate a new polyline
// object:

//         cout << "Untitled filename = " << untitled_filename << endl;
//         cout << "Untitled file " << path_number << " found in subdir"
//              << endl;

         string path_filename=subdir+"path_"+
            stringfunc::number_to_string(path_number)+".kml";
//         cout << "path_filename = " << path_filename << endl;

         string unixcommandstr="mv -v "+untitled_prefix+" "
            +path_filename;
//         cout << "unixcommand = " << unixcommandstr << endl;
         sysfunc::unix_command(unixcommandstr);

//         cout << "polyline kml parser input filename = "
//              << path_filename << endl;
         polyline flight_path=
            polyline_kml_parser.parse_polyline_kml(path_filename);
         
         double flight_path_distance_in_meters=
            SAMs.compute_flight_path_distance(flight_path);
         SAMs.find_closest_SAM_site(
            flight_path,flight_path_distance_in_meters);

         int closest_SAM_ID=SAMs.get_closest_SAM_ptr()->get_ID();
         colorfunc::Color danger_color=colorfunc::yellow;
         SAMs.generate_single_SAM_site_KML_file(closest_SAM_ID,danger_color);

         path_number++;

      } // untitled filename exists conditional
   } // infinite while loop
          

}

