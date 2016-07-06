// ==========================================================================
// Program REFINE_FACE_DETECTIONS

// ==========================================================================
// Last updated on 11/1/13
// ==========================================================================

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "math/prob_distribution.h"
#include "video/RGB_analyzer.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"
#include "video/videosdatabasefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();      

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
   string root_subdir=JAV_subdir;
   string faces_subdir=root_subdir+"faces/";
   
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

   typedef map<int,vector<int> > KEYFRAME_IMAGE_IDS_MAP;
   KEYFRAME_IMAGE_IDS_MAP::iterator keyframe_image_IDs_iter;

// independent int = keyframe ID
// dependent STL vector holds integer IDs for images associated with
// a particular keyframe

   KEYFRAME_IMAGE_IDS_MAP* keyframe_image_IDs_map_ptr=
      videosdatabasefunc::get_keyframe_IDs(
         postgis_db_ptr,campaign_ID,mission_ID);


   typedef map<int,vector<threevector> > KEYFRAME_FACES_MAP;
   KEYFRAME_FACES_MAP keyframe_faces_map;
   KEYFRAME_FACES_MAP::iterator keyframe_faces_iter;
   
// independent int = keyframe_ID
// dependent STL vector holds threevectors containing circle center
// and radius values

   string face_detections_filename=faces_subdir+"face_detections.txt";
   vector<vector<string> > all_substrings;
   filefunc::ReadInSubstrings(face_detections_filename,all_substrings);
   for (int i=0; i<all_substrings.size(); i++)
   {
      string image_filename=all_substrings[i].at(0);
      string image_basename=filefunc::getbasename(image_filename);
      string basename_prefix=stringfunc::prefix(image_basename);

      string separator_chars="_-";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         basename_prefix,separator_chars);
      int clip_ID=stringfunc::string_to_number(substrings[1]);
      int frame_ID=stringfunc::string_to_number(substrings[3]);

      int image_ID,keyframe_ID;
      videosdatabasefunc::get_image_and_keyframe_IDs(
         postgis_db_ptr,campaign_ID,mission_ID,clip_ID,frame_ID,
         image_ID,keyframe_ID);
      
//      cout << "clip_ID = " << clip_ID << " frame_ID = " << frame_ID 
//           << " image_ID = " << image_ID << " keyframe_ID = " << keyframe_ID
//           << endl;

      double circle_U=stringfunc::string_to_number(all_substrings[i].at(1));
      double circle_V=stringfunc::string_to_number(all_substrings[i].at(2));
      double circle_radius=stringfunc::string_to_number(
         all_substrings[i].at(3));
      threevector circle_coords(circle_U,circle_V,circle_radius);
      
      keyframe_faces_iter=keyframe_faces_map.find(keyframe_ID);
      if (keyframe_faces_iter==keyframe_faces_map.end())
      {
         vector<threevector> V;
         V.push_back(circle_coords);
         keyframe_faces_map[keyframe_ID]=V;
      }
      else
      {
         keyframe_faces_iter->second.push_back(circle_coords);
      }
   } // loop over index i labeling all_substrings

   cout << "keyframe_faces_map.size() = " << keyframe_faces_map.size()
        << endl;

// For each video temporal segment represented by a keyframe, count
// number of images which have face(s):

   const double min_frac_images_w_faces=0.75;
   for (keyframe_image_IDs_iter=keyframe_image_IDs_map_ptr->begin();
        keyframe_image_IDs_iter != keyframe_image_IDs_map_ptr->end();
        keyframe_image_IDs_iter++)
   {
      int keyframe_ID=keyframe_image_IDs_iter->first;
      int n_associated_images=keyframe_image_IDs_iter->second.size();

      int n_images_w_faces=0;
      keyframe_faces_iter=keyframe_faces_map.find(keyframe_ID);
      if (keyframe_faces_iter != keyframe_faces_map.end())
      {
         n_images_w_faces=keyframe_faces_iter->second.size();
      }
      double frac_images_w_faces=double(n_images_w_faces)/
         double(n_associated_images);

      cout << "keyframe_ID = " << keyframe_ID
           << " n_images_w_faces = " << n_images_w_faces
           << " n_associated_images = " << n_associated_images
           << " frac = " << frac_images_w_faces
           << endl;
      
      if (frac_images_w_faces < min_frac_images_w_faces &&
      n_images_w_faces > 0)
      {
         for (int i=0; i<n_associated_images; i++)
         {
            int curr_image_ID=keyframe_image_IDs_iter->second.at(i);
            int clip_ID,frame_ID;
            videosdatabasefunc::get_clip_and_frame_IDs(
               postgis_db_ptr,campaign_ID,mission_ID,curr_image_ID,
               clip_ID,frame_ID);
            string image_basename=
               "clip_"+stringfunc::integer_to_string(clip_ID,4)+
               "_frame-"+stringfunc::integer_to_string(frame_ID,5)+".jpg";
            cout << image_basename << " does NOT have face(s)" << endl;
         }
         outputfunc::enter_continue_char();
      } // frac_images_w_faces conditional

   } // iterator over *keyframe_image_IDs_map_ptr

   delete keyframe_image_IDs_map_ptr;

   cout << "At end of program REFINE_FACE_DETECTIONS" << endl;
   outputfunc::print_elapsed_time();
}


