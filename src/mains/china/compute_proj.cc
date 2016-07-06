// ========================================================================
// Program COMPUTE_PROJ hardwires in absolute camera world position
// and orientation information.  It also hardwires in a set of camera
// intrinsic parameter values.  COMPUTE_PROJ then computes the 3x4
// projection matrix corresponding to these pinhole model parameters.

// We wrote this utility program to determine the projection matrices
// for piers1-4.jpg derived from both 3D/2D and 2D/2D tiepoint
// matching.

//			compute_proj piers3.jpg

// ========================================================================
// Last updated on 9/27/07; 9/29/07; 10/1/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>

#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "osg/osg2D/Movie.h"
#include "numerical/param_range.h"
#include "passes/PassesGroup.h"

#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input files:

   const int ndims=3;
   PassesGroup passes_group(&arguments);

// Instantiate animation controller & key handler:

   int n_animation_frames=1;
   AnimationController* AnimationController_ptr=new AnimationController(
      n_animation_frames);

// Instantiate Movie containing a camera object:
   
   string video_filename=passes_group.get_videopass_ptr()->
      get_first_filename();
   Movie* Movie_ptr=new Movie(2,video_filename,
                              0,1,AnimationController_ptr);
   camera* camera_ptr=Movie_ptr->get_camera_ptr();

// Absolute extrinsic camera parameters derived for piers3.jpg based
// upon compromise internal and external parameters:

   threevector world_posn( 584609.3376 , 4506746.633 , 47.41765454 );
   camera_ptr->set_world_posn(world_posn);

   double az_3 =  -169.1283651 * PI/180;
   double el_3 =  2.588068193 * PI/180;
   double roll_3 =  -0.3128133326 * PI/180;

   double relative_az=(9.44052058118 + 10.1590261384) * PI/180;	  // piers1
   double relative_el=(0.0649480557267 + 0.14317099336) * PI/180; // piers1
   double relative_roll=(0.370403398153 + 0.436963450428) * PI/180; // piers1

//   double relative_az=0 * PI/180;		// piers3
//   double relative_el=0 * PI/180;		// piers3
//   double relative_roll=0 * PI/180;		// piers3

//   double relative_az=-8.94342810909 * PI/180;		// piers4
//   double relative_el=-0.122836910033 * PI/180;		// piers4
//   double relative_roll=-0.508578715938 * PI/180;	// piers4

   double az= az_3 + relative_az;
   double el= el_3 + relative_el;
   double roll= roll_3 + relative_roll;
   
   camera_ptr->set_Rcamera(az,el,roll);

/*
// Internal parameters corresponding to nearly ideal fit between
// piers3.jpg and NYC point cloud:

   double rho = -6947038.77848;
//   double rho = 1;
   double fu = -1.76739664653;
   double fv = -1.77530391234;
   double u0 = 0.611923873586;
   double v0 = 0.48698197413;
   double pixel_skew_angle = 90.5847879487 ;
*/

/*

Projection matrix for nearly ideal piers3.jpg fit to NYC point cloud:

1.339120051e-07 -2.334256049e-07        -8.372842193e-09        0.9737047237
6.053162507e-08 1.466985373e-08 -2.575778566e-07        -0.1014893688
1.413368395e-07 2.692243474e-08 -4.426703253e-09        -0.2039583757

*/

// Compromise internal camera parameters based upon mosaicing
// piers2.jpg and piers3.jpg:

   double fu=-1.79636363215;
   double fv=-1.72363636785;
   double u0=0.610321226453;
   double v0=0.534545448221;
   double pixel_skew_angle=90.9045454018;
   double rho=1;

   camera_ptr->set_internal_params(fu,fv,u0,v0,pixel_skew_angle);
   camera_ptr->set_rho(rho);
   camera_ptr->construct_projection_matrix();

   cout.precision(12);
   cout << endl;
   cout << "Projection matrix = "
        << *(camera_ptr->get_P_ptr()) << endl;

   camera_ptr->decompose_projection_matrix();
   camera_ptr->print_external_and_internal_params();
}

