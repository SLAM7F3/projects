// ==========================================================================
// Program GROUNDPLANE


// 			groundplane

// ==========================================================================
// Last updated on 11/16/09; 12/4/10; 12/9/10
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "geometry/plane.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "math/prob_distribution.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "math/threevector.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
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

// Read input TDP file containing XYZ cloud information:

   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   string cloud_filename=
      passes_group.get_pass_ptr(cloudpass_ID)->get_first_filename();
   cout << "cloud_filename = " << cloud_filename << endl;
   string tdp_filename=cloud_filename;

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID));
   vector<PointCloud*>* PointCloud_ptrs_ptr=clouds_group.generate_Clouds(
      passes_group);

   PointCloud* PointCloud_ptr=PointCloud_ptrs_ptr->at(0);
   cout << "PointCloud_ptr = " << PointCloud_ptr << endl;
   osg::Vec3Array* vertices_ptr=PointCloud_ptr->get_vertices_ptr();
   osg::Vec4ubArray* colors_ptr=PointCloud_ptr->get_colors_ptr();

   vector<double> X,Y,Z;
   vector<int> R,G,B;
   vector<threevector> xyz_pnts;
   for (unsigned int i=0; i<vertices_ptr->size(); i++)
   {
      osg::Vec3 curr_xyz(vertices_ptr->at(i));
      xyz_pnts.push_back(threevector(curr_xyz.x(),curr_xyz.y(),curr_xyz.z()));

      osg::Vec4ub curr_RGBA(colors_ptr->at(i));
      R.push_back(stringfunc::unsigned_char_to_ascii_integer(
         curr_RGBA.r()));
      G.push_back(stringfunc::unsigned_char_to_ascii_integer(
         curr_RGBA.g()));
      B.push_back(stringfunc::unsigned_char_to_ascii_integer(
         curr_RGBA.b()));
   }

   plane ground_plane;
   ground_plane.estimate_ground_plane(xyz_pnts);

/*
   double theta=22.3549382716049*PI/180;
   double phi=304.027777777778*PI/180;
   threevector origin(-4.11581969261169,-2.3548898696894,-3.83287000656128);
   threevector n_hat(
      sin(theta)*cos(phi),sin(theta)*sin(phi),cos(theta));
   ground_plane=plane(n_hat,origin);
*/

   

   cout << "ground_plane = " << ground_plane << endl;
   threevector n_hat=ground_plane.get_nhat();
   threevector origin=ground_plane.get_origin();

   rotation Rot;
   Rot=Rot.rotation_taking_u_to_v(z_hat,n_hat);
   cout << "Rot = " << Rot << endl;
   rotation Rinv=Rot.transpose();

// Rotate all XYZ points about ground plane origin:

   for (unsigned int i=0; i<xyz_pnts.size(); i++)
   {
      threevector curr_XYZ=xyz_pnts[i];
      curr_XYZ=origin+Rinv*(curr_XYZ-origin);
      X.push_back(curr_XYZ.get(0));
      Y.push_back(curr_XYZ.get(1));
      Z.push_back(curr_XYZ.get(2));
   }
   
   string UTMzone="";
   tdp_filename="rotated_"+tdp_filename;
   cout << "tdp_filename = " << tdp_filename << endl;
   
   cout << "X.size() = " << X.size()
        << " Y.size() = " << Y.size()
        << " Z.size() = " << Z.size() << endl;
   cout << "R.size() = " << R.size()
        << " G.size() = " << G.size()
        << " B.size() = " << B.size() << endl;

   tdpfunc::write_relative_xyzrgba_data(
      UTMzone,tdp_filename,X,Y,Z,R,G,B);
}

