// ========================================================================
// Program FIX_TEL_CONTOURS
// ========================================================================
// Last updated on 9/20/13
// ========================================================================

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "osg/osgGraphicals/AnimationController.h"
#include "models/Building.h"
#include "models/BuildingsGroup.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "math/rotation.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgGeometry/TrianglesGroup.h"
#include "osg/osgWindow/ViewerManager.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
//   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string contours_subdir=bundler_IO_subdir+"contours/";

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr,passes_group);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

// Import TEL contour:

   PolyLinesGroup* PolyLinesGroup_ptr=decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(0));
   string TEL_contour_filename;
   cout << "Enter TEL contour filename:" << endl;
   cin >> TEL_contour_filename;
   
   PolyLinesGroup_ptr->reconstruct_polylines_from_file_info(
      contours_subdir+TEL_contour_filename);

   threevector star_center(60.1208, 49.2312,1.6156);
   double phi=0;
//   cout << "Enter rotation angle phi in degs:" << endl;
//   cin >> phi;
//   phi *= PI/180;

   threevector n_hat(-0.9798235497 , 0.1998644826 , 0);
   threevector a_hat=z_hat.cross(n_hat);
//   cout << "n_hat = " << n_hat << endl;
//   cout << "a_hat = " << a_hat << endl;
   
   rotation R;
   R=R.rotation_about_nhat_by_theta(phi,a_hat);

/*
   string fixed_filename=contours_subdir+"fixed_"+TEL_contour_filename;
   ofstream vertexstream;
   filefunc::openfile(fixed_filename,vertexstream);
   vertexstream << "# Time   PolyLine_ID   Passnumber   X Y Z R G B A"
                << endl << endl;
*/

   int n_polylines=PolyLinesGroup_ptr->get_n_Graphicals();

   vector<double> delta_angles;
   for (int p=0; p<n_polylines; p++)
   {
      PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_PolyLine_ptr(p);
      polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
//      cout << "p = " << p << " polyline = " << *polyline_ptr << endl;
      int n_vertices=polyline_ptr->get_n_vertices();

      vector<threevector> contour_vertices;
      for (int v=0; v<n_vertices; v++)
      {
         threevector curr_vertex=polyline_ptr->get_vertex(v);
         threevector rel_vertex=curr_vertex-star_center;
         double n=rel_vertex.dot(n_hat);
         double a=rel_vertex.dot(a_hat);
         double z=rel_vertex.dot(z_hat);

         cout << "v = " << v << " n = " << n << " a = " << a
              << " z = " << z << endl;

      } // loop over index v labeling current polyline vertices
   } // loop over indx p labeling polylines

//   filefunc::closefile(fixed_filename,vertexstream);


}

