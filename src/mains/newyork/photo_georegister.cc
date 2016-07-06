// ==========================================================================
// Program PHOTO_GEOREGISTER writes output package files containing
// pin hole model parameters needed to visualize reconstructed photos
// as OBSFRUSTA.

//				photo_georegister

// ==========================================================================
// Last updated on 6/23/09; 6/24/09; 12/16/09
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "video/photograph.h"
#include "video/photogroup.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

// Read in Noah's original set of photos and reconstructed cameras:

//   string subdir=
//      "/home/cho/programs/c++/svn/projects/src/mains/newyork/bundler/Manhattan/";
//   string image_list_filename=subdir+"list.compressed.txt";
//   string bundle_compressed_filename=subdir+"bundle.compressed.out";

   string subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/newyork/bundler/nyc_1000/Manhattan1012Compressed/";
   string image_list_filename=subdir+"Manhattan.1012.txt";
   string bundle_compressed_filename=subdir+"Manhattan.1012.out";

   photogroup* bundler_photogroup_ptr=new photogroup();

   int n_photos_to_reconstruct=-1;
//   int n_photos_to_reconstruct=4;
   bundler_photogroup_ptr->reconstruct_bundler_cameras(
      subdir,image_list_filename,bundle_compressed_filename,
      n_photos_to_reconstruct);

   photogroup* photogroup_ptr=new photogroup(*bundler_photogroup_ptr);
   for (int n=0; n<photogroup_ptr->get_n_photos(); n++)
   {
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photograph_ptr->get_camera_ptr();

/*
      double fitted_nyc_to_bundler_distance_ratio = 466.301280029;
      threevector fitted_bundler_trans( 
         583552.414049, 4508737.128, 133.15476035);
      double global_az = 108.779639832*PI/180;
      double global_el = -1.11411987448*PI/180;
      double global_roll = -0.387659671875*PI/180;
*/

/*
      double fitted_nyc_to_bundler_distance_ratio = 465.107262286;
      threevector fitted_bundler_trans( 
         583554.894526, 4508719.49732, 135.21422601);
      threevector bundler_rotation_origin(
         583299.023754546 , 4506310.19790909 , 101.864543137272);
						      // NYC feature COM
      double global_az = 108.99283146*PI/180;
      double global_el= -1.01519976041*PI/180;
      double global_roll = -0.458266485503*PI/180;
*/
 
      double fitted_nyc_to_bundler_distance_ratio= 16.5752865695;
      threevector fitted_bundler_trans(
         583887.298309 , 4509465.89982  ,508.234588843);
      double global_az = 135.170030088*PI/180;
      double global_el = 2.24855835965*PI/180;
      double global_roll = -9.77460748613*PI/180;

      threevector bundler_rotation_origin(
         583299.091845454 , 4506309.89236364 , 101.864543137272);

      camera_ptr->convert_bundler_to_world_coords(
         fitted_bundler_trans.get(0),
         fitted_bundler_trans.get(1),
         fitted_bundler_trans.get(2),
         bundler_rotation_origin,
         global_az,global_el,global_roll,
         fitted_nyc_to_bundler_distance_ratio);

//      threevector camera_posn(camera_ptr->get_world_posn());

//      double az,el,roll;
//      camera_ptr->get_az_el_roll_from_Rcamera(az,el,roll);
//      cout << "az = " << az*180/PI << endl;
//      cout << "el = " << el*180/PI << endl;
//      cout << "roll = " << roll*180/PI << endl;

      string package_subdir=
         "/home/cho/programs/c++/svn/projects/src/mains/newyork/packages/bundler/";
//      int ndigits=3;
      int ndigits=4;

      string package_filename=
         package_subdir+"photo_"+stringfunc::integer_to_string(n,ndigits)
         +".pkg";

      double frustum_sidelength=50;	// meters
//      double frustum_sidelength=100;	// meters
//      double frustum_sidelength=150;	// meters
      double downrange_distance=-1;	// meters

      camera_ptr->write_camera_package_file(
         package_filename,photograph_ptr->get_ID(),
         photograph_ptr->get_filename(),
         frustum_sidelength,downrange_distance);
  
//      outputfunc::enter_continue_char();
   } // loop over index n labeling cameras
}

