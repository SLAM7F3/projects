// ========================================================================
// Program UPDATE_IMAGE_TIMES queries the user to enter a graph hierarchy ID
// for a set of images already loaded into the images table of the
// IMAGERY database.  It retrieves the campaign and mission IDs plus
// UTM zonenumber corresponding to a set of images residing within
// /data/ImageEngine/ from the world_regions table of the IMAGERY
// database.  
// If FLIR_flag==true, UPDATE_IMAGE_TIMES
// extract local image times from the image filenames themselves.
// If Reuters_flag==true, UPDATE_IMAGE_TIMES extracts dates from
// article filenames. Otherwise, UPDATE_IMAGE_TIMES tries to extract
// local image times from exif metadata tags. UPDATE_IMAGE_TIMES
// subsequently converts local image times to UTC and stores timing
// metadata within the images table of the IMAGERY database.

// This program will ask your for the graph_hierarchy ID. But it's going
// to expect that the campaign and mission IDs are already properly 
// populated within those tables.

//    ./update_image_times --GIS_layer ./packages/imagery_metadata.pkg

// ========================================================================
// Added "update_gps_sql_file" 7/29/13
// Created 7/5/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <time.h>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"
#include "general/inputfuncs.h"

// ==========================================================================

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::string;
using std::vector;

/* Creates a sql file with commmands to imput GPS information into postgis_db_ptr for 
   all images in image_URLs that have GPS info. 
*/
bool update_gps_sql_file(postgis_database* postgis_db_ptr, vector<string> image_URLs, 
string images_subdir,int campaign_ID, int mission_ID)
{

   string problems;

   int noGPS = 0;

   //bool modify_IMAGERY_database_flag_global=true;
   bool modify_IMAGERY_database_flag_global=false;
   if (modify_IMAGERY_database_flag_global)
   {
      cout << "modify_IMAGERY_database_flag_global = true" << endl;
   }
   else
   {
      cout << "modify_IMAGERY_database_flag_global = false" << endl;
   }
   outputfunc::enter_continue_char();




// Open text file to hold output SQL update commands:
   string bundler_IO_subdir = "/data/bundler/";
   string output_filename=bundler_IO_subdir+images_subdir+"update_image_gps.sql";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   
// Initiate variables for image loop
  
   bool parse_exif_metadata_flag=true;

   int n_images=image_URLs.size();
   cout << endl << "n_images = " << n_images << endl;

// Image Loop	
   for (int i=0; i<=n_images; i++) 
   {
      bool modify_IMAGERY_database_flag=modify_IMAGERY_database_flag_global;
		
      string image_filename=image_URLs[i];
      if (i%100==0) {cout << "image_filename = " << image_filename << endl;}
		
      int image_ID=imagesdatabasefunc::get_image_ID(postgis_db_ptr, 
      campaign_ID, mission_ID, image_filename);

      // try to extract GPS from exif metadata tags
      photograph photo(image_filename,0,parse_exif_metadata_flag);

      geopoint loc=photo.get_geolocation();

      double lat=loc.get_latitude();
      double log=loc.get_longitude();

      //cout << "Geopoint: " << loc << endl;
		

      if (lat==0.0 and log==0.0){
         modify_IMAGERY_database_flag=false;
         noGPS=noGPS+1;
      }
      else {
         cout << "Lat: " << lat << " Long: " << log << endl;
      }	
		
      string SQL_cmd=imagesdatabasefunc::generate_update_image_metadata_SQL_command(
         campaign_ID,mission_ID,image_ID,UTC,epoch);
      outstream << SQL_cmd << endl;

      // Modify database
/*      if (modify_IMAGERY_database_flag)
        {
        imagesdatabasefunc::update_image_metadata(
        postgis_db_ptr,campaign_ID,mission_ID,image_ID);
        }*/

//       outputfunc::enter_continue_char();
		
   } // end loop over index i 
   
   cout << "-------------------------------------------------------------------------------------------------------" \
        << endl << problems << endl << "No GPS: " << noGPS << endl;

}


string update_single_gps_sql_cmd(postgis_database* postgis_db_ptr, vector<string> image_URLs, 
string images_subdir,int campaign_ID, int mission_ID)


/*
  Used to look at original files for last update time
*/
   bool make_filenames_map_from_file (map<string, string> &filenames_map) 
{
   //map<string, string> filenames_map;
   string orig_filenames_file;
   while (orig_filenames_file.empty())
   {
      orig_filenames_file=inputfunc::enter_string( \
         "enter path and filename of list of original files. ex. /home/user/Pictures/orig_names_description.txt \n OR enter 'x' to use homogenized filenames [not recommended].");
      // full path to .txt file that has standardized names matched with original filenames			
		
      if (orig_filenames_file.compare("x")==0)
      {	
         cout << "Using original filenames..." << endl;
         return false;
      }
      else if (!filefunc::fileexist(orig_filenames_file))
      {	
         orig_filenames_file="";
         cout << endl << "ERROR: File does not exist!" << endl;
         cout << "pwd: " << filefunc::get_pwd() << endl;
      }
			
      else 
      {					
         cout << endl << "reading in list of original filenames ..." << endl;
			
         // parse file and put new/old filename pairs in map
         filefunc::ReadInfile(orig_filenames_file);
         for (int i=0; i<filefunc::text_line.size(); i++)
         {
            string new_old_filename=filefunc::text_line[i];
            vector<string> new_old=stringfunc::decompose_string_into_substrings(new_old_filename, " ");				
            filenames_map[new_old[0]]=new_old[1];
         }
      }
   }
	
   return true; // filenames_map has been updated
}

// parse date-time string YYYY-MM-DD HH:MM:SS.SSSSSSSSSSSS -0x00
// and return clock object set to specified (local) time
Clock parse_date_time_string (string date_string)
{	
   string year_string=date_string.substr(0,4);
   int year=stringfunc::string_to_number(year_string);
   string month_string=date_string.substr(5,2);
   int month=stringfunc::string_to_number(month_string);
   string day_string=date_string.substr(8,2);
   int day=stringfunc::string_to_number(day_string);

   string hour_str=date_string.substr(11, 2);
   int hour=stringfunc::string_to_number(hour_str);
   string min_str=date_string.substr(14, 2);
   int minute=stringfunc::string_to_number(min_str);
   string sec_str=date_string.substr(17, 12);
   double secs=stringfunc::string_to_number(sec_str);
   string off_str=date_string.substr(29);
   int offset=stringfunc::string_to_number(off_str);
   offset=offset/100;
	

   Clock time;
   time.set_UTM_zone_time_offset(offset);
   time.set_local_time(year, month, day, hour, minute, secs); // this line SETS LOCAL time

   int new_day=time.get_day();
   int new_mon=time.get_month();
   int new_year=time.get_year();
   cout << "Year: " << new_year << " Month: " << new_mon << " Day: " << new_day << endl;


	
   //<< " Hour: " << hour << " Min: " << minute << " secs: " << secs << endl;
	

   return time;
}









