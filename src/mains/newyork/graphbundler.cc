// ========================================================================
// Program GRAPHBUNDLER reads in the SIFT adjacency matrix generated
// by Noah Snavely's bundler program which establishes links between
// two photos if they share SIFT features in common.  GRAPHBUNDLER
// issues a series of ADD_EDGE calls to Michael Yee's GraphExplorer
// tool.  Michael's tool can therefore be used to visualize Noah's
// graphs.

// After starting Michael Yee's GraphExplorer program, simply chant

//		       		graphbundler

// in order to start sending ActiveMQ messages to Michael's tool.

// ========================================================================
// Last updated on 6/26/09; 6/30/09; 7/15/09; 7/19/09; 12/4/10
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
   string message_sender_ID="GRAPHBUNDLER";

// Instantiate urban network messengers for communication with Michael
// Yee's social network tool:

   string photo_network_message_queue_channel_name="photo_network";
   bool include_sender_and_timestamp_info_flag=false;
   Messenger photo_network_messenger( 
      broker_URL,photo_network_message_queue_channel_name,message_sender_ID,
      include_sender_and_timestamp_info_flag);

// Instantiate photogroup to hold Bundler photos:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->set_Messenger_ptr(&photo_network_messenger);

/*
   string subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/newyork/bundler/Manhattan/";
   string image_list_filename=subdir+"list.compressed.txt";
   string bundle_compressed_filename=subdir+"bundle.compressed.out";
   string covariances_filename=subdir+"covariances.txt";
   string matches_filename="reduced_matrix.txt";
*/

   string subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/newyork/bundler/nyc_1000/Manhattan1012Compressed/";
   string image_list_filename=subdir+"Manhattan.1012.txt";
   string covariances_filename=subdir+"covariances.txt";
   string matches_filename=subdir+"nmatches.M1012.txt";
   string image_sizes_filename=subdir+"image_sizes_1012.dat";
//   string message_sender_ID="DISALLOW_SELF_MESSAGES";

//   int n_photos_to_reconstruct=-1;
//   int n_photos_to_reconstruct=5;
//   int n_photos_to_reconstruct=50;
//   int n_photos_to_reconstruct=100;
   int n_photos_to_reconstruct=300;
//   int n_photos_to_reconstruct=500;
   photogroup_ptr->generate_bundler_photographs(
      subdir,image_list_filename,image_sizes_filename,
      n_photos_to_reconstruct);
//   photogroup_ptr->export_image_sizes();

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

// Generate nodes within photo network:

   cout << "Generating nodes in photo network:" << endl;
   for (int i=0; i<n_photos; i++)
   {
      cout << i << " " << flush;
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);

/*
      int n_total_SIFT_matches=0;
      for (int j=0; j<n_photos; j++)
      {
         n_total_SIFT_matches += matches_twoDarray_ptr->get(i,j);
      }
      photograph_ptr->set_n_matching_SIFT_features(n_total_SIFT_matches);
*/

//      cout << "photograph_ptr = " << photograph_ptr << endl;
      photogroup_ptr->issue_add_vertex_message(photograph_ptr);
   } // loop over index i labeling photographs
   cout << endl;

//   sleep(15);

// Generate edges between nodes in photo network:

   int n_min_edges=1000000;
   int n_max_edges=1;
   vector<double> n_matches;

   cout << "Generating edges in photo network:" << endl;
   cout << "n_photos = " << n_photos << endl;
   for (int i=0; i<n_photos; i++)
   {
      cout << i << " " << flush;
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);

      for (int j=i+1; j<n_photos; j++)
      {
         int curr_matches=matches_twoDarray_ptr->get(i,j);
         if (curr_matches > 0.5*POSITIVEINFINITY)
         {
            cout << "i = " << i << " j = " << j
                 << " curr_matches = " << curr_matches << endl;
            outputfunc::enter_continue_char();
         }
         
         if (curr_matches > 0)
         {
            n_matches.push_back(curr_matches);
            n_min_edges=basic_math::min(n_min_edges,curr_matches);
            n_max_edges=basic_math::max(n_max_edges,curr_matches);
            photograph* next_photograph_ptr=photogroup_ptr->
               get_photograph_ptr(j);
            photogroup_ptr->issue_add_edge_message(
               photograph_ptr,next_photograph_ptr,curr_matches);
         }
      } // loop over index j

      if (i%25==0)
      {
         sleep(2);
      }
      
   } // loop over index i labeling cameras

//   cout << "n_matches.size() = " << n_matches.size() << endl;
//   for (int n=0; n<n_matches.size(); n++)
//   {
//      cout << "n = " << n << " n_matches = " << n_matches[n] << endl;
//   }

/*
   double n_minimum=0;
   int n_output_bins=300;
   prob_distribution prob(n_matches,n_output_bins,n_minimum);
   prob.compute_density_distribution();
   prob.compute_cumulative_distribution();

   cout << "prob.median() = " << prob.median() << endl;
   cout << "prob.quartile_width() = " << prob.quartile_width() << endl;

   for (int l=75; l<=99; l++)
   {
      double pcum=0.01*l;
      double x=prob.find_x_corresponding_to_pcum(pcum);
      cout << "pcum = " << pcum << " x = " << x << endl;
   }

   prob.writeprobdists();
*/

//   cout << "n_min_edges = " << n_min_edges 
//        << " n_max_edges = " << n_max_edges << endl;

// n_min_edges = 16
// n_max_edges = 8625

}
