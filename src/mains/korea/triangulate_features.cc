// ========================================================================
// Program TRIANGULATE_FEATURES imports a set of manually selected
// tiepoint pairs between 2 internet ground photos.  It also imports 7
// parameters for each ground camera from their package files
// generated via program WRITEPACKAGES.  Tiepoint pairs are
// backprojected into 3D worldspace.  White contours [or red/green
// rays] are exported to output text files which can be visualized via
// program VIEW_TEL.
// ========================================================================
// Last updated on 9/19/13; 9/20/13; 9/26/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/ModeController.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

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
   string features_subdir=bundler_IO_subdir+"features/";
   string features1_filename=features_subdir+"all_features_groundphoto1.txt";
   string features2_filename=features_subdir+"all_features_groundphoto2.txt";

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

//   cout << "*curr_camera_ptr = " << *curr_camera_ptr << endl;
//   cout << "*next_camera_ptr = " << *next_camera_ptr << endl;

   threevector curr_world_posn=curr_camera_ptr->get_world_posn();
   threevector next_world_posn=next_camera_ptr->get_world_posn();

// Import 2D tiepoint feature image-plane coordinates for image #1.
// Backproject 2D feature coordinates into 3D world-space via
// *curr_P_ptr:

   filefunc::ReadInfile(features1_filename);

   vector<threevector> curr_rhats,next_rhats;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double u=column_values[3];
      double v=column_values[4];
      curr_rhats.push_back(curr_camera_ptr->pixel_ray_direction(u,v));
//      cout << "i = " << i << " curr_rhat = " << curr_rhats.back() << endl;
   }
   int n_tiepoints=curr_rhats.size();

// Import 2D tiepoint feature image-plane coordinates for image #2:

   filefunc::ReadInfile(features2_filename);

   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double u=column_values[3];
      double v=column_values[4];
      next_rhats.push_back(next_camera_ptr->pixel_ray_direction(u,v));
//      cout << "i = " << i << " next_rhat = " << next_rhats.back() << endl;
   }

   string contours_subdir=bundler_IO_subdir+"contours/";
   filefunc::dircreate(contours_subdir);

   string contour_filename=contours_subdir+"TEL_contours.dat";
   ofstream contourstream;
   filefunc::openfile(contour_filename,contourstream);
   contourstream << "# Time   PolyLine_ID   Passnumber   X Y Z R G B A" 
                 << endl << endl;

   int f_skip=1;	// white TEL body contours
//   int f_skip=10;	// colored world-space rays
   int polyline_ID=0;
   int ray_ID=1000;
   double time=0;
   int passnumber=0;
   for (int f=0; f<n_tiepoints+1; f += f_skip)
   {
      int g=f%n_tiepoints;
      threevector curr_rhat=curr_rhats[g];
      threevector next_rhat=next_rhats[g];
      double dotprod=curr_rhat.dot(next_rhat);
      threevector term=(next_world_posn-curr_world_posn)/(1-sqr(dotprod));

// Compute points along curr and next rays which lie closest to each
// other:

      threevector curr_p=curr_world_posn+
         term.dot(curr_rhat-dotprod*next_rhat)*curr_rhat;
      threevector next_p=next_world_posn-
         term.dot(next_rhat-dotprod*curr_rhat)*next_rhat;
      threevector avg_p=0.5*(curr_p+next_p);

// Export white contours for TEL body determined via 3D triangulation:

      contourstream << time << " "
                    << polyline_ID << " "
                    << passnumber << "   "
                    << avg_p.get(0) << "  "
                    << avg_p.get(1) << "  "
                    << avg_p.get(2) << "  1 1 1 1"
                    << endl;
      

/*

// Export red and green rays from ground camera frusta to their common
// 3D world-space intersection points on the TEL body:

      contourstream << time << " "
                    << ray_ID << " "
                    << passnumber << "   "
                    << curr_world_posn.get(0) << "  "
                    << curr_world_posn.get(1) << "  "
                    << curr_world_posn.get(2) << "  "
                    << "  1 0 0 1"
                    << endl;

      contourstream << time << " "
                    << ray_ID << " "
                    << passnumber << "   "
                    << avg_p.get(0) << "  "
                    << avg_p.get(1) << "  "
                    << avg_p.get(2) << "  "
                    << "  1 0 0 1"
                    << endl;
      ray_ID++;
      
      contourstream << time << " "
                    << ray_ID << " "
                    << passnumber << "   "
                    << next_world_posn.get(0) << "  "
                    << next_world_posn.get(1) << "  "
                    << next_world_posn.get(2) << "  "
                    << "  0 1 0 1"
                    << endl;

      contourstream << time << " "
                    << ray_ID << " "
                    << passnumber << "   "
                    << avg_p.get(0) << "  "
                    << avg_p.get(1) << "  "
                    << avg_p.get(2) << "  "
                    << "  0 1 0 1"
                    << endl;
      ray_ID++;
*/

   } // loop over index f labeling tiepoint pairs

   filefunc::closefile(contour_filename,contourstream);

}
