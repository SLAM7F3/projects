// ========================================================================
// Program UPDATE_PICTURE_TIMES queries the user to enter campaign and
// mission IDs for some set of images already loaded into the images
// table of the IMAGERY database.  We assume that the input images are
// *stills* and not video frames nor Reuters news
// articles. UPDATE_PICTURE_TIMES retrieves the UTM zonenumber
// corresponding to a set of images residing within /data/ImageEngine/
// from the world_regions table of the IMAGERY database.  

// UPDATE_PICTURE_TIMES tries to extract local image times from exif
// metadata tags.  If unsuccessful, a time based upon the image file's
// access, modification or change times is assigned to the photo.
// (If an exif time precedes 1 Jan 1990, we also replace its value with
// that coming from an image file time.)  UPDATE_PICTURE_TIMES
// subsequently converts local image times to UTC and exports a SQL
// script which updates timing columns within the images table of the
// IMAGERY database.

// UPDATE_PICTURE_TIMES does NOT depend upon any SIFT graph
// information.  So it can be run immediately after
// LOAD_IMAGE_METADATA.

// ========================================================================
// Last updated on 7/23/13; 7/31/13; 8/1/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "graphs/graphdbfuncs.h"
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

//   bool modify_IMAGERY_database_flag=true;
   bool modify_IMAGERY_database_flag=false;
   if (modify_IMAGERY_database_flag)
   {
      cout << "modify_IMAGERY_database_flag = true" << endl;
   }
   else
   {
      cout << "modify_IMAGERY_database_flag = false" << endl;
   }
//   outputfunc::enter_continue_char();

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

   string image_list_filename=passes_group.get_image_list_filename();
   cout << " image_list_filename = " << image_list_filename << endl;
//   string image_sizes_filename=passes_group.get_image_sizes_filename();
//   cout << " image_sizes_filename = " << image_sizes_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
   
   int campaign_ID,mission_ID;
   cout << "Enter campaign ID:" << endl;
   cin >> campaign_ID;
   cout << "Enter mission ID:" << endl;
   cin >> mission_ID;
   timefunc::initialize_timeofday_clock();      

   vector<string> image_URLs;
   imagesdatabasefunc::get_image_URLs(
      campaign_ID,mission_ID,postgis_db_ptr,image_URLs);
   string images_subdir=filefunc::getdirname(image_URLs.front());
   cout << "images_subdir = " << images_subdir << endl;

   if (!filefunc::direxist(images_subdir)) 
   {
      cout << "Images subdir=" << images_subdir << endl;
      cout << "  not found!" << endl;
      exit(-1);
   }

//  Retrieve UTM_zonenumber from world_regions table in IMAGERY database:

   int UTM_zonenumber=imagesdatabasefunc::retrieve_campaign_UTM_zonenumber(
      postgis_db_ptr,campaign_ID);
   cout << "UTM_zonenumber = " << UTM_zonenumber << endl;
//   outputfunc::enter_continue_char();

// Open text file to hold output SQL update commands:

   string output_filename=bundler_IO_subdir+"update_picture_times.sql";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

// Compute epoch corresponding to midnight 1 Jan 1990:

   Clock clock;
   int year=1990;
   int month=1;
   int day=1;
   int hour=0;
   int minute=0;
   double sec=0;
   clock.set_UTC_time(year,month,day,hour,minute,sec);
   double epoch_1990=clock.secs_elapsed_since_reference_date(); 

   double epoch;
   int n_images=image_URLs.size();
   for (int i=0; i<n_images; i++)
   {
      cout << "Processing photo " << i << " of " << n_images << endl;

      string photo_URL=image_URLs[i];
      bool parse_exif_metadata_flag=true;
      photograph photo(photo_URL,0,parse_exif_metadata_flag);
      photo.get_clock().compute_UTM_zone_time_offset(UTM_zonenumber);

      int image_ID=imagesdatabasefunc::get_image_ID(
         postgis_db_ptr,campaign_ID,mission_ID,photo_URL);

      if (!photo.parse_timestamp_exiftag())
      {
         cout << "  Assigning photo time based upon file information" << endl;
         epoch=filefunc::assign_file_epoch_time(photo_URL);
      }
      else
      {
         epoch=photo.get_clock().secs_elapsed_since_reference_date();
      }

// As of 8/1/13, we have empirically observed that some EXIF metadata
// times can be completely bogus.  So as a sanity check, we compare
// any EXIF time with 1 Jan 1990.  If the former is less than the
// latter, we replace the EXIF time with a file modification
// timestamp:
      
      if (epoch < epoch_1990)
      {
         epoch=filefunc::assign_file_epoch_time(photo_URL);
      }

      photo.get_clock().convert_elapsed_secs_to_date(epoch);

      string UTC=photo.get_clock().YYYY_MM_DD_H_M_S();
      int local_hour=photo.get_clock().get_local_hour();

      cout << "  Photo ID=" << image_ID 
           << " URL= "  << photo_URL << endl;
      cout << "  UTC=" << UTC 
           << " local hour=" << local_hour
           <<  " epoch=" << epoch << endl << endl;

      if (modify_IMAGERY_database_flag)
      {
         imagesdatabasefunc::update_image_metadata(
            postgis_db_ptr,campaign_ID,mission_ID,image_ID,UTC,epoch);
      }

      string SQL_cmd=
         imagesdatabasefunc::generate_update_image_metadata_SQL_command(
            campaign_ID,mission_ID,image_ID,UTC,epoch);
      outstream << SQL_cmd << endl;
      
   } // loop over index i labeling image filenames

   filefunc::closefile(output_filename,outstream);

   string banner="Exported "+output_filename;
   outputfunc::write_big_banner(banner);

   cout << "At end of program UPDATE_PICTURE_TIMES" << endl;
   outputfunc::print_elapsed_time();
}

