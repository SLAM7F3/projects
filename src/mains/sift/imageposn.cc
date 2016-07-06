// =======================================================================
// Program IMAGEPOSN reads in 2D & 3D features manually extracted from
// photo or video frame which is to be geoaligned with a ladar map.
// It also needs some initial guess for the camera's world position.
// Looping over the X, Y and Z UTM coordinates for the camera's
// position, IMAGEPOSN determines world-ray direction vectors to the
// 3D features.  It computes a homography which projects ray direction vectors
// into the UV image plane.  An RMS residual between projected
// rays and corresponding 2D feature tiepoints is minimized as a
// function of the camera's position.

// In March 2011, we realized that this approach to determining an
// uncalibrated camera's world position is much easier than trying to
// compute a full 3x4 projection matrix via 3D-2D tiepoint matching.
// Fewer numbers of tiepoints need to be manually picked.  Once a
// reasonable estimate for the camera's position is known, its focal
// length and rotation angles may also be determined via 3D ray and 2D
// feature matching. as for multi-image panoramas.

// =======================================================================
// Last updated on 3/4/11; 3/5/11
// =======================================================================

#include <iostream>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "geometry/homography.h"
#include "numerical/param_range.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "video/sift_detector.h"
#include "general/stringfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// =======================================================================
int main( int argc, char** argv )
{

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int n_passes=passes_group.get_n_passes();

   int framenumber;
   cout << "Enter video frame number:" << endl;
   cin >> framenumber;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(0);
   cout << "*photograph_ptr = " << *photograph_ptr << endl;

//   string subdir="./images/img_0134/";
//   string subdir="./images/pressbox/";
   string subdir="./images/stadium_renov/";
   string features_2D_filename=subdir+"features_2D_frame"+
      stringfunc::number_to_string(framenumber)+".txt";
   string features_3D_filename=subdir+"features_3D_frame"+
      stringfunc::number_to_string(framenumber)+".txt";
   
   vector<int> features_ID;
   vector<twovector> features_2D;
   filefunc::ReadInfile(features_2D_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_value=
         stringfunc::string_to_numbers(filefunc::text_line[i]);
      features_ID.push_back(column_value[1]);
      twovector curr_UV(column_value[3],column_value[4]);
      features_2D.push_back(curr_UV);
   }
   templatefunc::Quicksort(features_ID,features_2D);

   features_ID.clear();
   vector<threevector> features_3D;
   filefunc::ReadInfile(features_3D_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_value=
         stringfunc::string_to_numbers(filefunc::text_line[i]);
      features_ID.push_back(column_value[1]);
      threevector curr_XYZ(column_value[3],column_value[4],column_value[5]);
      features_3D.push_back(curr_XYZ);
   }
   templatefunc::Quicksort(features_ID,features_3D);   

   cout.precision(10);
   for (int i=0; i<features_2D.size(); i++)
   {
      cout << i << "  " 
           << features_2D[i].get(0) << "  "
           << features_2D[i].get(1) << "  "
           << features_3D[i].get(0) << "  "
           << features_3D[i].get(1) << "  "
           << features_3D[i].get(2) << endl;
   }

// img_034 panorama
//   threevector init_camera_posn(233429.0309, 3720472.64, 977.3587353);

// Pressbox panorama
//   threevector init_camera_posn(233279 , 3720507 , 1010 );
//   threevector init_camera_posn(233277.1824 ,  3720503.258, 1006.364875);

// Stadium renovation panorama

//   threevector init_camera_posn(233433, 3720601, 983);
//   threevector init_camera_posn(233436, 3720603, 983);
//   threevector init_camera_posn(233436.0325 , 3720603.539 , 983.2860854);
//   threevector init_camera_posn(233430 , 3720600 , 984);
   threevector init_camera_posn(233431.8176 , 3720599.809 , 983.4573999);

   param_range camera_X(
      init_camera_posn.get(0)-1, init_camera_posn.get(0)+1,7);
   param_range camera_Y(
      init_camera_posn.get(1)-1, init_camera_posn.get(1)+1,7);
   param_range camera_Z(
      init_camera_posn.get(2)-1, init_camera_posn.get(2)+1,7);
   
   vector<threevector> rays;

   bool check_homography_flag=true;
   double input_frac_to_use=1.0;
   double min_residual=POSITIVEINFINITY;
   sift_detector SIFT(NULL);


   int n_iters=10;
   for (int iter=0; iter<n_iters; iter++)
   {
      cout << "iter = " << iter << " of " << n_iters << endl;

// ========================================================================
// Begin while loop over camera position parameters
// ========================================================================

      while (camera_X.prepare_next_value())
      {
         while (camera_Y.prepare_next_value())
         {
            while (camera_Z.prepare_next_value())
            {
               threevector camera_posn(camera_X.get_value(),
               camera_Y.get_value(),camera_Z.get_value());
//                cout << "camera_posn = " << camera_posn << endl;

               rays.clear();
               for (int i=0; i<features_3D.size(); i++)
               {
                  threevector curr_r=features_3D[i]-camera_posn;
                  rays.push_back(curr_r.unitvector());
               }

               double curr_residual=SIFT.compute_ray_feature_homography(
                  features_2D,rays,input_frac_to_use,
                  check_homography_flag);
               if (curr_residual < min_residual)
               {
                  min_residual=curr_residual;
                  camera_X.set_best_value();
                  camera_Y.set_best_value();
                  camera_Z.set_best_value();
               }

            } // camera_Z while loop
         } // camera_Y while loop
      } // camera_X while loop

// ========================================================================
// End while loop over camera position parameters
// ========================================================================

      double frac=0.45;
      camera_X.shrink_search_interval(camera_X.get_best_value(),frac);
      camera_Y.shrink_search_interval(camera_Y.get_best_value(),frac);
      camera_Z.shrink_search_interval(camera_Z.get_best_value(),frac);
   } // loop over iter index

   threevector best_camera_posn(
	camera_X.get_best_value(),camera_Y.get_best_value(),
        camera_Z.get_best_value());
   cout << "Best camera posn = " << best_camera_posn << endl;
   camera* camera_ptr=photograph_ptr->get_camera_ptr();
   camera_ptr->set_world_posn(best_camera_posn);

   SIFT.compute_projection_matrix(photograph_ptr);

   cout << "Aspect ratio = " << photograph_ptr->get_aspect_ratio() << endl;
   double f_avg=0.5*(camera_ptr->get_fu()+camera_ptr->get_fv());
   double FOV_u,FOV_v;
   camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
      f_avg,photograph_ptr->get_aspect_ratio(),FOV_u,FOV_v);
   cout << "FOV_u " << FOV_u*180/PI << endl;
   cout << "FOV_v " << FOV_v*180/PI << endl;

   double frustum_sidelength=20;	// meters
   double downrange_distance=-1;
   string output_package_subdir="./packages/";
   SIFT.write_projection_package_file(
      frustum_sidelength,downrange_distance,
      output_package_subdir,photograph_ptr);

}
