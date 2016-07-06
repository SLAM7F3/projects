// ========================================================================
// Program PLUMEVOLUME reads in calibration parameters for N <= 10
// fixed tripod cameras.  It instantiates a
// VolumetricCoincidenceProcessor volume whose lateral dimensions are
// set by the cameras' positions.  PLUMEVOLUME iterates over all
// voxels within the VCP volume and projects each into all 10 cameras'
// image planes.  If the voxel lies inside some minimal number of the
// cameras' smoke contours, it is marked with the number of such
// contours.  PLUMEVOLUME generates a TDP file for the point cloud
// associated with all of the marked voxels.
// ========================================================================
// Last updated on 12/14/11; 12/18/11; 12/19/11
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
   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << "image_sizes_filename = " << image_sizes_filename << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   int n_photos=photogroup_ptr->get_n_photos();

   cout << endl << endl;
   cout << "Number of tripod calibration photos  = " << n_photos << endl;

   const double u0=0.75;
   const double v0=0.5;
   double u,v;

   int n_visible_tripods;
   cout << "Enter minimum number of tripod cameras which must see smoke:" 
        << endl;
   cin >> n_visible_tripods;
   int n_invisible_tripods=n_photos-n_visible_tripods;

//   int n_invisible_tripods=0;
//   int n_invisible_tripods=1;
//   int n_invisible_tripods=3;

   double smoke_threshold_prob=0.8;
//   cout << "Enter smoke voxel threshold probability:" << endl;
//   cin >> smoke_threshold_prob;

   bounding_box bbox;
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

/*
// Generate FAKE set of masks for alg develop purposes only...

      string photo_filename=photo_ptr->get_filename();
      int width=photo_ptr->get_xdim();
      int height=photo_ptr->get_ydim();
      int n_images=1;
      int n_channels=3;
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         photo_filename,NULL);
      for (int px=0; px<width; px++)
      {
         for (int py=0; py<height; py++)
         {
            texture_rectangle_ptr->get_uv_coords(px,py,u,v);
            double sqrd_dist=sqr(u-u0)+sqr(v-v0);
            if (sqrd_dist > 0.2)
            {
               texture_rectangle_ptr->set_pixel_RGB_values(px,py,0,0,0);
            }
         } // loop over py index
      } // loop over px index

      string mask_filename="mask_"+stringfunc::integer_to_string(n,2)+".jpg";
      texture_rectangle_ptr->write_curr_frame(mask_filename);
      delete texture_rectangle_ptr;
      
      string banner="Exported "+mask_filename;
      outputfunc::write_banner(banner);
*/

   } // loop over index n labeling fixed tripod photos
   cout << "bbox = " << bbox << endl;

// Import masks for all tripod cameras.  Convert their contents
// into texture rectangles store within STL vector
// mask_texture_rectangle_ptrs:

   string mask_subdir="./masks/Nov_2011/Day5/5B/";
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   vector<string> mask_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,mask_subdir);

/*
//   string substring="MASK";
   vector<string> mask_filenames=
      filefunc::files_in_subdir_matching_substring(mask_subdir,substring);
*/

   vector<texture_rectangle*> mask_texture_rectangle_ptrs;
   for (int m=0; m<mask_filenames.size(); m++)
   {
      cout << "m = " << m << " mask filename = " << mask_filenames[m] << endl;
      texture_rectangle* mask_texture_rectangle_ptr=new texture_rectangle(
         mask_filenames[m],NULL);
      mask_texture_rectangle_ptrs.push_back(mask_texture_rectangle_ptr);
   }

   double xmin=bbox.get_xmin();
   double xmax=bbox.get_xmax();
   double ymin=bbox.get_ymin();
   double ymax=bbox.get_ymax();
   double zmin=0;
   double zmax=25;	// meters

   double voxel_binsize=0.1;	// meter
   double voxel_volume=voxel_binsize*voxel_binsize*voxel_binsize;

   VolumetricCoincidenceProcessor* VCP_ptr=NULL;

// ========================================================================
// Perform first round of smoke plume volume determination using
// coarse steps through VCP lattice.  After refining XYZ_min and
// XYZ_max, recompute plume volume using fine steps through smaller
// VPC lattice.

   int n_iters=2;
   int mstep=5;
   int nstep=5;
   int pstep=5;

   for (int iter=0; iter < n_iters; iter++)
   {
      if (iter > 0)
      {
         mstep=nstep=pstep=1;
      }

      string banner="Iteration "+stringfunc::number_to_string(iter)+
         " : mstep=nstep=pstep="+stringfunc::number_to_string(mstep);
      outputfunc::write_big_banner(banner);

// Instantiate VCP to hold 3D voxel lattice:

      delete VCP_ptr;
      VCP_ptr=new VolumetricCoincidenceProcessor();

      threevector XYZ_min(xmin,ymin,zmin);
      threevector XYZ_max(xmax,ymax,zmax);
      cout << "XYZ_min = " << XYZ_min << " XYZ_max = " << XYZ_max << endl;
      
      VCP_ptr->initialize_coord_system(XYZ_min,XYZ_max,voxel_binsize);
      cout << "VCP = " << *VCP_ptr << endl;

      xmin=POSITIVEINFINITY;
      xmax=NEGATIVEINFINITY;
      ymin=POSITIVEINFINITY;
      ymax=NEGATIVEINFINITY;
      zmin=POSITIVEINFINITY;
      zmax=NEGATIVEINFINITY;

      int mdim=VCP_ptr->get_mdim();
      int ndim=VCP_ptr->get_ndim();
      int pdim=VCP_ptr->get_pdim();
      int n_voxels=mdim*ndim*pdim;

// Iterate over all voxels in *VCP_ptr.  Project each voxel into all
// tripod cameras' image planes.  Mark those voxels whose projections
// land inside every image plane mask:

      int R,G,B;
      threevector voxel_posn;
      for (int m=0; m<mdim; m += mstep)
      {
         outputfunc::update_progress_fraction(m,0.05*mdim,mdim);
         double x=VCP_ptr->m_to_x(m);
         voxel_posn.put(0,x);
         for (int n=0; n<ndim; n += nstep)
         {
            double y=VCP_ptr->n_to_y(n);
            voxel_posn.put(1,y);

            for (int p=0; p<pdim; p += pstep)
            {
               double z=VCP_ptr->p_to_z(p);
               voxel_posn.put(2,z);

               double numer=0;
               double denom=0;
               int n_projs_inside=0;

               for (int c=0; c<camera_ptrs.size(); c++)
               {
                  camera* camera_ptr=camera_ptrs.at(c);

                  double curr_sqr_range=(
                     voxel_posn-camera_ptr->get_world_posn()).sqrd_magnitude();
                  double curr_weight=1/curr_sqr_range;
                  curr_weight=basic_math::min(1.0,curr_weight);
                  denom += curr_weight;

// Make sure threevector(x,y,z) lies in front of camera!

                  if (!camera_ptr->XYZ_in_front_of_camera(x,y,z)) continue;

                  camera_ptr->project_XYZ_to_UV_coordinates(x,y,z,u,v);
//               cout << "x = " << x << " y = " << y << " z = " << z
//                    << " u = " << u << " v = " << v << endl;

                  R=G=B=0;
                  mask_texture_rectangle_ptrs[c]->get_RGB_values(u,v,R,G,B);

                  if (R > 1 || G > 1 || B > 1)
                  {
                     double curr_prob=1.0;
                     numer += curr_prob*curr_weight;
                     n_projs_inside++;
                  }
               } // loop over index c labeling cameras

               if (n_projs_inside < camera_ptrs.size()-n_invisible_tripods) 
                  continue;

               double avg_smoke_prob=numer/denom;
//            cout << "avg_smoke_prob = " << avg_smoke_prob << endl;
//            if (avg_smoke_prob < smoke_threshold_prob) continue;

               long key=VCP_ptr->mnp_to_key(m,n,p);
               VCP_ptr->increment_voxel_counts(key,n_projs_inside);
//            VCP_ptr->set_voxel_prob(key,avg_smoke_prob);
//            VCP_ptr->set_voxel_prob(m,n,p,avg_smoke_prob);

               xmin=basic_math::min(xmin,x);
               xmax=basic_math::max(xmax,x);
               ymin=basic_math::min(ymin,y);
               ymax=basic_math::max(ymax,y);
               zmin=basic_math::min(zmin,z);
               zmax=basic_math::max(zmax,z);

            } // loop over index p
         } // loop over index n
//      cout << "VCP frac = " << double(VCP_ptr->size())/double(n_voxels)
//           << " volume = " << VCP_ptr->size()*voxel_volume << endl;

      } // loop over index m
      cout << endl;

      VCP_ptr->renormalize_counts_into_probs();
//      cout << "VCP = " << *VCP_ptr << endl;

   } // loop over iter index

// ========================================================================   

// Write out XYZP point cloud to TDP and OSGA files.  Calculate
// plume's total volume as product of its number of non-zero voxels
// times voxel_volume:

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
   cout << "Total plume volume = " << Z_ptr->size() * voxel_volume
        << " m**3 " << endl;
//   cout << "VCP volume frac = " << double(VCP_ptr->size())/double(n_voxels)
//        << endl;

   string tdp_filename="plume_hull_"+stringfunc::number_to_string(
      n_visible_tripods)+"_votes.tdp";
//      smoke_threshold_prob,3)+".tdp";

   tdpfunc::write_xyzp_data(
      tdp_filename,"",Zero_vector,X_ptr,Y_ptr,Z_ptr,P_ptr);
   string unix_cmd="lodtree "+tdp_filename;
   sysfunc::unix_command(unix_cmd);

   string banner="Finished writing TDP/OSGA file "+tdp_filename;
   outputfunc::write_big_banner(banner);
   
   delete X_ptr;
   delete Y_ptr;
   delete Z_ptr;
   delete P_ptr;

   for (int m=0; m<mask_texture_rectangle_ptrs.size(); m++)
   {
      delete mask_texture_rectangle_ptrs[m];
   }

}
