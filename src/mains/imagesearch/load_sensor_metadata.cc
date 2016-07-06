// ========================================================================
// Program LOAD_SENSOR_METADATA queries the user to enter campaign and
// mission IDs for a set of image package files containing
// georegistered camera calibration parameters.  LOAD_IMAGE_METADATA
// then inserts metadata into the sensor_metadata table of the images
// database table.

// 	./load_sensor_metadata --GIS_layer ./packages/imagery_metadata.pkg

// ========================================================================
// Last updated on 2/17/12; 2/19/12; 11/11/15
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

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
//   campaign_ID=1;

   cout << "Enter mission_ID:" << endl;
   cin >> mission_ID;
//   mission_ID=0;

   string bundler_IO_subdir_basename;
   cout << "Enter bundler_IO_subdir = subdir of ./bundler/ in which images & packages reside:" << endl;
   cin >> bundler_IO_subdir_basename;
   filefunc::add_trailing_dir_slash(bundler_IO_subdir_basename);

   string projects_rootdir = getenv("PROJECTSROOT");
   string bundler_dir=projects_rootdir+"/src/mains/photosynth/bundler/";
   string bundler_IO_subdir=bundler_dir+bundler_IO_subdir_basename;
   string packages_subdir=bundler_IO_subdir+"packages/";
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   cout << "packages_subdir = " << packages_subdir << endl;
   outputfunc::enter_continue_char();
   
   if (!filefunc::direxist(packages_subdir)) 
   {
      cout << "Packages subdir=" << packages_subdir << endl;
      cout << "  not found!" << endl;
      exit(-1);
   }

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("pkg");

   vector<string> packages_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,packages_subdir);

// Note added on 2/19/12:

// Sensor_ID should be pulled from IMAGERY database tables rather than
// hardwired below...

//   int sensor_ID=0;	// hand-held digital camera
   int sensor_ID=1;	// FLIR
   int status=1;

   int n_images=packages_filenames.size();
   for (int p=0; p<n_images; p++)
//   for (int p=0; p<5; p++)
   {
      string curr_package_filename=packages_filenames[p];
//      cout << "p = " << p 
//           << " curr_package_filename = " << curr_package_filename 
//           << endl;
      if (stringfunc::first_substring_location(
	curr_package_filename,"peter") > 0) continue;
      
      filefunc::ReadInfile(curr_package_filename);
      vector<string> substrings;

      string image_name=filefunc::getbasename(filefunc::text_line[0]);
      int substr_posn=stringfunc::first_substring_location(
         image_name,".rd.jpg");
      if (substr_posn > 0)
      {
         image_name=image_name.substr(0,substr_posn);
         image_name=image_name+".JPG";
      }
      
      string image_URL="/data/ImageEngine/"+bundler_IO_subdir_basename+
         image_name;

      substrings.clear();
      substrings=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[2]);
      double fu=stringfunc::string_to_number(substrings[1]);

      substrings.clear();
      substrings=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[3]);
      double fv=stringfunc::string_to_number(substrings[1]);

      substrings.clear();
      substrings=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[4]);
      double U0=stringfunc::string_to_number(substrings[1]);

      substrings.clear();
      substrings=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[5]);
      double V0=stringfunc::string_to_number(substrings[1]);

      substrings.clear();
      substrings=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[6]);
      double az=stringfunc::string_to_number(substrings[1]);

      substrings.clear();
      substrings=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[7]);
      double el=stringfunc::string_to_number(substrings[1]);

      substrings.clear();
      substrings=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[8]);
      double roll=stringfunc::string_to_number(substrings[1]);

      substrings.clear();
      substrings=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[9]);
      double X=stringfunc::string_to_number(substrings[1]);

      substrings.clear();
      substrings=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[10]);
      double Y=stringfunc::string_to_number(substrings[1]);

      substrings.clear();
      substrings=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[11]);
      double Z=stringfunc::string_to_number(substrings[1]);

      int datum_ID,image_ID,npx,npy;
      imagesdatabasefunc::retrieve_particular_image_metadata_from_database(
         postgis_db_ptr,campaign_ID,mission_ID,
         image_URL,datum_ID,image_ID,npx,npy);
//      cout << "image_URL = " << image_URL << endl;
//      cout << "datum_ID = " << datum_ID
//           << " image_ID = " << image_ID
//           << " npx = " << npx << " npy = " << npy << endl;
      
      double aspect_ratio=double(npx)/double(npy);
      double FOV_u,FOV_v;
      camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
         fu,aspect_ratio,FOV_u,FOV_v);
      FOV_u *= 180/PI;
      FOV_v *= 180/PI;

      if (p%100==0)
      {
         cout.precision(12);
         cout << "p = " << p << " of " << n_images << endl;
         cout << "image_name = " << image_name << endl;
         cout << "fu = " << fu << endl;
         cout << "fv = " << fv << endl;
         cout << "U0 = " << U0 << endl;
         cout << "V0 = " << V0 << endl;
         cout << "az = " << az << endl;
         cout << "el = " << el << endl;
         cout << "roll = " << roll << endl;
         cout << "X = " << X << endl;
         cout << "Y = " << Y << endl;
         cout << "Z = " << Z << endl;
         cout << "FOV_U = " << FOV_u << " FOV_V = " << FOV_v << endl;
         cout <<  endl;
      }
      
//      for (int i=0; i<filefunc::text_line.size(); i++)
//      {
//         cout << filefunc::text_line[i] << endl;
//      }
//      cout << endl;

      int metadata_ID=p;

      imagesdatabasefunc::insert_sensor_metadata(
         postgis_db_ptr,campaign_ID,mission_ID,sensor_ID,
         metadata_ID,image_ID,datum_ID,status,X,Y,Z,
         az,el,roll,FOV_u,FOV_v,fu,U0,V0);

   } // loop over index p labeling package filenames

}

