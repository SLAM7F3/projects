// ========================================================================
// Program LOAD_IMAGE_METADATA queries the user to enter campaign and
// mission IDs for a set of images to be loaded into the images table
// of the IMAGERY database.  It also requests a subdirectory of
// /data/ImageEngine/ where a set of image and thumbnail files must
// already exist.  LOAD_IMAGE_METADATA then inserts metadata for
// images within image_list_filename into the images database table.

// 	./load_image_metadata --GIS_layer ./packages/imagery_metadata.pkg

// ========================================================================
// Last updated on 4/14/12; 11/20/13; 6/17/14; 11/28/15
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

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
   timefunc::initialize_timeofday_clock();

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

   string image_list_filename=passes_group.get_image_list_filename();
   cout << " image_list_filename = " << image_list_filename << endl;

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

   string images_subdir;
   cout << "Enter subdirectory of /data/ImageEngine/ in which images reside:"
        << endl;
   cin >> images_subdir;
   filefunc::add_trailing_dir_slash(images_subdir);
   images_subdir="/data/ImageEngine/"+images_subdir;
   cout << "images_subdir = " << images_subdir << endl;
 
   if (!filefunc::direxist(images_subdir)) 
   {
      cout << "Images subdir=" << images_subdir << endl;
      cout << "  not found!" << endl;
      exit(-1);
   }

   string thumbnails_subdir=images_subdir+"thumbnails/";
   if (!filefunc::direxist(thumbnails_subdir)) 
   {
      cout << "Thumbnails subdir=" << thumbnails_subdir << endl;
      cout << "  not found!" << endl;
      exit(-1);
   }

// As of 4/14/12, we only insert metadata for images within the
// image_list_filename and NOT for all images within images_subdir.
// This ensures consistency between SIFT graph calculated via
// parallelized bundler on TXX and the images table within the imagery
// database!

   vector<string> image_filenames,thumbnail_filenames;
   filefunc::ReadInfile(image_list_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_image_filename=filefunc::text_line[i];
      string basename=filefunc::getbasename(curr_image_filename);
      string image_filename=images_subdir+basename;

// Note: As of 11/20/13, we experiment with forcing all thumbnails to
// be in PNG format in order to minimize rendering problems with
// Michael Yee's graph viewer:

//      string prefix=filefunc::getprefix(curr_image_filename);
//      string thumbnail_filename="thumbnail_"+prefix+".png";

      string thumbnail_filename="thumbnail_"+basename;
      
      thumbnail_filename=images_subdir+"thumbnails/"+thumbnail_filename;
//      cout << "i = " << i << " "
//           << image_filename << " "
//           << thumbnail_filename << endl;
      image_filenames.push_back(image_filename);
      thumbnail_filenames.push_back(thumbnail_filename);
   }

   int n_images = image_filenames.size();
   for (int i=0; i<n_images; i++)
   {
      outputfunc::update_progress_and_remaining_time(i, 0.05 * n_images, n_images);
      string URL=image_filenames[i];
//       if (i%200==0) cout << "Processing image " << i << endl;
//      cout << "i = " << i << " image_filename = " << URL << endl;
      string thumbnail_URL=thumbnail_filenames[i];
//      cout << "   thumbnailURL = " << thumbnail_URL << endl;

      int image_ID=i;
      int importance=1;
      unsigned int npx,npy;
      imagefunc::get_image_width_height(URL,npx,npy);
      unsigned int thumbnail_npx,thumbnail_npy;
      imagefunc::get_image_width_height(
         thumbnail_URL,thumbnail_npx,thumbnail_npy);

      imagesdatabasefunc::insert_image_metadata(
         postgis_db_ptr,campaign_ID,mission_ID,image_ID,importance,
         URL,npx,npy,thumbnail_URL,thumbnail_npx,thumbnail_npy);
   } // loop over index i labeling image filenames

}

