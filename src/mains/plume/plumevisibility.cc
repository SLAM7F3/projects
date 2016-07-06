// ========================================================================
// Program PLUMEVISIBILITY reads in calibration parameters for the 10
// fixed tripod cameras.  It instantiates a
// VolumetricCoincidenceProcessor volume whose lateral dimensions are
// set by the cameras' positions.  PLUMEVISIBILITY iterates over all
// voxels within the VCP volume and projects each into all 10 cameras'
// image planes.  If the voxel lies inside some specified number of
// the cameras' fields-of-view, it is added to the VCP.  PLUMEVISIBILITY
// generates a TDP file for the point cloud associated with all VCP
// voxels.

// We wrote this minor variant of PLUMEVOLUME in order to visualize
// the 3D volume visible to various numbers of the tripod cameras.

// ========================================================================
// Last updated on 11/30/11; 12/14/11
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <osgDB/FileUtils>

#include "osg/osgGraphicals/AnimationController.h"
#include "geometry/bounding_box.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_database.h"
#include "osg/osg3D/tdpfuncs.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"
#include "osg/osgWindow/ViewerManager.h"
#include "coincidence_processing/VolumetricCoincidenceProcessor.h"

#include "general/outputfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   int texturepass_ID=passes_group.get_curr_texturepass_ID();

   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << "image_sizes_filename = " << image_sizes_filename << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   const double u0=0.75;
   const double v0=0.5;
   double u,v;

   int n_visible_tripods;
   cout << "Enter minimum number of visible tripods:" << endl;
   cin >> n_visible_tripods;
   int n_invisible_tripods=n_photos-n_visible_tripods;

   bounding_box bbox(
      POSITIVEINFINITY,NEGATIVEINFINITY,
      POSITIVEINFINITY,NEGATIVEINFINITY,
      POSITIVEINFINITY,NEGATIVEINFINITY);
   
   vector<camera*> camera_ptrs;
   for (int n=0; n<n_photos; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
//      cout << "n = " << n << " *photo_ptr = " << *photo_ptr << endl;
      camera* camera_ptr=photo_ptr->get_camera_ptr();
//      cout << "n = " << n
//           << " camera = " << *camera_ptr << endl;
      camera_ptrs.push_back(camera_ptr);
      threevector camera_posn=camera_ptr->get_world_posn();
      bbox.update_bounds(camera_posn);
   } // loop over index n labeling fixed tripod photos
   cout << "bbox = " << bbox << endl;
   
// Instantiate VCP to hold 3D voxel lattice:

   VolumetricCoincidenceProcessor* VCP_ptr=
      new VolumetricCoincidenceProcessor();

//   const double zmin=0;
   const double zmax=50;	// meters
   threevector XYZ_min(bbox.get_xmin(),bbox.get_ymin(),bbox.get_zmin());
   threevector XYZ_max(bbox.get_xmax(),bbox.get_ymax(),zmax);

   cout << "XYZ_min = " << XYZ_min << " XYZ_max = " << XYZ_max << endl;
   const double binsize=0.1;	// meter
//   const double binsize=0.2;	// meter
//   const double binsize=0.25;	// meter
   VCP_ptr->initialize_coord_system(XYZ_min,XYZ_max,binsize);
   cout << "VCP = " << *VCP_ptr << endl;

// Iterate over all voxels in *VCP_ptr.  Project each voxel into all
// tripod cameras' image planes.  Mark those voxels whose projections
// land inside every image plane mask:

   int R,G,B;
   int mdim=VCP_ptr->get_mdim();
   for (int m=0; m<VCP_ptr->get_mdim(); m++)
   {
      outputfunc::update_progress_fraction(m,0.05*mdim,mdim);
      double x=VCP_ptr->m_to_x(m);
      for (int n=0; n<VCP_ptr->get_ndim(); n++)
      {
         double y=VCP_ptr->n_to_y(n);
         for (int p=0; p<VCP_ptr->get_pdim(); p++)
         {
            double z=VCP_ptr->p_to_z(p);

            int n_projs_inside=0;
            for (int c=0; c<n_photos; c++)
            {
               camera* camera_ptr=camera_ptrs.at(c);

// Make sure threevector(x,y,z) lies in front of camera!

               if (!camera_ptr->XYZ_in_front_of_camera(x,y,z)) continue;

               camera_ptr->project_XYZ_to_UV_coordinates(x,y,z,u,v);
//               cout << "x = " << x << " y = " << y << " z = " << z
//                    << " u = " << u << " v = " << v << endl;

               bounding_box* UV_bbox_ptr=camera_ptr->get_UV_bbox_ptr();
               if (!UV_bbox_ptr->point_inside(u,v)) continue;
               n_projs_inside++;
            } // loop over index c labeling cameras

            if (n_projs_inside <= 0) continue;
            if (n_projs_inside < n_photos-n_invisible_tripods) continue;

            long key=VCP_ptr->mnp_to_key(m,n,p);
            VCP_ptr->increment_voxel_counts(key,n_projs_inside);

         } // loop over index p
      } // loop over index n
   } // loop over index m
   cout << endl;

   VCP_ptr->renormalize_counts_into_probs();
   cout << "VCP = " << *VCP_ptr << endl;

// Write out XYZP point cloud to TDP and OSGA files.  Calculate
// plume's total volume as product of its number of non-zero voxels
// times voxel (binsize)**3:

   double min_prob_threshold=0;
//   bool perturb_voxels_flag=false;
   bool perturb_voxels_flag=true;
   vector<double>* X_ptr=new vector<double>;
   vector<double>* Y_ptr=new vector<double>;
   vector<double>* Z_ptr=new vector<double>;
   vector<double>* P_ptr=new vector<double>;

   int npoints=VCP_ptr->size();
   X_ptr->reserve(npoints);
   Y_ptr->reserve(npoints);
   Z_ptr->reserve(npoints);
   P_ptr->reserve(npoints);

   VCP_ptr->retrieve_XYZP_points(
      X_ptr,Y_ptr,Z_ptr,P_ptr,min_prob_threshold,perturb_voxels_flag);

   cout << "npoints = " << npoints << endl;
   cout << "Z_ptr->size() = " << Z_ptr->size() << endl;
   cout << "Total plume volume = " << Z_ptr->size() * binsize*binsize*binsize
        << " m**3 " << endl;

   string tdp_filename="visible_volume_"+stringfunc::number_to_string(
      n_visible_tripods)+"cam.tdp";
   tdpfunc::write_xyzp_data(tdp_filename,X_ptr,Y_ptr,Z_ptr,P_ptr);
   string unix_cmd="lodtree "+tdp_filename;
   sysfunc::unix_command(unix_cmd);

   delete X_ptr;
   delete Y_ptr;
   delete Z_ptr;
   delete P_ptr;

}
