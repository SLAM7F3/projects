// ==========================================================================
// Program DRAPE_EO reads in some baghdad satellite EO tile along with
// some baghdad [NYC RTV] point cloud tile.  It also takes in manually
// selected 2D/3D feature text files and computes the 3x4 projection
// matrix which maps latter onto the former.  This program then
// iterates over every point in the cloud and colors it to indicate
// combined height/reflectivity information.  Fused data is written to
// TDP file output.

// 		drape_EO --region_filename fuse.pkg

//  drape_EO --region_filename NYC_sat_EO.pkg --region_filename NYC_tdp.pkg

// ==========================================================================
// Last updated on 5/2/07; 5/3/07; 5/10/07; 5/11/07; 8/12/07; 10/30/07
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "general/filefuncs.h"
#include "osg/osgFusion/FusionGroup.h"
#include "math/genmatrix.h"
#include "osg/osg2D/MoviesGroup.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main (int argc, char* argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

   vector<double> min_U,min_V;

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   int videopass_ID=passes_group.get_videopass_ID();
   cout << "videopass_ID = " << videopass_ID << endl;

// First instantiate group to hold movie:

   AnimationController* AnimationController_ptr=new AnimationController();

   MoviesGroup movies_group(
      2,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group);
   AnimationController_ptr->set_nframes(movie_ptr->get_Nimages());

// Instantiate camera object:

   int frame_number=0;
   camera* curr_camera_ptr=movie_ptr->get_camera_ptr(frame_number);

// =========================================================================

/*

// In this next section, we calibrate the camera using manually
// selected 3D/2D tiepoint information.  Once the camera's 3x4
// projection matrix is determined, we can comment out this section
// and move on to actual backprojection RGB color draping:

// Read file containing 2D feature UV locations renormalized so that
// all V values range between 0 and 1:

   string feature2D_filename="f2D_renorm.txt";
   if (!filefunc::ReadInfile(feature2D_filename))
   {
      cout << "Could not read in file containing UV features" << endl;
      exit(-1);
   }

   cout.precision(12);
   vector<twovector> UV;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double curr_u=column_values[3];
      double curr_v=column_values[4];
      UV.push_back(twovector(curr_u,curr_v));
//      cout << "i = " << i << " U = " << curr_u << " V = " << curr_v
//           << endl;
   }

// Read file containing 3D feature XYZ locations in absolute world
// space:

   string feature3D_filename="f3d.txt";
//   string feature3D_filename="f3D_georegistered.txt";
   if (!filefunc::ReadInfile(feature3D_filename))
   {
      cout << "Could not read in file containing XYZ features" << endl;
      exit(-1);
   }

   vector<threevector> XYZ;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double curr_x=column_values[3];
      double curr_y=column_values[4];
      double curr_z=column_values[5];
      XYZ.push_back(threevector(curr_x,curr_y,curr_z));
//      cout << "i = " << i << " X = " << curr_x 
//           << " Y = " << curr_y << " Z = " << curr_z << endl;
   }

// Consolidate 2D and 3D feature information within genmatrix *XYZUV_ptr:

   genmatrix* XYZUV_ptr=new genmatrix(XYZ.size(),6);
   for (int n=0; n<XYZ.size(); n++)
   {
      XYZUV_ptr->put(n,0,n);
      XYZUV_ptr->put(n,1,XYZ[n].get(0));
      XYZUV_ptr->put(n,2,XYZ[n].get(1));
      XYZUV_ptr->put(n,3,XYZ[n].get(2));
      XYZUV_ptr->put(n,4,UV[n].get(0));
      XYZUV_ptr->put(n,5,UV[n].get(1));
   }
//   cout << "*XYZUV_ptr = " << *XYZUV_ptr << endl;

// Compute and check 3x4 projection matrix from 2D/3D feature tiepoint
// information:

   curr_camera_ptr->compute_tiepoint_projection_matrix(XYZUV_ptr);
   curr_camera_ptr->check_projection_matrix(XYZUV_ptr);
   delete XYZUV_ptr;

   exit(-1);
*/

/*

// 3x4 projection matrix for NYC satellite EO imagery mapped onto RTV
// ladar data:

2.19962187097e-07       6.86682554355e-11       1.82956984265e-08       -0.128385052067
-1.49654457283e-10      2.20171398011e-07       1.3132407665e-08        -0.99171640874
-3.20012220916e-10      1.27733974713e-10       8.02367730103e-09       0.00398033195968

Camera world posn = 
627440.333936
4537123.65273
-543278.107944


Camera pointing direction = 
0.039846761629
-0.0159049714656
-0.999079209808

RMS residual between measured and calculated UV points = 6.15450376632e-05
RMS U pixel offset = 0.49934945713 out of 14513 = 0.00344070458989 %
RMS V pixel offset = 0.459774626586 out of 11029 = 0.00416877891546 %

*/

/*

Projection matrix for Baghdad EO tiles in rows 2-3 and columns 1-3
matching onto Baghdad ladar tiles 38, 39 40 & 47, 48, 49:

3x4 projection matrix P =
2.70373228579e-07       -1.34745251998e-11      -2.92256657526e-08      -0.116204609728
-1.03290731964e-11      2.70357702167e-07       8.51022277584e-09       -0.993213941977
-3.30582650516e-12      -3.12074103845e-11      1.46001757758e-08       0.00474911988953

Camera world posn =
395677.146375
3683706.59027
-317314.871915

===============================================
RMS residual between measured and calculated UV points = 8.08316612194e-05
RMS U pixel offset = 0.905914356692 out of 14336 = 0.00631915706398 %
RMS V pixel offset = 0.708247820322 out of 14226 = 0.00497854506061 %
===============================================

*/

// =========================================================================

// After projection matrix has been calculated once, we store it
// within an input video package file.  Subsequently, we just read
// the 3x4 matrix entries into curr_camera_ptr's P matrix:

   genmatrix* projmatrix_ptr=
      passes_group.get_pass_ptr(videopass_ID)->get_PassInfo_ptr()->
      get_projection_matrix_ptr();
   curr_camera_ptr->set_projection_matrix(*projmatrix_ptr);

// NYC satellite EO imagery tiles:

   min_U.push_back(0);
   min_U.push_back(0.438631487);
   
   min_V.push_back(0);
   min_V.push_back(0.3333333333333333);
   min_V.push_back(0.6666666666666666);
   min_V.push_back(1.0);

/*
// Baghdad satellite EO imagery tiles:

   min_U.push_back(0);
   min_U.push_back(0);
   min_U.push_back(0.50192563546);
   min_U.push_back(1.00385127092);
   min_U.push_back(1.50577690638);
   
   min_V.push_back(0);
   min_V.push_back(0.49807436454);
   min_V.push_back(1);
*/

   int row=1;
//    int row=2;
//   int row=3;
   int column=0;

//   cout << "Enter tile row number: " << endl;
//   cin >> row;
//   cout << "Enter tile column number:" << endl;
//   cin >> column;

   double u_min=min_U[column];
   double u_max=min_U[column+1];
//   double v_min=min_V[3-row];
//   double v_max=min_V[3-row+1];
   double v_min=min_V[3-row];
   double v_max=min_V[3-row+1];

// Row "1.5" corresponding to NYC RTV tiles x1y5, x2y5, x3y5:

//   v_min=0.519478949;
//   v_max=0.720978028;

// Row "2.5" corresponding to NYC RTV tiles x0y2, x1y2, x2y2, x3y2:

//   v_min=0.267277769;
//   v_max=0.418744522;

   cout << "min_U = " << u_min << " max_U = " << u_max << endl;
   cout << "min_V = " << v_min << " max_V = " << v_max << endl;

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));
   clouds_group.generate_Clouds(passes_group);
   PointCloud* cloud_ptr=clouds_group.get_Cloud_ptr(0);

// Hardwire z thresholds derived from entire Baghdad ladar map:
   
// j = 0 min_threshold = 417197 max_threshold = 451176
// j = 1 min_threshold = 3.67647e+06 max_threshold = 3.68977e+06
// j = 2 min_threshold = -71.5953 max_threshold = 63.9469
// j = 3 min_threshold = 0 max_threshold = 1

//   cloud_ptr->get_z_ColorMap_ptr()->set_min_threshold(-71.5953);
//   cloud_ptr->get_z_ColorMap_ptr()->set_max_threshold(63.9469);

// Hardwire z thresholds derived from entire georegistered Baghdad
// ladar map (5/10/07):

// min_threshold_frac = 0 max_threshold_frac = 1
// j = 0 min_threshold = 417180.789482 max_threshold = 451164.366768
// j = 1 min_threshold = 3676467.41972 max_threshold = 3689771.58028
// j = 2 min_threshold = 32.3045275032 max_threshold = 170.331870812
// j = 3 min_threshold = 0 max_threshold = 1

/*
   cloud_ptr->get_z_ColorMap_ptr()->set_min_threshold(32.3);
//   cloud_ptr->get_z_ColorMap_ptr()->set_max_threshold(170.3);
   cloud_ptr->get_z_ColorMap_ptr()->set_max_threshold(150.3);
*/

// Hardwire z thresholds derived from entire georegistered NYC RTV
// ladar map (8/12/07):

// j = 0 min_threshold = 582788.272715 max_threshold = 590686.914785
// j = 1 min_threshold = 4505936.31833 max_threshold = 4522754.68167
// j = 2 min_threshold = -8.67029124024 max_threshold = 394.300333316
// j = 3 min_threshold = 0 max_threshold = 1

   cloud_ptr->get_z_ColorMap_ptr()->set_min_threshold(-8.6);
   cloud_ptr->get_z_ColorMap_ptr()->set_max_threshold(390);

// Instantiate fusion group to hold math relationships between 3D and
// 2D imagery for draping purposes:

   FusionGroup* FusionGroup_ptr=new FusionGroup(
      passes_group.get_pass_ptr(cloudpass_ID),
      cloud_ptr,movie_ptr,AnimationController_ptr,false,false);

   movie_ptr->get_texture_rectangle_ptr()->
      reset_UV_coords(u_min,u_max,v_min,v_max);

   double s_weight=0.1;	// Looks best for NYC RTV data (at least in Wall St)
//   cout << "Enter saturation weight:" << endl;
//   cin >> s_weight;

   FusionGroup_ptr->backproject_videoframe_onto_pointcloud(
      frame_number,s_weight);

   string cloud_filename=passes_group.get_pass_ptr(cloudpass_ID)->
      get_first_filename();
   string basename=filefunc::getbasename(cloud_filename);
   string fused_filename=stringfunc::prefix(basename)+"_fused";
   cout << "fused_filename = " << fused_filename << endl;

   cloud_ptr->write_TDP_file(fused_filename);
} 

