// ========================================================================
// Program DUMMY_VIDEOS_EDGELIST queries the user for campaign and
// mission IDs corresponding to some set of video clips.  It generates
// a dummy edgelist for this imagery set where each video frame within
// a clip is connected to its nearest temporal neighbors.  Frames
// between video clips are disconnected.  So the number of
// disconnected components within the image graph equals the number of
// distinct video clips.

// ========================================================================
// Last updated on 10/29/13; 10/30/13; 2/9/14
// ========================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "templates/mytemplates.h"
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
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string graphs_subdir=bundler_IO_subdir+"graphs/";
   filefunc::dircreate(graphs_subdir);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   int campaign_ID,mission_ID;
   cout << "Enter campaign_ID:" << endl;
   cin >> campaign_ID;
   cout << "Enter mission_ID:" << endl;
   cin >> mission_ID;

   vector<int> image_IDs,datum_IDs;
   vector<string> image_URLs;
   if (!imagesdatabasefunc::retrieve_image_metadata_from_database(
      postgis_db_ptr,campaign_ID,mission_ID,
      image_IDs,datum_IDs,image_URLs))
   {
      exit(-1);
   }
   templatefunc::Quicksort(image_IDs,datum_IDs,image_URLs);

// Generate initial edgelist among video frames which simply links
// every frame within a clip to its nearest temporal neighbors.  No
// links initially exist between frames within different clips:

   ofstream edgestream;
//   string edgelist_filename=graphs_subdir+"videoclips_edgelist.dat";
//   string edgelist_filename=graphs_subdir+"sift_edgelist.dat";
   string edgelist_filename=graphs_subdir+"dummy_edgelist.dat";
   filefunc::openfile(edgelist_filename,edgestream);
   edgestream << "# Edge weight threshold = 0" << endl;
   edgestream << "# NodeID  NodeID'  Edge weight" << endl << endl;

   int edge_weight=100;
   int n_frames_in_clip=0;
   int prev_image_ID=-1;
   int prev_clip_ID=-1;
   vector<int> n_clusters;
   vector<string> image_basenames;
   for (unsigned int i=0; i<image_IDs.size(); i++)
   {
      image_basenames.push_back(filefunc::getbasename(image_URLs[i]));
      vector<string> substrings=
         stringfunc::decompose_string_into_substrings(
            filefunc::getbasename(image_URLs[i])," _-");
//      for (int s=0; s<substrings.size(); s++)
//      {
//         cout << "i = " << i << " s = " << s << " substring[s] = " 
//              << substrings[s] << endl;
//      }

      int clip_ID=-1;
//      int frame_ID=-1;
      if (substrings.size()==4)
      {
         if (substrings[0]=="clip")
         {
            clip_ID=stringfunc::string_to_number(substrings[1]);
//            frame_ID=stringfunc::string_to_number(substrings[3]);
            n_frames_in_clip++;
         }
      }

      bool export_edge_flag=true;
      if (i==image_IDs.size()-1)
      {
         n_clusters.push_back(n_frames_in_clip);
      }
      else if (prev_clip_ID > 0 && clip_ID != prev_clip_ID)
      {
         n_clusters.push_back(n_frames_in_clip-1);
         n_frames_in_clip=1;
         export_edge_flag=false;
      }

/*
      cout << "i = " << i << "  "
           << "clip_ID = " << clip_ID << " "
           << "frame_ID = " << frame_ID << " "
           << image_IDs[i] << " "
           << datum_IDs[i] << " "
           << image_basenames[i] << endl;
*/

      if (export_edge_flag && prev_image_ID >= 0)
      {
         edgestream << prev_image_ID << "  "
                    << image_IDs[i] << "  "
                    << edge_weight << endl;
      }

      prev_clip_ID=clip_ID;
      prev_image_ID=image_IDs[i];
      
   } // loop over index i labeling all video frames
   filefunc::closefile(edgelist_filename,edgestream);

   string banner="Exported video clips' dummy edgelist to"+edgelist_filename;
   outputfunc::write_big_banner(banner);

/*
// Generate soft link between edgelist_filename and
// bundler_IO_subdir/graphs/sift_edgelist.dat:
   
   string sift_edgelist_filename=graphs_subdir+"sift_edgelist.dat";
   cout << "sift_edgelist_filename = " << sift_edgelist_filename << endl;
   filefunc::deletefile(sift_edgelist_filename);
   string unix_cmd="ln -s "+edgelist_filename+" "+
      graphs_subdir+"sift_edgelist.dat";
   cout << "unix_cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);
*/

}
