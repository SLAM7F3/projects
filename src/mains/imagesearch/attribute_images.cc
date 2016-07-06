// ========================================================================
// Program ATTRIBUTE_IMAGES takes in campaign and mission IDs.  It
// then queries the user to enter an image filename substring to
// search for within all URLs matching the specified campaign and
// mission.  ATTRIBUTE_IMAGES fills in rows within the
// image_attributes table of the IMAGERY database corresponding to the
// retrieved URLS with key-value pairs specified by the user.
// ========================================================================
// Last updated on 2/10/12; 2/26/12; 4/16/13
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
   PassesGroup passes_group(&arguments);

//   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
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

// campaign 2, mission 0: GC:  aerial vs ground
// campaign 1, mission 2: MIT32K:  aerial vs ground

// campaign 3, mission 0: Tstorm   EO vs IR
// campaign 3, mission 0: Tstorm   HFOV

   int campaign_ID,mission_ID;
   cout << "Enter campaign_ID:" << endl;
   cin >> campaign_ID;

   cout << "Enter mission_ID:" << endl;
   cin >> mission_ID;

   vector<int> image_IDs,datum_IDs;
   vector<string> image_URLs;
   imagesdatabasefunc::retrieve_image_metadata_from_database(
      postgis_db_ptr,campaign_ID,mission_ID,image_IDs,datum_IDs,image_URLs);

   string filename_substring,key,value,value_type;
   cout << "Enter image filename substring:" << endl;
   cin >> filename_substring;

   cout << "Enter attribute key" << endl;
   cin >> key;

// Make sure key exists within image_attribute_metadata table:

   if (!imagesdatabasefunc::image_attribute_key_exists(postgis_db_ptr,key))
   {
      cout << "Error: Key doesn't exist within image_attribute_metadata table" << endl;
      cout << "  of IMAGERY database!" << endl;
      exit(-1);
   }
   
   cout << "Enter attribute value:" << endl;
   cin >> value;
//   cout << "Enter anti-attribute value:" << endl;
//   cin >> value;

// Make sure value exists within image_attribute_metadata table

   if (!imagesdatabasefunc::image_attribute_value_exists(postgis_db_ptr,value))
   {
      cout << "Error: Value doesn't exist within image_attribute_metadata table" << endl;
      cout << "  of IMAGERY database!" << endl;
      exit(-1);
   }

   for (unsigned int i=0; i<image_URLs.size(); i++)
   {
      string curr_URL=image_URLs[i];
      string basename=filefunc::getbasename(curr_URL);
      int substr_posn=stringfunc::first_substring_location(
         basename,filename_substring);
      if (substr_posn < 0) continue;	// URL does NOT match filename_substr
//      if (substr_posn >= 0) continue; // URL does match filename_substr
      
      cout << "i = " << i << " URL = " << curr_URL << endl;

      if (modify_IMAGERY_database_flag)
      {
         imagesdatabasefunc::insert_image_attribute(
            postgis_db_ptr,campaign_ID,mission_ID,image_IDs[i],datum_IDs[i],
            key,value);
      }
   } // loop over index i labeling image URLs

}

