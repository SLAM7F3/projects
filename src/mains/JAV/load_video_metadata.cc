// ========================================================================
// Program LOAD_VIDEO_METADATA queries the user to enter campaign and
// mission IDs for a set of video clips to be loaded into the videos
// table of the IMAGERY database.  For each video frame, it retrieves
// the image ID and URL from the images table.  It also extracts the
// clip and frame ID from the image's filename.  If an audio
// transcript and/or mp4 file exists within the same subdirectory of
// /data/ImageEngine/ as the jpg images, their URLs are recovered as well. 
// For each video frame from multiple clips, these metadata are stored
// within the videos table

// 	./load_video_metadata --GIS_layer ./packages/imagery_metadata.pkg

// ========================================================================
// Last updated on 10/30/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"
#include "video/videosdatabasefuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::map;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
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
   imagesdatabasefunc::retrieve_image_metadata_from_database(
      postgis_db_ptr,campaign_ID,mission_ID,
      image_IDs,datum_IDs,image_URLs);

   int n_images=image_URLs.size();
   for (int i=0; i<n_images; i++)
   {
      outputfunc::update_progress_fraction(i,100,n_images);

//      cout << "image_URL = " << image_URLs[i] << endl;
      string image_dirname=filefunc::getdirname(image_URLs[i]);

      string image_basename=filefunc::getbasename(image_URLs[i]);
      string basename_prefix=stringfunc::prefix(image_basename);
//      cout << "basename_prefix = " << basename_prefix << endl;
      
// As of 10/30/13, we assume video images have names of the form
// clip_0020_frame-00002.jpg :

      string separator_chars="_-";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         basename_prefix,separator_chars);
      int clip_ID=stringfunc::string_to_number(substrings[1]);
      int frame_ID=stringfunc::string_to_number(substrings[3]);
//      cout << "clip_ID = " << clip_ID 
//           << " frame_ID = " << frame_ID << endl;

      string transcripts_subdir=image_dirname+"transcripts/";      
      string transcript_filename=transcripts_subdir+
         "clip_"+stringfunc::integer_to_string(clip_ID,4)+".transcript";
      if (filefunc::fileexist(transcript_filename))
      {
//         cout << "transcript = " << transcript_filename << endl;
      }
      else
      {
         transcript_filename="";
      }
      

      string videos_subdir=image_dirname+"videos/";      
      string video_filename=videos_subdir+
         "clip_"+stringfunc::integer_to_string(clip_ID,4)+".mp4";
      if (filefunc::fileexist(transcript_filename))
      {
      }
      else
      {
         video_filename="";
      }

      videosdatabasefunc::insert_video_metadata(
         postgis_db_ptr,campaign_ID,mission_ID,image_IDs[i],
         clip_ID,frame_ID,video_filename,transcript_filename);


   } // loop over index i labeling video images
   
}
