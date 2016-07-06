// ========================================================================
// Program GRAPHJSON reads in the SIFT adjacency matrix generated
// by Noah Snavely's bundler program which establishes links between
// two photos if they share SIFT features in common.  GRAPHJSON
// issues a series of ADD_EDGE calls to Michael Yee's GraphExplorer
// tool.  Michael's tool can therefore be used to visualize Noah's
// graphs.

// After starting Michael Yee's GraphExplorer program, simply chant

//		       		graphjson

// in order to start sending ActiveMQ messages to Michael's tool.

// ========================================================================
// Last updated on 7/15/09; 7/19/09; 7/21/09; 8/28/09
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_database.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"
#include "math/prob_distribution.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

// Instantiate separate messengers for each Decorations group which
// needs to receive mail:

   string broker_URL="tcp://127.0.0.1:61616";
   cout << "ActiveMQ broker_URL = " << broker_URL << endl;
   string message_sender_ID="GRAPHJSON";

// Instantiate urban network messengers for communication with Michael
// Yee's social network tool:

   string photo_network_message_queue_channel_name="photo_network";
//   bool include_sender_and_timestamp_info_flag=false;
   bool include_sender_and_timestamp_info_flag=true;
   Messenger photo_network_messenger( 
      broker_URL,photo_network_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);

// Instantiate photogroup to hold Bundler photos:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->set_Messenger_ptr(&photo_network_messenger);

   char json_str;
   cout << "Enter 't' to transmit existing JSON graph to GraphExplorer"
        << endl;
   cin >> json_str;
   if (json_str=='t')
   {
      string JSON_URL=
         "http://127.0.0.1:8080/photosynth/nyc_1000/graphs/graph.json";
      photogroup_ptr->issue_build_graph_message(JSON_URL);
      exit(-1);
   }
   
   photogroup_ptr->set_base_thumbnail_URL(
      "http://127.0.0.1:8080/photosynth/nyc_1000/images/thumbnails/");

   string subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/newyork/bundler/nyc_1000/Manhattan1012Compressed/";
   string image_list_filename=subdir+"Manhattan.1012.txt";
   string covariances_filename=subdir+"covariances.txt";
   string matches_filename=subdir+"nmatches.M1012.txt";
   string image_sizes_filename=subdir+"image_sizes_1012.dat";

/*
//   int n_photos_to_reconstruct=-1;
//   int n_photos_to_reconstruct=10;
//   int n_photos_to_reconstruct=15;
//   int n_photos_to_reconstruct=100;
//   int n_photos_to_reconstruct=200;
//   int n_photos_to_reconstruct=500;
   photogroup_ptr->generate_bundler_photographs(
      subdir,image_list_filename,image_sizes_filename,
      n_photos_to_reconstruct);
*/

   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);

// Read in Noah's covariance traces for each photograph:

   photogroup_ptr->read_photograph_covariance_traces(
      covariances_filename);

// Instantiate and fill twoDarray holding Noah's SIFT track matches
// between actual reconstructed images:

   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;
   twoDarray* matches_twoDarray_ptr=new twoDarray(n_photos,n_photos);

   filefunc::ReadInfile(matches_filename);

   for (int i=0; i<n_photos; i++)
   { 
//      cout << i << " " << flush;
      vector<double> curr_row_entries=
         stringfunc::string_to_numbers(filefunc::text_line[i]);
//      cout << "curr_row_entries.size() = "
//           << curr_row_entries.size() << endl;
      for (int j=0; j<n_photos; j++)
      {
         matches_twoDarray_ptr->put(i,j,curr_row_entries[j]);
      } // loop over index j labeling original photos
   } // loop over index i labeling original photos
   cout << endl;

//   double min_value,max_value;
//   matches_twoDarray_ptr->minmax_values(min_value,max_value);
//   cout << "min_value = " << min_value 
//        << " max_value = " << max_value << endl;

   photogroup_ptr->issue_build_graph_message(matches_twoDarray_ptr);
}
