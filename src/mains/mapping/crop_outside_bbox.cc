// ========================================================================
// Program CROP_OUTSIDE_BBOX

//     crop_outside_bbox 20101025-130231-1064nm_317262-394792.xform_adj.tdp

// ========================================================================
// Last updated on 11/11/10; 11/12/10; 11/14/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CentersKeyHandler.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgGeometry/TrianglesGroup.h"
#include "osg/osgWindow/ViewerManager.h"
#include "threeDgraphics/xyzpfuncs.h"

#include "geometry/bounding_box.h"
#include "ladar/ladarimage.h"
#include "general/stringfuncs.h"
#include "osg/osg3D/tdpfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ifstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   
// Use an ArgumentParser object to manage the program arguments:
   
   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;

//   int p_map=2; 	// small_hue_value
//   int p_map=4;    	// large_hue_value_sans_white for Baghdad ground mask
//   int p_map=6;     	// grey scale
//   int p_map=8;	// wrap1
//   int p_map=9;	// wrap2 
//   int p_map=10;	// wrap3 for ALIRT-A and RTV New York map
//   cout << "Enter map number of height colormap:" << endl;
//   cin >> p_map;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("3D imagery");
   
// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_ptr,passes_group);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_ptr,window_mgr_ptr);
   CM_3D_ptr->set_min_camera_height_above_grid(0.1);	// meters
//   CM_3D_ptr->set_min_camera_height_above_grid(100);	// meters
   window_mgr_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_3D_ptr);

   AlirtGrid* grid_ptr=decorations.add_AlirtGrid(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));
   threevector* grid_origin_ptr=grid_ptr->get_world_origin_ptr();
   cout << "*grid_origin_ptr = " << *grid_origin_ptr << endl;
   CM_3D_ptr->set_Grid_ptr(grid_ptr);
   decorations.set_grid_origin_ptr(grid_origin_ptr);

// Instantiate triangle decorations group:

   decorations.add_Triangles(ndims,passes_group.get_pass_ptr(cloudpass_ID));
   decorations.add_Rectangles(
      ndims,passes_group.get_pass_ptr(cloudpass_ID));

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(
      passes_group.get_pass_ptr(cloudpass_ID),grid_origin_ptr);
   bool index_tree_flag=false;
   clouds_group.generate_Clouds(
      passes_group,index_tree_flag,decorations.get_TrianglesGroup_ptr());
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(
      new PointCloudKeyHandler(&clouds_group,ModeController_ptr,CM_3D_ptr));

// Read in bounding box for relatively noise-free region of ladar
// data:

//   string subdir="/media/5f11a671-48d4-4fe7-9997-b6e2d7070af0/ALIRT/Sortie23_tgts/20101025-131010-1064nm/osga/";
//   string subdir="/media/5f11a671-48d4-4fe7-9997-b6e2d7070af0/ALIRT/Sortie23_tgts/20101025-130724-1064nm/tdp/";
//   string subdir="/media/5f11a671-48d4-4fe7-9997-b6e2d7070af0/ALIRT/Sortie23_tgts/20101025-130231-1064nm/tdp/";
   string subdir="/media/5f11a671-48d4-4fe7-9997-b6e2d7070af0/ALIRT/Haiti/";

   string bounding_corners_filename=subdir+"corners.dat";
   filefunc::ReadInfile(bounding_corners_filename);
   vector<threevector> corners;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      corners.push_back(threevector(column_values[3],column_values[4],
	      column_values[5]));
      cout << "c = " << i << " corner = " << corners.back() << endl;
   }

   bounding_box bbox;
   bbox.inscribed_bbox(corners);
   
// Loop over all points within input TDP file.  Reject those which lie
// outside inscribed bbox.  Write out new TDP file for just inlier points:

// Load XYZP points into STL vector of fourvectors:

   osg::Vec3Array* filtered_vertices_ptr=new osg::Vec3Array;
   osg::FloatArray* filtered_probs_ptr=new osg::FloatArray;

   osg::FloatArray* probs_ptr=new osg::FloatArray;
   
   for (int p=0; p<clouds_group.get_n_Graphicals(); p++)
   {
      PointCloud* cloud_ptr=clouds_group.get_Cloud_ptr(p);
      osg::Vec3Array* vertices_ptr=cloud_ptr->get_vertices_ptr();
      model::Metadata* metadata_ptr=cloud_ptr->get_metadata_ptr();

      int n_points=cloud_ptr->get_npoints();
      cout << "n_points = " << n_points << endl;
      probs_ptr->clear();
      probs_ptr->reserve(n_points);
      
//      double xmin=POSITIVEINFINITY;
//      double xmax=NEGATIVEINFINITY;
//      double ymin=POSITIVEINFINITY;
//      double ymax=NEGATIVEINFINITY;
      
      int d_size=0.1*n_points;
      for (int i=0; i<n_points; i++)
      {
         if (i%d_size==0) cout << i/d_size << " " << flush;
         osg::Vec3 xyz_vertex=vertices_ptr->at(i);

         double curr_x=xyz_vertex.x();
         double curr_y=xyz_vertex.y();
         if (!bbox.XY_inside_WL_bbox(curr_x,curr_y)) continue;

//         xmin=min(xmin,xyz_vertex.x());
//         xmax=max(xmax,xyz_vertex.x());
//         ymin=min(ymin,xyz_vertex.y());
//         ymax=max(ymax,xyz_vertex.y());

         filtered_vertices_ptr->push_back(
            osg::Vec3(curr_x,curr_y,xyz_vertex.z()));
         filtered_probs_ptr->push_back(metadata_ptr->get(i,0));

      } // loop over index i labeling points within current cloud
      cout << endl;

//      cout << "xmin = " << xmin << " xmax = " << xmax << endl;
//      cout << "ymin = " << ymin << " ymax = " << ymax << endl;


/*
      double deltax=0.25;	// meter
      double deltay=0.25;	// meter
      ladarimage zp_image;
      zp_image.store_input_data(xmin,ymin,xmax,ymax,deltax,deltay,
         vertices_ptr,probs_ptr);

      threevector currpoint;
      twoDarray* ztwoDarray_ptr=zp_image.get_z2Darray_ptr();
      twoDarray* ptwoDarray_ptr=zp_image.get_p2Darray_ptr();
      for (int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
      {
         if (px%100==0) cout << px << " " << flush;
         for (int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
         {
            ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
            bool pixel_inside_poly=bounding_poly.point_inside_polygon(
               currpoint);

            if (pixel_inside_poly)
            {
               double z=ztwoDarray_ptr->get(px,py);
               float p=ptwoDarray_ptr->get(px,py);
               filtered_vertices_ptr->push_back(
                  osg::Vec3(currpoint.get(0),currpoint.get(1),z));
               filtered_probs_ptr->push_back(p);
            }
         } // loop over py index
      } // loop over px index
      cout << endl;
*/

      Pass* cloudpass_ptr=passes_group.get_pass_ptr(cloudpass_ID);
      string pass_filename=cloudpass_ptr->get_first_filename();
      cout << "pass_filename = " << pass_filename << endl;
      string prefix=stringfunc::prefix(pass_filename);
      cout << "prefix = " << prefix << endl;

      string tdp_filename=prefix+"_cropped.tdp";
      string UTMzone="";
      tdpfunc::write_relative_xyzp_data(
         tdp_filename,UTMzone,filtered_vertices_ptr,filtered_probs_ptr);

      string unix_cmd="lodtree "+tdp_filename;
      sysfunc::unix_command(unix_cmd);

//      double maximum_separation=1;	// meter
//      xyzpfunc::density_filter(maximum_separation,XYZP_ptr);

   } // loop over index p labeling point clouds



}

