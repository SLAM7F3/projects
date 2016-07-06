// ========================================================================
// Program RELROT computes the Uaxis and Vaxis focal length parameters
// fu and fv, camera center coords U0 and V0, pixel skew angle theta,
// and the relative rotation angles between two ground photos taken
// from a common camera location.  It derives these parameters using
// feature tiepoints extracted from both images.

//	relrot piers1.jpg --newpass piers2.jpg

// 	relrot piers2.jpg --newpass piers3.jpg

//	relrot piers3.jpg --newpass piers4.jpg

//     	relrot Shanghai_04.png --newpass Shanghai_05.png

//     	relrot Shanghai_06.png --newpass Shanghai_05.png

// ========================================================================
// Last updated on 9/26/07; 10/1/07; 10/15/07; 10/27/08
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgText/Text>

#include "osg/osgGrid/AlirtGridsGroup.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "math/constant_vectors.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "general/filefuncs.h"
#include "math/lttwovector.h"
#include "math/mathfuncs.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgOperations/Operations.h"
#include "numerical/param_range.h"
#include "passes/PassesGroup.h"
#include "math/rotation.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

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
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   vector<int> videopass_ID;
   for (unsigned int n=0; n<passes_group.get_n_passes(); n++)
   {
      if (passes_group.get_pass_ptr(n)->get_passtype()==Pass::video)
      {
         videopass_ID.push_back(n);
         cout << "n = " << n << " videopass_ID = " << videopass_ID.back()
              << endl;
      }
   }

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Instantiate AlirtGrid decorations group:

   double min_X=0;
   double max_X=100;
   double min_Y=0;
   double max_Y=100;
   double min_Z=0;

   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(videopass_ID.front()),
      min_X,max_X,min_Y,max_Y,min_Z);

   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate ObsFrusta decoration group:

   decorations.add_ObsFrusta(
      passes_group.get_pass_ptr(videopass_ID.front()),
      AnimationController_ptr,CM_3D_ptr);

// Instantiate an individual ObsFrustum for every input video.
// Each contains a separate movie object.

   vector<int> pass_numbers;
   vector<string> video_filename;
   vector<ObsFrustum*> ObsFrusta_ptrs;

   typedef std::map<twovector,twovector,lttwovector > FEATURES_MAP;
   FEATURES_MAP features_map;

   double Ufactor=1;
   double Vfactor=1;
//   double Ufactor=1000;
//   double Vfactor=1000;

   for (unsigned int n=0; n<passes_group.get_n_passes(); n++)
   {
      video_filename.push_back(passes_group.get_pass_ptr(n)->
                               get_first_filename());
      cout << "n = " << n << " video_filename = " << video_filename.back()
           << endl;
      ObsFrusta_ptrs.push_back(
         decorations.get_ObsFrustaGroup_ptr()->generate_movie_ObsFrustum(
            video_filename.back()));

// Read UV feature pairs from input text files into an STL map.
// Features in one image which have no counterpart in the other will
// be ignored:

      string basename=stringfunc::prefix(video_filename.back());
      string features_filename="features_2D_"+basename+".txt";
      cout << "features_filename = " << features_filename << endl;
      filefunc::ReadInfile(features_filename);

      int curr_passnumber;
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<double> columns=stringfunc::string_to_numbers(
            filefunc::text_line[i]);
         features_map[twovector(columns[2],columns[1])]=
            twovector(Ufactor*columns[3],Vfactor*columns[4]);
         curr_passnumber=columns[2];
         cout << "pass = " << curr_passnumber << " f = " << columns[1]
              << " U = " << columns[3] << " V = " << columns[4] << endl;
      }
      pass_numbers.push_back(curr_passnumber);
   } // loop over index n labeling video passes

// Erase information from STL map for features found in one image
// which have no counterpart in the other:

   vector<twovector> keys_to_erase;
   for (FEATURES_MAP::iterator map_iter=features_map.begin(); 
        map_iter != features_map.end(); ++map_iter)
   {
      twovector key(map_iter->first);
      twovector value(map_iter->second);
//      cout << "key = " << key << " value = " << value << endl;
      int curr_pass_number=key.get(0);
      int curr_feature_number=key.get(1);

      int other_pass_number=pass_numbers[1];
      if (curr_pass_number==pass_numbers[1])
      {
         other_pass_number=pass_numbers[0];
      }
      
//      cout << "curr_pass_number = " << curr_pass_number
//           <<  " other_pass_number = " << other_pass_number << endl;
//      cout << "feature number = " << curr_feature_number << endl;

      FEATURES_MAP::iterator counterpart_feature_map_iter=
         features_map.find(twovector(other_pass_number,curr_feature_number));

      if (counterpart_feature_map_iter != features_map.end())
      {
//         cout << "Counterpart feature identified:" << endl;
//         twovector counterpart_key(counterpart_feature_map_iter->first);
//         twovector counterpart_value(counterpart_feature_map_iter->second);
//         cout << "key = " << counterpart_key 
//              << " value = " << counterpart_value << endl;
      }
      else
      {
//         cout << "Feature not found in features_map" << endl;
         keys_to_erase.push_back(key);
      }
   } // loop over map_iter indexing entries within features_map

   for (unsigned int k=0; k<keys_to_erase.size(); k++)
   {
      features_map.erase(keys_to_erase[k]);
   }
   cout << "After erasings, features_map.size() = "
        << features_map.size() << endl;
   int n_rays=features_map.size()/2;

   cout << "n_rays = " << n_rays << endl;

// Instantiate canonically oriented ObsFrustum pointing along +x_hat:
   
   ObsFrustum* ObsFrustum_ptr=ObsFrusta_ptrs.back();

   threevector abs_posn=*grid_origin_ptr+threevector(60,60,0);
   double az=0*PI/180;
   double el=0*PI/180;
   double roll=0*PI/180;

   camera* camera_ptr=ObsFrustum_ptr->get_Movie_ptr()->get_camera_ptr();
   camera_ptr->set_world_posn(abs_posn);
   camera_ptr->set_Rcamera(az,el,roll);

// Initialize loops over internal camera parameters:

//   param_range fu(-1.79636363215 , -1.79636363215 , 1);
//   param_range fv(-1.72363636785 , -1.72363636785 , 1);
//   param_range u0(0.610321226453 , 0.610321226453 , 1);
//   param_range v0(0.534545448221 , 0.534545448221 , 1);
//   param_range theta( 90.9045454018 , 90.9045454018 , 1);

// Nearly ideal alignment between piers2.jpg and piers3.jpg (but which
// is wildly inconsistent with NYC point cloud!):

//   param_range fu( -1.96879774102,  -1.96879774102 , 1);
//   param_range fv( -1.53636367853 ,  -1.53636367853 , 1);
//   param_range u0(  0.631734128323 ,   0.631734128323 , 1);
//   param_range v0(0.435204270819 , 0.435204270819 , 1);
//   param_range theta(  89.010901726 ,  89.010901726 , 1);

   param_range fu(-2.3, -1.5, 9);
//   param_range fv(-1.78, -1.74, 7);
//   param_range fu(-2.1*Ufactor , -1.7*Ufactor , 15);
//   param_range fv(-2.1*Vfactor , -1.7*Vfactor, 15);
//   param_range u0(0.6 , 0.65 , 7);
//   param_range v0(0.45 , 0.51 , 7);
   param_range u0(camera_ptr->get_u0()-0.1,camera_ptr->get_u0()+0.1, 7);
   param_range v0(camera_ptr->get_v0()-0.1,camera_ptr->get_v0()+0.1, 7);
//   param_range u0((camera_ptr->get_u0()-0.1)*Ufactor,
//                  (camera_ptr->get_u0()+0.1)*Ufactor, 7);
//   param_range v0((camera_ptr->get_v0()-0.1)*Vfactor,
//                  (camera_ptr->get_v0()+0.1)*Vfactor, 7);

//   param_range theta( 90.2 , 90.7 , 7);
//   param_range theta( 89 , 91 , 7);

   double min_score=POSITIVEINFINITY;
   double best_az=0;
   double best_el=0;
   double best_roll=0;

   genmatrix G(3,n_rays),H(3,n_rays);
   genmatrix Gtransinv(3,n_rays),Ginv(n_rays,3);
   genmatrix Delta(3,n_rays);
   rotation R;

   int n_iters=20;
   for (int iter=0; iter<n_iters; iter++)
   {
      cout << "iter = " << iter << " of " << n_iters << endl;

// ========================================================================
// Begin while loop over internal camera parameters
// ========================================================================

      while (fu.prepare_next_value())
      {
//         while (fv.prepare_next_value())
         {
            while (u0.prepare_next_value())
            {
               while (v0.prepare_next_value())
               {
//                  while (theta.prepare_next_value())
                  {
                     vector<threevector> ray_hat;
                     for (unsigned int n_frusta=0;
                          n_frusta<ObsFrusta_ptrs.size(); n_frusta++)
                     {
                        double curr_theta=90;
                        camera_ptr->set_internal_params(
//                           fu.get_value(),fv.get_value(),
                           fu.get_value(),fu.get_value(),
                           u0.get_value(),v0.get_value(),
//                           theta.get_value());
                           curr_theta);

//                        cout << "fu = " << fu.get_value()
//                             << " fv = " << fv.get_value()
//                             << " u0 = " << u0.get_value()
//                             << " v0 = " << v0.get_value() << endl;
                        camera_ptr->construct_projection_matrix();
       
                        for (FEATURES_MAP::iterator map_iter=
                                features_map.begin(); map_iter != 
                                features_map.end(); ++map_iter)
                        {
                           twovector key(map_iter->first);
                           int curr_pass_number=key.get(0);
//                           cout << "curr_pass_number = "
//                                << curr_pass_number << endl;
//                           cout << "n_frusta = " << n_frusta << endl;
                           if (curr_pass_number==pass_numbers[n_frusta])
                           {
                              twovector value(map_iter->second);
                              ray_hat.push_back(
                                 camera_ptr->pixel_ray_direction(value));

//                              cout << " r0 = " << ray_hat.back().get(0)
//                                   << " r1 = " << ray_hat.back().get(1)
//                                   << " r2 = " << ray_hat.back().get(2)
//                                   << endl;
//                              outputfunc::enter_continue_char();
                           }
                        } // loop over map_iter labeling ray direction vectors
                     } // loop over index n_frusta labeling ObsFrusta

// Fill columns of G and H matrices with ray direction vectors
// corresponding to two photos' UV features:

                     for (int i=0; i<n_rays; i++)
                     {
                        G.put_column(i,ray_hat[i]);
                        H.put_column(i,ray_hat[i+n_rays]);
                     }
//      cout << "G = " << G << endl;
//      cout << "H = " << H << endl;

// Compute relative rotation which maps rays in G into those in H:

                     const double min_abs_singular_value=1E-6;
                     G.transpose().pseudo_inverse(
                        min_abs_singular_value,Gtransinv);
                     Ginv=Gtransinv.transpose();
                     R=H*Ginv;
//      cout << "R = " << R << endl;
//                     double detR=R.determinant();
//      cout << "det(R) = " << detR << endl;
//      cout << "R*Rtrans = " << R*R.transpose() << endl;
//      outputfunc::enter_continue_char();

// Decompose relative rotation into relative azimuth, elevation and
// roll angles:

                     double az_rel,el_rel,roll_rel;
                     R.az_el_roll_from_rotation(az_rel,el_rel,roll_rel);
                     double relative_az=az_rel*180/PI;
                     double relative_el=el_rel*180/PI;
                     double relative_roll=roll_rel*180/PI;
   
// Compute angular difference between rotated entries in G and entries
// in H.  Multiply by absolute value of focal parameter to generate
// goodness-of-fit score:

                     Delta=R*G-H;
                     double score=0;
                     threevector curr_column;
                     for (int i=0; i<n_rays; i++)
                     {
                        Delta.get_column(i,curr_column);
                        score += curr_column.sqrd_magnitude();
                     }
                     score=1000*fabs(
                        sqrt(fu.get_value()*fu.get_value()*score))/
//                        sqrt(fu.get_value()*fv.get_value()*score))/
                        double(n_rays);

//         cout.precision(6);
//         cout << "f, score, az, el, roll = " 
//              << f.get_value() << " , " 
//              << score << " , " 
//              << relative_az << " , " 
//              << relative_el << " , "
//              << relative_roll << endl;

                     if (score < min_score)
                     {
                        min_score=score;
                        fu.set_best_value();
//                        fv.set_best_value();
                        u0.set_best_value();
                        v0.set_best_value();
//                        theta.set_best_value();
                        best_az=relative_az;
                        best_el=relative_el;
                        best_roll=relative_roll;
                     }
                  } // theta while loop
               } // v0 while loop
            } // u0 while loop
         } // fv while loop
      } // fu while loop

// ========================================================================
// End while loop over internal camera parameters
// ========================================================================

      double frac=0.45;
      fu.shrink_search_interval(fu.get_best_value(),frac);
//      fv.shrink_search_interval(fv.get_best_value(),frac);
      u0.shrink_search_interval(u0.get_best_value(),frac);
      v0.shrink_search_interval(v0.get_best_value(),frac);
//      theta.shrink_search_interval(theta.get_best_value(),frac);

      cout << "Best fu value = " << fu.get_best_value() << endl;
//      cout << "Best fv value = " << fv.get_best_value() << endl;
      cout << "Best u0 value = " << u0.get_best_value() << endl;
      cout << "Best v0 value = " << v0.get_best_value() << endl;  
//      cout << "Best theta value = " << theta.get_best_value() << endl;

      cout << "Best azimuth angle = " << best_az << endl;
      cout << "Best elevation angle = " << best_el << endl;
      cout << "Best roll angle = " << best_roll << endl;

      cout << "min_score = " << min_score << endl;

   } // loop over iter index

   cout.precision(12);

   cout << "Minimum score = " << min_score << endl;
   cout << "1st photo orientation relative to 2nd: " << endl << endl;

//   cout << "--focal_length " << f.get_best_value() << endl;
   cout << "--Uaxis_focal_length  " << fu.get_best_value() << endl;
   cout << "--Vaxis_focal_length  " << fu.get_best_value() << endl;
//   cout << "--Vaxis_focal_length  " << fv.get_best_value() << endl;
   cout << "--U0  " << u0.get_best_value() 
        << " --V0  " << v0.get_best_value() << endl;
//   cout << "--pixel_skew_angle " << theta.get_best_value() << endl;
   cout << "--relative_az " << best_az << endl;
   cout << "--relative_el " << best_el << endl;
   cout << "--relative_roll " << best_roll << endl;

}

