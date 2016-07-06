// ========================================================================
// Program UPDATE_IMAGE_TIMES queries the user to enter a campaign and 
// mission ID for a set of images already loaded into the images table 
// of the IMAGERY database.  We assume that the input images are *stills*
// and not video frames nor Reuters news articles. 
// It retrieves the UTM zonenumber corresponding to a set of images 
// residing within /data/ImageEngine/ from the world_regions table of the
// IMAGERY database.  
// *It assumes this set of images has been renamed, see README.imagesearch
// 
// UPDATE_IMAGE_TIMES tries to extract image times from exif metadata tags. 
// If unsuccessful, a time based upon the image file's
// access, modification or change times is assigned to the photo.
// UPDATE_IMAGE_TIMES subsequently converts local image times to UTC
// and exports a SQL script which updates timing columns within the
// images table of the IMAGERY database.

//    ./update_image_times --GIS_layer ./packages/imagery_metadata.pkg

// ========================================================================
// Last updated on 7/25/13; 10/29/13
// Major Overhaul 7/23/13
// updated on 8/27/12; 9/24/12; 2/24/13; 6/16/13,
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

#include "updateImageMetaData.cc"

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

   string problems;

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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs(); // used for database

   // Instantiate postgis database objects to send data to and retrieve
   // data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);

   // User Input for Campaign and Mission ID
   int campaign_ID, mission_ID;
   cout << "Enter Campaign ID:" << endl;
   cin >> campaign_ID;
   cout << "Enter Mission ID:" << endl;
   cin >> mission_ID;
   if (campaign_ID < 0 or mission_ID < 0) exit(-1);
   if (imagesdatabasefunc::images_in_database(postgis_db_ptr, campaign_ID, mission_ID))
      cout << "Found images with given campaign_ID = " 
           << campaign_ID << " and mission_ID = " << mission_ID << endl;

 
   // look up URLs corresponding to campaign_ID and mission_ID in database
   vector<string> image_URLs;
   if (!imagesdatabasefunc::get_image_URLs(
      campaign_ID,mission_ID,postgis_db_ptr,image_URLs))
   {
      cout << "ERROR getting image URLs." << endl;
   }
   
   string images_subdir=filefunc::getdirname(image_URLs.front());
   cout << "images_subdir = " << images_subdir << endl;	// location of images
	
//  Retrieve UTM_zonenumber from world_regions table in IMAGERY database:

   int UTM_zonenumber=imagesdatabasefunc::retrieve_campaign_UTM_zonenumber(
      postgis_db_ptr,campaign_ID);
   cout << endl << "UTM_zonenumber = " << UTM_zonenumber << endl;

   outputfunc::enter_continue_char();


// Unlike program used for SIFT graph, not inserting metadata based 
// on image_list_filename (a .dat file)
// This program inserts metadata for all images in image_URLs. 

// Open text file to hold output SQL update commands:
   string bundler_IO_subdir = "/data/bundler/";
   string output_filename=bundler_IO_subdir+
      images_subdir+"update_image_times.sql";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

// Initiate variables for image loop
  
   bool parse_exif_metadata_flag=true; 

   int n_images=image_URLs.size();
   cout << endl << "n_images = " << n_images << endl;

   bool made_map=false;
   std::map<string, string> filenames_map;
   bool use_map=true;
/*
// Testing specific images
int i=1;
while (i !=0)
{ cout << "Please enter index or 0 to stop:" << endl;
cin >> i;
*/
   for (int i=0; i<n_images; i++) // Image loop
   {
      bool modify_IMAGERY_database_flag=modify_IMAGERY_database_flag_global;
      string UTC;
      double epoch;
		
      string image_filename=image_URLs[i];
      if (i%100 == 0) {cout << "image_filename = " << image_filename << endl;	}

      int image_ID=imagesdatabasefunc::get_image_ID(
         postgis_db_ptr,campaign_ID,mission_ID,image_filename);

      // try to extract local image times from exif metadata tags
      photograph photo(image_filename,0,parse_exif_metadata_flag);

      photo.get_clock().compute_UTM_zone_time_offset(UTM_zonenumber);

      bool exif_timestamp=photo.parse_timestamp_exiftag();
      if (exif_timestamp)
      {
         epoch=photo.get_clock().secs_elapsed_since_reference_date();
      }
      if (epoch <=0.0) // can't use EXIF data, get epoch from file
      { 
         string filename=image_filename; // use new, homogenized name if told to not use map 			
         if (use_map) 			// otherwise, use map to original filename
         {
            if (!made_map){ // map of new filenames to original filenames
               use_map=make_filenames_map_from_file(filenames_map);
               made_map=true;
            }

            // find original file from the list		
            string pic_name=filefunc::getbasename(image_filename);
            map<string, string>::iterator result=filenames_map.find(pic_name);

            if (result==filenames_map.end())
            { 	cout << "Couldn't find " << pic_name << " in map. Using homogenized filename." << endl;
               filename=image_filename;
            }			
            else 
            {
               filename=result->second; // use original filename (from the map result)

               cout << "original file path for " << image_filename << " : " << filename << endl;
            }
         } // end if use_map

         epoch=filefunc::assign_file_epoch_time(filename);

      } // end of getting epoch from file

      if (epoch < 0)
      { 	
         problems.append("epoch < 0: ").append(image_filename)
            .append(" Used exif data: ").append(stringfunc::boolean_to_string(exif_timestamp))
            .append(", epoch: " ).append(stringfunc::number_to_string(epoch)).append("\n");
         modify_IMAGERY_database_flag=false;
      } // end of if zero

      photo.get_clock().convert_elapsed_secs_to_date(epoch);

      UTC=photo.get_clock().YYYY_MM_DD_H_M_S();
      int local_hour=photo.get_clock().get_local_hour();

      // Modify database
      if (modify_IMAGERY_database_flag)
      {
         bool fail=imagesdatabasefunc::update_image_metadata(
            postgis_db_ptr,campaign_ID,mission_ID,image_ID,UTC,epoch);
         if (fail){
            problems.append(" Couldn't update database for: ").append(image_filename).append("\n");
         }
      }
		
      string SQL_cmd=imagesdatabasefunc::generate_update_image_metadata_SQL_command(
         campaign_ID,mission_ID,image_ID,UTC,epoch);
      outstream << SQL_cmd << endl;
		
   } // end loop over index i 

   filefunc::closefile(output_filename, outstream);

   cout << "-------------------------------------------------------------------------------------------------------" 
        << endl << problems << endl;

   string banner="Exported "+output_filename;
   outputfunc::write_big_banner(banner);
}

