// ========================================================================
// Program ARCHIVE_CAMERA_PARAMS queries the user for a plume mission
// number.  It also asks if the 10 cameras were video or SLR sensors.
// ARCHIVE_CAMERA_PARAMS exports the reconstructed and correctly
// scaled tripod camera parameters to a user-friendly text file.  It
// also inserts or updates rows within the sensors table of the plume
// database with intrinsic and georegistered extrinsic camera
// parameter information.
// ========================================================================
// Last updated on 4/24/12; 4/27/12; 1/7/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "postgres/plumedatabasefuncs.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"

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
   PassesGroup passes_group(&arguments);
   string image_list_filename=passes_group.get_image_list_filename();
   cout << " image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

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

   int mission_ID=11;
   cout << "Enter mission ID for current plume experiment:" << endl;
   cout << "e.g. mission ID = 4 for Experiment 2B" << endl;
   cout << "Reminder:  Use PGADMIN3 to look at contents of missions table within PLUME database" << endl;
   cin >> mission_ID;

   string camera_type_string;
   cout << "Enter 'v' ['s'] for video [SLR] camera type:" << endl;
   cin >> camera_type_string;

   bool video_cameras_flag=false;
   if (camera_type_string=="v")
   {
      video_cameras_flag=true;
   }

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
//   cout << "photogroup_ptr = " << photogroup_ptr << endl;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   string output_filename=bundler_IO_subdir+"SLR_camera_params.txt";
   if (video_cameras_flag)
      output_filename=bundler_IO_subdir+"video_camera_params.txt";

   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   
   for (int n=0; n<n_photos; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
//      cout << "n = " << n << " *photo_ptr = " << *photo_ptr << endl;
      camera* camera_ptr=photo_ptr->get_camera_ptr();
      cout << "n = " << n
           << " camera = " << *camera_ptr << endl;

      int video_camera_ID=1+n;
      int SLR_camera_ID=17+n;

      int camera_ID=SLR_camera_ID;
      if (video_cameras_flag) camera_ID=video_camera_ID;

      cout << "n = " << n << " camera_ID = " << camera_ID << endl;

      int status_flag=1;
      camera_ptr->print_params(camera_ID,outstream);

      double az,el,roll;
      camera_ptr->get_az_el_roll_from_Rcamera(az,el,roll);
      az *= 180/PI;
      el *= 180/PI;
      roll *= 180/PI;
      threevector camera_posn=camera_ptr->get_world_posn();

      if (plumedatabasefunc::retrieve_camera_metadata_from_database(
         postgis_db_ptr,mission_ID,camera_ID))
      {
         bool update_flag=plumedatabasefunc::update_camera_metadata(
            postgis_db_ptr,camera_ID,mission_ID,
            camera_ptr->get_fu(),
            camera_ptr->get_u0(),
            camera_ptr->get_v0(),
            camera_ptr->get_rel_az()*180/PI,
            camera_ptr->get_rel_el()*180/PI,
            camera_ptr->get_rel_roll()*180/PI,
            camera_posn.get(0),
            camera_posn.get(1),
            camera_posn.get(2));
         cout << "update_flag = " << update_flag << endl;
      }
      else
      {
         bool insert_flag=plumedatabasefunc::insert_camera_metadata(
            postgis_db_ptr,camera_ID,mission_ID,status_flag,
            camera_ptr->get_fu(),camera_ptr->get_u0(),camera_ptr->get_v0(),
            az,el,roll,camera_posn);
         cout << "insert_flag = " << insert_flag << endl;
      }
   }

   filefunc::closefile(output_filename,outstream);   

   string banner="Wrote camera parameters to "+output_filename;
   outputfunc::write_big_banner(banner);

   banner="Entered camera parameters into sensors table in PLUME database";
   outputfunc::write_big_banner(banner);
}
