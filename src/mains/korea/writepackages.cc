// ========================================================================
// Program WRITEPACKAGES has 7 hardwired camera parameters for each of
// 2 internet ground photos derived via program RELATIVE_POSE.  It 
// also incorporates globaly rotation parameters needed to align
// ground camera #1 with its pose as determined via comparison with
// Google Earth imagery.  WRITEPACKAGES exports package files for the
// two ground cameras.
// ========================================================================
// Last updated on 9/19/13; 9/26/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "osg/ModeController.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string packages_subdir=bundler_IO_subdir+"packages/";

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_images=photogroup_ptr->get_n_photos();
   cout << "n_images = " << n_images << endl;

   vector<camera*> camera_ptrs;
   for (int n=0; n<n_images; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      camera* camera_ptr=photo_ptr->get_camera_ptr();
      camera_ptrs.push_back(camera_ptr);
   }

   int curr_photo_ID=0;
   int next_photo_ID=1;
   photograph* curr_photo_ptr=photogroup_ptr->get_photograph_ptr(
      curr_photo_ID);
   photograph* next_photo_ptr=photogroup_ptr->get_photograph_ptr(
      next_photo_ID);
   camera* curr_camera_ptr=camera_ptrs.at(curr_photo_ID);
   camera* next_camera_ptr=camera_ptrs.at(next_photo_ID);

// Hardwire focal parameters for ground_photo1.jpg and ground_photo2.jpg

   curr_camera_ptr->set_f(-3.46866);
   curr_camera_ptr->set_Rcamera(0,0,0);
//   double z_camera1=0;	// height above ground for ground_photo1.jpg
   double z_camera1=22.18-19.1;	// height above ground for ground_photo1.jpg
   threevector curr_world_posn(0,0,z_camera1);
   rotation Rcurr;
   Rcurr=Rcurr.rotation_from_az_el_roll(0,0,0);
   curr_camera_ptr->set_world_posn(curr_world_posn);
   curr_camera_ptr->construct_seven_param_projection_matrix();

   double radius=100;	// meters (arbitrary separation between cameras)
   double frustum_sidelength=0.2*radius;


   next_camera_ptr->set_f(-5.04454);
   double next_phi = 94.5153852321*PI/180;
   double next_theta = 58.8392826326*PI/180;


   double curr_X=radius*cos(next_theta)*cos(next_phi);
   double curr_Y=radius*cos(next_theta)*sin(next_phi);
   double curr_Z=radius*sin(next_theta);
   threevector next_world_posn(curr_X,curr_Y,curr_Z+z_camera1);
   next_camera_ptr->set_world_posn(next_world_posn);

   double next_az = 332.121309186*PI/180;
   double next_el = -38.4606973327*PI/180;
   double next_roll = -1.38210993637*PI/180;
   rotation Rnext;
   Rnext=Rnext.rotation_from_az_el_roll(next_az,next_el,next_roll);
   
   next_camera_ptr->set_Rcamera(next_az,next_el,next_roll);
   next_camera_ptr->construct_seven_param_projection_matrix();

// Perform global rotation order to map curr_camera to its pose as
// determined via GE measurements:

   double curr_camera_az=47.45628506*PI/180;
   double curr_camera_el=2.5*PI/180;
   double curr_camera_roll=0.9145428992*PI/180;

   curr_camera_ptr->set_Rcamera(
      curr_camera_az,curr_camera_el,curr_camera_roll);

   rotation Rglobal;
   Rglobal=Rglobal.rotation_from_az_el_roll(
      curr_camera_az,curr_camera_el,curr_camera_roll);

   Rcurr=Rglobal;
   Rnext=Rglobal * Rnext;
   Rnext.az_el_roll_from_rotation(next_az,next_el,next_roll);
   next_camera_ptr->set_Rcamera(next_az,next_el,next_roll);

   curr_world_posn=Rglobal*curr_world_posn;
   next_world_posn=Rglobal*next_world_posn;
   curr_camera_ptr->set_world_posn(curr_world_posn);
   next_camera_ptr->set_world_posn(next_world_posn);

   curr_camera_ptr->construct_seven_param_projection_matrix();
   next_camera_ptr->construct_seven_param_projection_matrix();

// Export package files for curr_camera and next_camera:

   string curr_package_filename=packages_subdir+"photo_0000.pkg";
   curr_camera_ptr->write_camera_package_file(
      curr_package_filename,curr_photo_ID,curr_photo_ptr->get_filename(),
      frustum_sidelength);

   string next_package_filename=packages_subdir+"photo_0001.pkg";
   next_camera_ptr->write_camera_package_file(
      next_package_filename,next_photo_ID,next_photo_ptr->get_filename(),
      frustum_sidelength);
}
