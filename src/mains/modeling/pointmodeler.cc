// ==========================================================================
// Program POINTMODELER reads in 2D UV features selected from at least
// 2 reconstructed photos.  It backprojects their associated rays
// into 3D space to find their intersection point.  POINTMODELER
// implements a RANSAC procedure to eliminate the outlier rays.  It
// outputs a best-estimate for the 3D location corresponding to the
// input 2D UV tiepoints.
// ==========================================================================
// Last updated on 1/23/12; 1/24/12; 2/28/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "osg/osgOrganization/Decorations.h"
#include "general/filefuncs.h"
#include "math/genvector.h"
#include "geometry/geometry_funcs.h"
#include "geometry/linesegment.h"
#include "osg/ModeController.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=false;
   bool display_movie_number=false;
   bool display_movie_world_time=false;
   bool hide_Mode_HUD_flag=false;
   Operations operations(
      ndims,window_mgr_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

   FeaturesGroup* FeaturesGroup_ptr=
      decorations.add_Features(ndims,passes_group.get_pass_ptr(cloudpass_ID));


// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   threevector camera_world_COM(Zero_vector);
   vector<camera*> camera_ptrs;
   for (int n=0; n<n_photos; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
//      cout << "n = " << n << " *photo_ptr = " << *photo_ptr << endl;
      camera* camera_ptr=photo_ptr->get_camera_ptr();
      camera_ptrs.push_back(camera_ptr);
      cout << "n = " << n
           << " camera = " << *camera_ptr << endl;
      camera_world_COM += camera_ptr->get_world_posn();
   }
   camera_world_COM /= n_photos;
   cout << "camera_world_COM = " << camera_world_COM << endl;

// Subtract off camera_world_COM from camera world positions in order
// to avoid working with numerically large values:

   int n_cameras=camera_ptrs.size();
   for (int n=0; n<n_cameras; n++)
   {
      camera* camera_ptr=camera_ptrs[n];
      threevector camera_world_posn=camera_ptr->get_world_posn();
      camera_world_posn -= camera_world_COM;
      camera_ptr->set_world_posn(camera_world_posn);
      camera_ptr->construct_projection_matrix(false);
      genmatrix* P_ptr=camera_ptr->get_P_ptr();
      cout << "n = " << n << " reset P = " << *P_ptr << endl;
   }

// Read in 2D features manually extracted from reconstructed photos:

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("txt");

   string jpg_files_subdir="./";
   string substring="features_2D";
   vector<string> feature_filenames=
      filefunc::files_in_subdir_matching_substring(
         jpg_files_subdir,substring);

   vector<twovector> feature_UVs;
   vector<linesegment> line_segments;
   for (int f=0; f<feature_filenames.size(); f++)
   {
      cout << "f = " << f
           << " feature filename = " << feature_filenames[f] << endl;

      FeaturesGroup_ptr->read_feature_info_from_file(
         feature_filenames[f]);
//      cout << "n_features = " << FeaturesGroup_ptr->get_n_Graphicals()
//           << endl;

      Feature* Feature_ptr=FeaturesGroup_ptr->get_Feature_ptr(0);
//      cout << "Curr feature = " << *Feature_ptr << endl;
      threevector UVW;
      Feature_ptr->get_UVW_coords(
         FeaturesGroup_ptr->get_curr_t(),
         FeaturesGroup_ptr->get_passnumber(),UVW);
      feature_UVs.push_back(twovector(UVW));
//      cout << "U = " << feature_UVs.back().get(0)
//           << " V = " << feature_UVs.back().get(1) << endl;

      camera* camera_ptr=camera_ptrs[f];
      threevector camera_posn=camera_ptr->get_world_posn();
      threevector r_hat=camera_ptr->pixel_ray_direction(feature_UVs.back());
      line_segments.push_back(linesegment(camera_posn,camera_posn+r_hat));
   } // loop over index f labeling feature filename or equivalently camera

// Exhaustively compute intersection_point for each pair of
// line_segments.  Then implement RANSAC-like rejection of outliers.
// Compute final intersection point using only inliers...

   int max_n_inliers=-1;
   vector<int> inlier_indices;
   for (int i=0; i<line_segments.size(); i++)
   {
      for (int j=i+1; j<line_segments.size(); j++)
      {
         vector<linesegment> two_line_segments;
         two_line_segments.push_back(line_segments[i]);
         two_line_segments.push_back(line_segments[j]);
         
         threevector candidate_intersection_point;
         geometry_func::multi_line_intersection_point(
            two_line_segments,candidate_intersection_point);

         vector<int> curr_inlier_indices;
         for (int c=0; c<n_cameras; c++)
         {
            camera* camera_ptr=camera_ptrs[c];
            threevector camera_posn=camera_ptr->get_world_posn();
            threevector n_hat=(candidate_intersection_point-camera_posn).
               unitvector();
            threevector r_hat=camera_ptr->pixel_ray_direction(feature_UVs[c]);
            double dotproduct=n_hat.dot(r_hat);
            double theta=acos(dotproduct);
            
// Declare any camera whose theta value exceeds 0.01 radian = 0.57 deg
// to be an outlier:

            const double max_theta=0.01;
            if (theta > max_theta) continue;
            
            curr_inlier_indices.push_back(c);
         } // loop over index c labeling cameras

         if (curr_inlier_indices.size() > inlier_indices.size())
         {
            inlier_indices.clear();
            for (int c=0; c<curr_inlier_indices.size(); c++)
            {
               inlier_indices.push_back(curr_inlier_indices[c]);
            }
         }

      } // loop over index j labeling 2nd line segment
   } // loop over index i labeling 1st line segment
   
   int n_inliers=inlier_indices.size();
   cout << "n_inliers = " << n_inliers
        << " n_cameras = " << n_cameras << endl;

   if (n_inliers <= 2)
   {
      cout << "Too few inlier rays found to yield reliable 3D intersection point..." 
           << endl;
      exit(-1);
   }

   vector<linesegment> inlier_line_segments;
   for (int i=0; i<n_inliers; i++)
   {
      int index=inlier_indices[i];
      inlier_line_segments.push_back(line_segments[index]);
   }
   threevector intersection_point;
   geometry_func::multi_line_intersection_point(inlier_line_segments,
   intersection_point);

// Add camera world COM back onto intersection_point:

   intersection_point += camera_world_COM;

   cout << endl;
   cout << "3D intersection point = "
        << intersection_point.get(0) << "  "
        << intersection_point.get(1) << "  "
        << intersection_point.get(2) << endl << endl;
   
}
