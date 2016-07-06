// ========================================================================
// Program UPDATE_IMAGE_TIMES queries the user to enter a graph hierarchy ID
// for a set of images already loaded into the images table of the
// IMAGERY database.  It retrieves the campaign and mission IDs plus
// UTM zonenumber corresponding to a set of images residing within
// /data/ImageEngine/ from the world_regions table of the IMAGERY
// database.  If Newswrap_flag==true, UPDATE_IMAGE_TIMES bases video
// times upon frame number.  If FLIR_flag==true, UPDATE_IMAGE_TIMES
// extract local image times from the image filenames themselves.
// If Reuters_flag==true, UPDATE_IMAGE_TIMES extracts dates from
// article filenames. Otherwise, UPDATE_IMAGE_TIMES tries to extract
// local image times from exif metadata tags. UPDATE_IMAGE_TIMES
// subsequently converts local image times to UTC and stores timing
// metadata within the images table of the IMAGERY database.

// This program will ask for the graph_hierarchy ID. But it's going
// to expect that the campaign and mission IDs are already properly 
// populated within those tables.

//    ./update_image_times --GIS_layer ./packages/imagery_metadata.pkg

// ========================================================================
// Last updated on 8/27/12; 9/24/12; 2/24/13
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

   bool modify_IMAGERY_database_flag=true;
//   bool modify_IMAGERY_database_flag=false;
   if (modify_IMAGERY_database_flag)
   {
      cout << "modify_IMAGERY_database_flag = true" << endl;
   }
   else
   {
      cout << "modify_IMAGERY_database_flag = false" << endl;
   }
   outputfunc::enter_continue_char();

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
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
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

//   int hierarchy_ID=2;			// Grand Canyon
//   int hierarchy_ID=3;	       		// MIT32K
//   int hierarchy_ID=4;			// NewsWrap
//   int hierarchy_ID=5;			// Tstorm
   int hierarchy_ID=9;				// Reuters 43K
   cout << "Enter ID for graph hierarchy:" << endl;
   cin >> hierarchy_ID;
   if (hierarchy_ID < 0) exit(-1);

   int n_graphs,n_levels,n_connected_components;
   string hierarchy_description;
   graphdbfunc::retrieve_graph_hierarchy_metadata_from_database(
      postgis_db_ptr,hierarchy_ID,hierarchy_description,n_graphs,n_levels,
      n_connected_components);
   cout << "n_connected_components = " << n_connected_components
        << endl;
   
   int campaign_ID,mission_ID;
   imagesdatabasefunc::get_campaign_mission_IDs(
      postgis_db_ptr,hierarchy_ID,campaign_ID,mission_ID);
// Note:  campaign_ID=2, mission_ID=0 for Grand Canyon

   cout << "campaign_ID = " << campaign_ID
        << " mission_ID = " << mission_ID << endl;

   int graph_ID=0;
   vector<string> image_URLs;
   imagesdatabasefunc::get_image_URLs(
      postgis_db_ptr,hierarchy_ID,graph_ID,image_URLs);
   string images_subdir=filefunc::getdirname(image_URLs.front());
   cout << "images_subdir = " << images_subdir << endl;

   if (!filefunc::direxist(images_subdir)) 
   {
      cout << "Images subdir=" << images_subdir << endl;
      cout << "  not found!" << endl;
      exit(-1);
   }

// As of 4/14/12, we only insert metadata for images within the
// image_list_filename and NOT for all images within images_subdir.
// This ensures consistency between SIFT graph calculated via
// parallelized bundler on TXX and the images table within the imagery
// database!

   vector<string> image_filenames;
   filefunc::ReadInfile(image_list_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      string curr_image_filename=filefunc::text_line[i];
      string basename=filefunc::getbasename(curr_image_filename);
      string image_filename=images_subdir+basename;
      cout << "i = " << i 
           << " " << image_filename << endl;
      image_filenames.push_back(image_filename);
   }

//  Retrieve UTM_zonenumber from world_regions table in IMAGERY database:

   int UTM_zonenumber=imagesdatabasefunc::retrieve_campaign_UTM_zonenumber(
      postgis_db_ptr,campaign_ID);
   cout << "UTM_zonenumber = " << UTM_zonenumber << endl;
//   outputfunc::enter_continue_char();

   bool Reuters_flag=true;
//   bool Reuters_flag=false;

   bool FLIR_flag=false;
//   bool FLIR_flag=true;

   bool NewsWrap_flag=false; // "video" flag
   if (hierarchy_ID==4) NewsWrap_flag=true;
   double epoch,start_epoch;
   if (NewsWrap_flag)
   {
      Clock clock;
      clock.compute_UTM_zone_time_offset(UTM_zonenumber);
      clock.set_local_time(2011,6,15,18,0,0);	// NewsWrap from June 15, 2011
      start_epoch=clock.secs_elapsed_since_reference_date();
      epoch=start_epoch-0.2;
   }

   bool parse_exif_metadata_flag=true;
   if (Reuters_flag || FLIR_flag || NewsWrap_flag)
   {
      parse_exif_metadata_flag=false;
   }


   string UTC;
   int n_images=image_filenames.size();
   cout << "n_images = " << n_images << endl;
   for (int i=0; i<n_images; i++)
   {
      int image_ID=i;
      photograph photo(image_filenames[i],0,parse_exif_metadata_flag);
      photo.get_clock().compute_UTM_zone_time_offset(UTM_zonenumber);
//         cout << "image_filename = " << image_filenames[i] << endl;
      string basename=filefunc::getbasename(image_filenames[i]);
//         cout << "basename = " << basename << endl;

      if (NewsWrap_flag)
      {
         epoch += 0.2;
         photo.get_clock().convert_elapsed_secs_to_date(epoch);
         UTC=photo.get_clock().YYYY_MM_DD_H_M_S();
      }
      else if (FLIR_flag)
      {
         int posn=stringfunc::first_substring_location(basename,".jpg");
         string date_time_str=basename.substr(0,posn);
//         cout << "date_time_str = " << date_time_str << endl;
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               date_time_str,"_");
         string date_str=substrings[0];
         string time_str=substrings[1];
         
//         cout << "date = " << substrings[0]
//              << " time = " << substrings[1] << endl;
         string year_str=date_str.substr(0,4);
         int year=stringfunc::string_to_number(year_str);
         string month_str=date_str.substr(4,2);
         int month=stringfunc::string_to_number(month_str);
         string day_str=date_str.substr(6,2);
         int day=stringfunc::string_to_number(day_str);

         string hour_str=time_str.substr(0,2);
         int hour=stringfunc::string_to_number(hour_str);
         string minute_str=time_str.substr(2,2);
         int minute=stringfunc::string_to_number(minute_str);
         string seconds_str=time_str.substr(4,6);
         double seconds=stringfunc::string_to_number(seconds_str);

         int local_hour=hour-3;	// local Tucson time for Tstorm-4

         photo.get_clock().set_local_time(
            year,month,day,local_hour,minute,seconds);
         int UTC_hour=photo.get_clock().local_to_UTC_hour(local_hour);
         UTC=photo.get_clock().YYYY_MM_DD_H_M_S();
         epoch=photo.get_clock().secs_elapsed_since_reference_date();

//         cout << "year = " << year << " month = " << month
//              << " day = " << day << endl;
//         cout << "hour = " << hour << " minute = " << minute
//              << " seconds = " << seconds << endl;
//         cout << "UTC_hour = " << UTC_hour << endl;
//         cout << "local_hour = " << local_hour << endl;
//         cout << "UTC = " << UTC << endl;
//         cout << endl;
      }
      else if (Reuters_flag)
      {
         string basename=filefunc::getbasename(image_filenames[i]);
//         cout << "file basename = " << basename << endl;
         string prefix=stringfunc::prefix(basename);         
         int prefix_length=prefix.size();
         int date_posn=prefix.size()-8;
         string date_string=prefix.substr(date_posn,8);
//         cout << "date_string = " << date_string << endl;
         string year_string=date_string.substr(0,4);
         int year=stringfunc::string_to_number(year_string);
         string month_string=date_string.substr(4,2);
         int month=stringfunc::string_to_number(month_string);
         string day_string=date_string.substr(6,2);
         int day=stringfunc::string_to_number(day_string);
         
         int hour=0;
         int minute=0;
         double sec=0;
         photo.get_clock().set_UTC_time(
            year,month,day,hour,minute,sec);
         UTC=photo.get_clock().YYYY_MM_DD_H_M_S();
         epoch=photo.get_clock().secs_elapsed_since_reference_date();
         
//         cout << "year = " << year << " month = " << month
//              << " day = " << day << endl;
//         cout << "UTC = " << UTC << endl;
//         cout << endl;
      }
      else
      {
         photo.parse_timestamp_exiftag();

         int local_hour=photo.get_clock().get_local_hour();
         int UTC_hour=photo.get_clock().local_to_UTC_hour(local_hour);
//      cout << "UTC_hour = " << UTC_hour << endl;
         cout << "local_hour = " << local_hour << endl;

         UTC=photo.get_clock().YYYY_MM_DD_H_M_S();
         epoch=photo.get_clock().secs_elapsed_since_reference_date();
      } // FLIR_flag conditional

//      if (i%100==0)
      {
         cout << i << " of " << n_images << endl;
         cout.precision(12);
         cout << "image ID=" << image_ID
              << " URL= "  << image_filenames[i] << endl;
         cout << "    UTC=" << UTC 
              <<  " epoch=" << epoch << endl << endl;
      }

      if (modify_IMAGERY_database_flag)
      {
         imagesdatabasefunc::update_image_metadata(
            postgis_db_ptr,campaign_ID,mission_ID,image_ID,UTC,epoch);
      }

//       outputfunc::enter_continue_char();

   } // loop over index i labeling image filenames
   

}

