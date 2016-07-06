// ==========================================================================
// Program BUNDLER_POSNS reads in the contents of bundle.out as well
// as photo filenames.  It converts from Noah's raw coordinate system
// to our world coordinates where Z correlates with height relative to
// local ground.  BUNDLER_POSNS writes out text file
// "bundler_camera_posns.dat" containing the reconstructed camera
// positions in world coordinates which can be read into program
// GPSREGISTER.

//    ./bundler_posns --region_filename ./bundler/lighthawk/packages/peter_inputs.pkg

// ==========================================================================
// Last updated on 1/21/11; 6/14/11
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::pair;
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
   string bundle_filename=passes_group.get_bundle_filename();
   cout << "bundle_filename = " << bundle_filename << endl;


// ------------------------------------------------------------------------
// Instantiate reconstructed photos:

   cout << "Instantiating photogroup:" << endl;
   photogroup* photogroup_ptr=new photogroup();
   photogroup_ptr->set_UTM_zonenumber(12);	// AZ
//   photogroup_ptr->set_UTM_zonenumber(19);	// Boston/Lowell
   photogroup_ptr->set_northern_hemisphere_flag(true);
   string basename=filefunc::getbasename(bundler_IO_subdir);
//   cout << "basename = " << basename << endl;

   int n_photos_to_reconstruct=-1;
   photogroup_ptr->reconstruct_bundler_cameras(
      bundler_IO_subdir,image_list_filename,image_sizes_filename,
      bundle_filename,n_photos_to_reconstruct);

   string output_filename=bundler_IO_subdir+"bundler_camera_posns.dat";
   ofstream outstream;
   outstream.precision(8);
   filefunc::openfile(output_filename,outstream);

   for (unsigned int n=0; n<photogroup_ptr->get_n_photos(); n++)
   {
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photograph_ptr->get_camera_ptr();

      camera_ptr->convert_bundler_to_world_coords(
         0,0,0,Zero_vector,0,0,0,1);
      string photo_filename=filefunc::getbasename(
         photograph_ptr->get_filename());
      threevector cam_posn=camera_ptr->get_world_posn();
    
      outstream << n << "  "
                << photo_filename << "  "
                << cam_posn.get(0) << "  "
                << cam_posn.get(1) << "  "
                << cam_posn.get(2)  << endl;
   }
   
   filefunc::closefile(output_filename,outstream);
}

