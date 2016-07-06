// ========================================================================
// Program GENERATE_MONTAGE_AVIS loops over "t>0" images for the 10
// SLR or video tripod cameras.  For each camera, it retrieves all
// non-time averaged colored mask jpeg files exported by program
// TEMPORALLY_FILTER_MASKS in temporal order. This program forms an
// mpeg4 AVI movie which shows the raw tripod camera's view on the
// left-hand side and the simultaneous corresponding smoke mask on the
// right-hand side.

// The 10 output montage AVI files are output to subdirectory
// "./AVIs/".

//    ./generate_montage_AVIs --GIS_layer ./packages/plume_metadata.pkg 

// ========================================================================
// Last updated on 1/14/13; 1/25/13; 1/27/13; 1/28/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "postgres/plumedatabasefuncs.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

#include "general/outputfuncs.h"

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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   int mission_ID=22;
//   cout << "Enter mission ID:" << endl;
//   cin >> mission_ID;

   int fieldtest_ID=plumedatabasefunc::retrieve_fieldtest_ID_given_mission_ID(
      postgis_db_ptr,mission_ID);
//   cout << "fieldtest_ID = " << fieldtest_ID << endl;

   string tmp_dirname="./tmp/";
   filefunc::dircreate(tmp_dirname);
   string finished_movie_subdir="./AVIs/";
   filefunc::dircreate(finished_movie_subdir);

   int start_camera_ID=17;
   int stop_camera_ID=26;
   if (fieldtest_ID==2)
   {
//      start_camera_ID=1;
      start_camera_ID=2;
      stop_camera_ID=10;
   }

   for (int camera_ID=start_camera_ID; camera_ID<=stop_camera_ID; camera_ID++)
   {
      vector<string> URLs=
         plumedatabasefunc::retrieve_photo_URLs_from_database(
            postgis_db_ptr,fieldtest_ID,mission_ID,camera_ID);
      vector<string> mask_URLs=
         plumedatabasefunc::retrieve_mask_URLs_from_database(
            postgis_db_ptr,fieldtest_ID,mission_ID,camera_ID);

      filefunc::purge_files_in_subdir(tmp_dirname);
      cout << endl;

      int n_timeslices=URLs.size();
      for (int m=1; m<n_timeslices; m++)
      {
         string photo_filename=URLs[m];
         string mask_filename=mask_URLs[m];
//         cout << "mask = " << mask_filename << endl;

         string dirname=filefunc::getdirname(mask_filename);
         string basename=filefunc::getbasename(mask_filename);
//         cout << "basename = " << basename << endl;
         string prefix=stringfunc::prefix(basename);
         string suffix=stringfunc::suffix(basename);
         if (suffix=="bin")
         {
            basename += ".jpg";
         }

         string colored_mask_filename=dirname+"Colored_TimeAvgd_0_"+
            basename;
         string fused_mask_filename=dirname+"fused_classification_"+
            prefix+".jpg";
//       cout << "fused_mask_filename = " << fused_mask_filename << endl;

         if (!filefunc::fileexist(colored_mask_filename)) 
         {
            cout << "timeslice ID = " << m << " " 
                 << colored_mask_filename << " not found" << endl;
            continue;
         }
         
         string montage_filename="montage_"+basename;
         cout << "Processing montage " << m << " of " << URLs.size()
              << " for tripod camera " << camera_ID << endl;

         string unix_cmd="montage -mode concatenate "+
            photo_filename+" "+fused_mask_filename+" "+montage_filename;
//         string unix_cmd="montage -mode concatenate "+
//            photo_filename+" "+colored_mask_filename+" "+montage_filename;
//         string unix_cmd="montage -mode concatenate "+
//            photo_filename+" "+fused_mask_filename+" "
//            +colored_mask_filename+" "+montage_filename;
         sysfunc::unix_command(unix_cmd);

         unix_cmd="convert "+montage_filename+" -resize 50% "+montage_filename;
//         unix_cmd="convert "+montage_filename+" -resize 50% "+montage_filename;
//         unix_cmd="convert "+montage_filename+" -resize 40% "+montage_filename;
         sysfunc::unix_command(unix_cmd);

         unix_cmd="mv "+montage_filename+" "+tmp_dirname;
         sysfunc::unix_command(unix_cmd);

/*
//         cout << "m = " << m << " URL = " << URL << endl;
         string colored_mask_filename=dirname+"Colored_TimeAvgd_2_"+
            basename;
         cout << "m = " << m << " colored mask filename = "
              << colored_mask_filename << endl;
         string unix_cmd="cp "+colored_mask_filename+" "+tmp_dirname;
         sysfunc::unix_command(unix_cmd);
*/
      } // loop over index m labeling URLs
      cout << endl;

      string video_codec="msmpeg4v2";
      string input_image_subdir;
//      string image_suffix="JPG";
      string image_suffix="jpg";

      double fps=5;
      if (fieldtest_ID >= 2)
      {
         fps=20;
      }

      string output_movie_filename_prefix=
         "montage2_"+stringfunc::number_to_string(camera_ID);
//         "montage3_"+stringfunc::number_to_string(camera_ID);
//         "video_"+stringfunc::number_to_string(camera_ID);
//      string output_movie_filename_prefix="mask_"+stringfunc::number_to_string(
//         camera_ID);

      string AVI_movie_filename=videofunc::generate_AVI_movie(
         video_codec,tmp_dirname,image_suffix,fps,
         output_movie_filename_prefix,finished_movie_subdir);

   } // loop over camera ID

   string banner="Montage AVI movies written to "+finished_movie_subdir;
   outputfunc::write_big_banner(banner);
}
