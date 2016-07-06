// ==========================================================================
// Program INTERP_FRAMES reads in reconstructed camera parameters for
// some set of video frames.  Using spline interpolation, it
// estimates camera parameters for all video frames which were not
// reconstructed by bundler.  Spline interpolation reproduces the same
// camera parameters for actual reconstructed photos.  INTERP_FRAMES
// writes out a new set of package files for all video frames.

/*

./interp_frames \
--region_filename ./bundler/lighthawk/packages/peter_inputs.pkg \
--region_filename ./bundler/lighthawk/packages/photo_0000.pkg \
--region_filename ./bundler/lighthawk/packages/photo_0001.pkg \
--region_filename ./bundler/lighthawk/packages/photo_0002.pkg \
--region_filename ./bundler/lighthawk/packages/photo_0003.pkg \
--region_filename ./bundler/lighthawk/packages/photo_0004.pkg \
--region_filename ./bundler/lighthawk/packages/photo_0005.pkg \
--region_filename ./bundler/lighthawk/packages/photo_0006.pkg \
--region_filename ./bundler/lighthawk/packages/photo_0007.pkg 

*/

// ==========================================================================
// Last updated on 1/29/11; 2/19/11; 2/22/11
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << "image_sizes_filename = " << image_sizes_filename << endl;

// First read in radial distortion parameters:

   string bundle_filename=bundler_IO_subdir+"5/bundle_5.out";
   filefunc::ReadInfile(bundle_filename);
   vector<double> column_values=stringfunc::string_to_numbers(
      filefunc::text_line[0]);

   int line_number=0;
   int n_reconstructed_images=column_values[line_number];
   cout << "n_reconstructed_images = " << n_reconstructed_images << endl;

   vector<threevector> focal_rd1_rd2;
   for (int i=0; i<n_reconstructed_images; i++)
   {
      for (int j=0; j<5; j++)
      {
         line_number++;
         if (j > 0) continue;
         column_values.clear();
         column_values=stringfunc::string_to_numbers(
            filefunc::text_line[line_number]);
         double curr_focal=column_values[0];
         double curr_rd1=column_values[1];
         double curr_rd2=column_values[2];
         focal_rd1_rd2.push_back(threevector(curr_focal,curr_rd1,curr_rd2));
      }
   } // loop over index i labeling reconstructed images

// Next read in timing values for all aerial video frames:

   string timing_filename=bundler_IO_subdir+"aircraft_gps.dat";
   
   vector<string> frame_name;
   vector<double> t_frame;
   filefunc::ReadInfile(timing_filename);

// Store frame time as function of frame filename within STRING_MAP:

   typedef map<string,double> STRING_MAP;
   STRING_MAP* frametime_map_ptr=new STRING_MAP;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> column_strings=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[i]);
      frame_name.push_back(column_strings[0]);
      t_frame.push_back(stringfunc::string_to_number(column_strings[1]));
//      cout << "i = " << i 
//           << " frame = " << frame_name.back() 
//           << " t_frame = " << t_frame.back() << endl;
      (*frametime_map_ptr)[frame_name.back()]=t_frame.back();
   }

//    outputfunc::enter_continue_char();

// Read reconstructed photographs' input packages:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   int n_reconstructed_photos=photogroup_ptr->get_n_photos();
   cout << "n_reconstructed_photos = " << n_reconstructed_photos << endl;

   int n_start=0;
   vector<double> t_reconstructed;
   vector<string> photo_filename;
   vector<threevector> world_posn;
   vector<fourvector> f_az_el_roll;
   for (int n=n_start; n<photogroup_ptr->get_n_photos(); n++)
   {
//      cout << n << "  " << flush;

      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photograph_ptr->get_camera_ptr();
      photo_filename.push_back(filefunc::getbasename(
         photograph_ptr->get_filename()));

      double f=camera_ptr->get_fu();
      double az,el,roll;
      camera_ptr->get_az_el_roll_from_Rcamera(az,el,roll);
      f_az_el_roll.push_back(fourvector(f,az,el,roll));
      world_posn.push_back(camera_ptr->get_world_posn());

      STRING_MAP::iterator string_iter=frametime_map_ptr->find(
         photo_filename.back());
      if (string_iter != frametime_map_ptr->end()) 
      {
         double curr_t(string_iter->second);
         t_reconstructed.push_back(curr_t);

         cout << "n = " << n 
              << " photo_filename = " << photo_filename.back() 
              << " t_reconstructed = " << t_reconstructed.back() 
              << " f = " << f 
              << " -focal/ydim = " 
              << -focal_rd1_rd2[n].get(0)/photograph_ptr->get_ydim()
              << endl;
      }
   } // loop over index n labeling reconstructed video frames

   cout << "world_posn.size() = " << world_posn.size() << endl;
   cout << "f_az_el_roll.size() = " << f_az_el_roll.size() << endl;

   vector<threevector> interp_world_posn;
   mathfunc::spline_interp(
      t_reconstructed,world_posn,
      t_frame,interp_world_posn);

   vector<threevector> interp_focal_rd1_rd2;
   mathfunc::spline_interp(
      t_reconstructed,focal_rd1_rd2,
      t_frame,interp_focal_rd1_rd2);

   vector<fourvector> interp_f_az_el_roll;
   mathfunc::spline_interp(
      t_reconstructed,f_az_el_roll,
      t_frame,interp_f_az_el_roll);

// Write out interpolated bundler file for used in undistorting all
// video frames via Noah's RadialUndistort script:

   string fake_bundler_filename=bundler_IO_subdir+"interp_bundle.out";
   ofstream outstream;   
   outstream.precision(12);
   filefunc::openfile(fake_bundler_filename,outstream);

// On 2/22/11, we learned the hard and painful way that bundle.out
//  must contain the following first line!

   outstream << "# Bundle file v0.3" << endl;
   outstream << interp_focal_rd1_rd2.size() << " 0 " << endl;

   for (int i=0; i<interp_focal_rd1_rd2.size(); i++)
   {
      double curr_focal=interp_focal_rd1_rd2[i].get(0);
      double curr_rd1=interp_focal_rd1_rd2[i].get(1);
      double curr_rd2=interp_focal_rd1_rd2[i].get(2);
      outstream << curr_focal << "  " << curr_rd1 << "  " << curr_rd2 << endl;

// Insert fictitious identity for rotation matrix and zero translation
// vector in output bundle file:

      outstream << "1.0   0.0   0.0" << endl;
      outstream << "0.0   1.0   0.0" << endl;
      outstream << "0.0   0.0   1.0" << endl;
      outstream << "0.0   0.0   0.0" << endl;
   }
   filefunc::closefile(fake_bundler_filename,outstream);   

// Write out interpolated set of package files for all video
// frames:

   for (int i=0; i<t_frame.size(); i++)
   {
      string package_filename=bundler_IO_subdir+"interp_packages/photo_"+
         stringfunc::integer_to_string(i,4)+".pkg";
//      cout << package_filename << endl;
      filefunc::openfile(package_filename,outstream);

      outstream << bundler_IO_subdir+"images/"+frame_name[i] << endl;
      outstream << "--photo_ID " << i << endl;
      outstream << "--Uaxis_focal_length " << interp_f_az_el_roll[i].get(0) 
                << endl;
      outstream << "--Vaxis_focal_length " << interp_f_az_el_roll[i].get(0) 
                << endl;
      outstream << "--U0 0.75047" << endl;
      outstream << "--V0 0.5" << endl;
      outstream << "--relative_az " << interp_f_az_el_roll[i].get(1)*180/PI 
                << endl;
      outstream << "--relative_el " << interp_f_az_el_roll[i].get(2)*180/PI 
                << endl;
      outstream << "--relative_roll " << interp_f_az_el_roll[i].get(3)*180/PI 
           << endl;
      outstream << "--camera_x_posn " << interp_world_posn[i].get(0) << endl;
      outstream << "--camera_y_posn " << interp_world_posn[i].get(1) << endl;
      outstream << "--camera_z_posn " << interp_world_posn[i].get(2) << endl;
      outstream << "--frustum_sidelength 100" << endl;
      outstream << "--downrange_distance -1" << endl;
      filefunc::closefile(package_filename,outstream);

   } // loop over index i labeling all video frames
   
}
