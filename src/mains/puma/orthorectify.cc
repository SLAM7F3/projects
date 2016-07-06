// =======================================================================
// Program ORTHORECTIFY imports camera parameters for reconstructed
// and georegistered aerial video frames.  It also works with a world
// z-plane whose altitude is input by the user.  ORTHORECTIFY
// reprojects each aerial image onto the ground plane and outputs an
// orthorectified image.
// =======================================================================
// Last updated on 2/8/13; 9/13/13; 12/5/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "geometry/homography.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
//   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string rectified_images_subdir=bundler_IO_subdir+"rectified_images/";
   filefunc::dircreate(rectified_images_subdir);

// Model ground as horizontal Z-plane:

   double z_groundplane=42;	// Camp Edwards, Cape Cod
//   double z_groundplane=352;	// Yuma Proving Grounds, AZ
   cout << "Enter Z ground plane altitude:" << endl;
   cin >> z_groundplane;
   plane groundplane(z_hat,threevector(0,0,z_groundplane));

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_images=photogroup_ptr->get_n_photos();
   cout << "n_images = " << n_images << endl;

// First determine easting-northing bounding box which holds all
// backprojected images:

   double Emin=POSITIVEINFINITY;
   double Emax=NEGATIVEINFINITY;
   double Nmin=POSITIVEINFINITY;
   double Nmax=NEGATIVEINFINITY;
   for (int n=0; n<n_images; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photo_ptr->get_camera_ptr();

      vector<threevector> world_corner=
         camera_ptr->corner_ray_intercepts_with_zplane(z_groundplane);

      for (int c=0; c<world_corner.size(); c++)
      {
         Emin=basic_math::min(Emin,world_corner[c].get(0));
         Emax=basic_math::max(Emax,world_corner[c].get(0));
         Nmin=basic_math::min(Nmin,world_corner[c].get(1));
         Nmax=basic_math::max(Nmax,world_corner[c].get(1));
      }
   }
   cout << "Emin = " << Emin << " Emax = " << Emax << endl;
   cout << "Nmin = " << Nmin << " Nmax = " << Nmax << endl;

   for (int n=0; n<n_images; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      string image_filename=photo_ptr->get_filename();
      cout << "n = " << n << " image_filename = " << image_filename << endl;
      camera* camera_ptr=photo_ptr->get_camera_ptr();      

//      homography* H_ptr=camera_ptr->homography_from_zplane_to_imageplane(
//         z_groundplane);
//      cout << "n = " << n << " homography = " << *H_ptr << endl;

      homography* H_ptr=camera_ptr->homography_from_worldplane_to_imageplane(
         groundplane);

      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         image_filename,NULL);

      int rectified_width=2*texture_rectangle_ptr->getWidth();
      int rectified_height=2*texture_rectangle_ptr->getHeight();

      string basename=filefunc::getbasename(image_filename);
      string rectified_image_filename=rectified_images_subdir+
         "orthorectified_"+basename;
      
      camerafunc::orthorectify_image(
         texture_rectangle_ptr,
         rectified_width,rectified_height,
         Emin,Emax,Nmin,Nmax,*H_ptr,rectified_image_filename);

      delete texture_rectangle_ptr;
   } // loop over index n labeling images

}
