// ========================================================================
// Program UPDATE_REL_FRAME_TIMES queries the user to enter campaign
// and mission IDs for a set of video frames already loaded into the
// images table of the IMAGERY database.  It retrieves the UTM
// zonenumber corresponding to a set of images residing within
// /data/ImageEngine/ from the world_regions table of the IMAGERY
// database.  UPDATE_REL_FRAME_TIMES assumes the current video clip
// began at midnight, and it bases video frame times upon frame
// number.  UPDATE_REL_FRAME_TIMES exports a series of SQL commands
// which update timing metadata within the images table of the IMAGERY
// database.

//    ./update_rel_frame_times --GIS_layer ./packages/imagery_metadata.pkg

// ========================================================================
// Last updated on 10/29/13; 10/30/13; 12/21/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "math/ltduple.h"
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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

   string image_list_filename=passes_group.get_image_list_filename();
   cout << " image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string sql_filename=bundler_IO_subdir+"update_image_times.sql";
   ofstream sql_stream;
   filefunc::openfile(sql_filename,sql_stream);
   
// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   int campaign_ID,mission_ID;
   cout << "Enter campaign ID for video corpus:" << endl;
   cin >> campaign_ID;
   cout << "Enter mission ID for video corpus:" << endl;
   cin >> mission_ID;

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

//   double delta_t=0.5;    // Default time between successive video frames
   double delta_t=1.0;	    // Default time between successive video frames

   Clock clock;
   clock.compute_UTM_zone_time_offset(UTM_zonenumber);
   clock.set_local_time(2013,4,15,0,0,0);	// midnight Patriots' Day
//   clock.set_local_time(2013,9,18,0,0,0);	// midnight 18 Sep 2013
   double start_epoch=clock.secs_elapsed_since_reference_date();
   double epoch=start_epoch-delta_t;

   string banner="Assumed starting time for all video clips = "+
      clock.YYYY_MM_DD_H_M_S();
   outputfunc::write_big_banner(banner);
   outputfunc::enter_continue_char();

// As of 10/29/13, we assume image URLs have forms like
// clip_0000_frame-00001.jpg:

   int n_images=image_URLs.size();
   cout << "n_images = " << n_images << endl;
   for (int i=0; i<n_images; i++)
   {
      outputfunc::update_progress_fraction(i,100,n_images);    
      
      vector<string> substrings=
         stringfunc::decompose_string_into_substrings(
            filefunc::getbasename(image_URLs[i])," _-");
//      for (int s=0; s<substrings.size(); s++)
//      {
//         cout << "i = " << i << " s = " << s << " substring[s] = " 
//              << substrings[s] << endl;
//      }

//      int clip_ID=-1;
      int frame_ID=-1;
      if (substrings.size()==4)
      {
         if (substrings[0]=="clip")
         {
//            clip_ID=stringfunc::string_to_number(substrings[1]);
            frame_ID=stringfunc::string_to_number(substrings[3]);
         }
      }

      epoch=start_epoch+frame_ID*delta_t;
      clock.convert_elapsed_secs_to_date(epoch);
      string UTC=clock.YYYY_MM_DD_H_M_S();

//      cout << i << " of " << n_images << endl;
//      cout.precision(15);
//      cout << " URL= "  << image_URLs[i] << endl;
//      cout << "    UTC=" << UTC  << endl;
//      cout <<  " epoch=" << epoch << endl << endl;

      int image_ID=imagesdatabasefunc::get_image_ID(
         postgis_db_ptr,campaign_ID,mission_ID,image_URLs[i]);
      string curr_update_command=
         imagesdatabasefunc::generate_update_image_metadata_SQL_command(
            campaign_ID,mission_ID,image_ID,UTC,epoch);
      sql_stream << curr_update_command << endl;

   } // loop over index i labeling image filenames
   
   filefunc::closefile(sql_filename,sql_stream);

   banner="Exported update image time SQL commands to "+sql_filename;
   outputfunc::write_big_banner(banner);
}

