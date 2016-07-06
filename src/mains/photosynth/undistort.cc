// ==========================================================================
// Program UNDISTORT is a simple rewrite of Noah Snavely's
// RadialUndistort executable which uses Peter's classes and
// conventions.  UNDISTORT creates a new undistorted_images
// subdirectory within bundler_IO_subdir.  It exports radially
// undistorted JPG files into this subdirectory along with new
// versions of bundle.out and list_tmp.txt.  UNDISTORT then moves the
// original versions of bundle.out, list_tmp.txt and images/ and creates
// soft links to their new replacements within bundler_IO_subdir.

// Note added on 12/17/11: In order to replicate results from Noah's
// RadialUndistort routine, we must work with the same sized images.
// Recall that TXX-bundler subsamples all input images down to 2400xH
// or Wx2400.  So we must supply same-sized subsampled images in order
// to reproduce RadialUndistort output image appearance!

// ==========================================================================
// Last updated on 11/25/11; 12/17/11
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "bundler/bundlerfuncs.h"
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "math/rotation.h"          
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "math/threevector.h"
#include "video/videofuncs.h"

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
   string undistorted_images_subdir=bundler_IO_subdir+"/undistorted_images/";
   filefunc::dircreate(undistorted_images_subdir);

// ------------------------------------------------------------------------
// Instantiate reconstructed photos:

   cout << "Instantiating photogroup:" << endl;
   photogroup* photogroup_ptr=new photogroup();
   int n_photos_to_reconstruct=-1;
   photogroup_ptr->reconstruct_bundler_cameras(
      bundler_IO_subdir,image_list_filename,image_sizes_filename,
      bundle_filename,n_photos_to_reconstruct);

   string undistorted_list_filename=undistorted_images_subdir+"list.rd.txt";
   ofstream liststream;
   filefunc::openfile(undistorted_list_filename,liststream);

   int n_photos=photogroup_ptr->get_n_photos();
   for (int n=0; n<n_photos; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
//      cout << "n = " << n << " *photo_ptr = " << *photo_ptr << endl;
//       camera* camera_ptr=photo_ptr->get_camera_ptr();
//      cout << "n = " << n
//           << " camera = " << *camera_ptr << endl;
      string undistorted_filename=
         bundlerfunc::radial_undistort(photo_ptr,undistorted_images_subdir);
      liststream << "./images/"+filefunc::getbasename(undistorted_filename) 
                 << endl;
   }
   filefunc::closefile(undistorted_list_filename,liststream);

   bundlerfunc::generate_undistorted_bundle_file(
      bundle_filename,undistorted_images_subdir);

   exit(-1);

// Rename original bundle.out and list_tmp.txt files for distorted
// photos.  Then create soft links to new versions of the text files
// corresponding to radially undistorted photos:

   string unix_cmd="mv "+bundle_filename+" "+bundler_IO_subdir+
      "bundle_distorted.out";
//   cout << "unix cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);

   unix_cmd="mv "+bundler_IO_subdir+"list_tmp.txt "+
      bundler_IO_subdir+"list_tmp_distorted.txt";
//   cout << "unix_cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);

   unix_cmd="mv "+bundler_IO_subdir+"images "+
      bundler_IO_subdir+"distorted_images";
//   cout << "unix_cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);

   unix_cmd="ln -s ./undistorted_images/bundle.rd.out "
      +bundler_IO_subdir+"bundle.out";
//   cout << "unix cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);

   unix_cmd="ln -s ./undistorted_images/list.rd.txt "
      +bundler_IO_subdir+"list_tmp.txt";
//   cout << "unix cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);

   unix_cmd="ln -s ./undistorted_images "+bundler_IO_subdir+"images";
//   cout << "unix cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);
}



