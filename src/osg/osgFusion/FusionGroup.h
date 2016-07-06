// ==========================================================================
// Header file for FUSIONGROUP class
// ==========================================================================
// Last modified on 7/18/11; 11/19/11; 12/26/11
// ==========================================================================

#ifndef FUSIONGROUP_H
#define FUSIONGROUP_H

#include <algorithm>
#include <iostream>
#include <osg/Array>
#include "video/camera.h"
#include "osg/osgSceneGraph/DataGraph.h"
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "osg/osgModels/ObsFrustaGroup.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgTiles/ray_tracer.h"
#include "osg/osgGeometry/RectanglesGroup.h"
#include "math/threevector.h"
#include "datastructures/Triple.h"
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

class AnimationController;
class FeaturesGroup;
class Movie;
class PointCloudsGroup;
class PolyLinesGroup;

class FusionGroup : public GraphicalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   FusionGroup(
      Pass* PI_ptr,PointCloud* cloud_ptr,Movie* Movie_ptr,
      AnimationController* AC_ptr,bool view_draped_video,
      bool init_cloud_members_flag=true);
   FusionGroup(
      PassesGroup* PG_ptr,Pass* PI_ptr,PointCloudsGroup* PCG_ptr,
      Movie* Movie_ptr,FeaturesGroup* FG2D_ptr,FeaturesGroup* FG3D_ptr,
      threevector* GO_ptr,AnimationController* AC_ptr);
   FusionGroup(
      PassesGroup* PG_ptr,Pass* PI_ptr,PointCloudsGroup* PCG_ptr,
      Movie* Movie_ptr,FeaturesGroup* FG2D_ptr,FeaturesGroup* FG3D_ptr,
      PolyLinesGroup* PLG2D_ptr,PolyLinesGroup* PLG3D_ptr,
      threevector* GO_ptr,AnimationController* AC_ptr);

   virtual ~FusionGroup();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const FusionGroup& f);

// Set & get methods:

   void set_orthographic_projection_flag(bool flag);
   void set_ObsFrustaGroup_ptr(ObsFrustaGroup* OFG_ptr);
   ObsFrustaGroup* get_ObsFrustaGroup_ptr();
   void set_Ground_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OFG_ptr);
   OBSFRUSTAGROUP* get_Ground_OBSFRUSTAGROUP_ptr();
   void set_sub_Ground_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OFG_ptr);
   void set_Aerial_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OFG_ptr);
   OBSFRUSTAGROUP* get_Aerial_OBSFRUSTAGROUP_ptr();
   void set_sub_Aerial_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OFG_ptr);
   void set_ground_photogroup_ptr(photogroup* pg_ptr);
   void set_air_photogroup_ptr(photogroup* pg_ptr);
   void set_PolyhedraGroup_ptr(PolyhedraGroup* PHG_ptr);
   void set_raytracer_ptr(ray_tracer* rt_ptr);
   void set_RectanglesGroup_ptr(RectanglesGroup* RG_ptr);

// Tiepoint fusion methods:

   void consolidate_XYZ_and_UV_feature_info();
   genmatrix* retrieve_XYZUV_feature_info(int imagenumber);
   genmatrix* retrieve_XYZABC_polyline_info();

   void update_XYZUV_feature_info(int imagenumber,const genmatrix* XYZUV_ptr);
   void output_all_tiepoint_data();
   void write_XYZUV_feature_info(double curr_t,std::string XYZUVfilename);
   void tiepoint_backprojection();
   void video_backprojection();
   void insert_2D_image_into_3D_worldspace();
   void constrained_image_insertion();

// Pointcloud coloring methods:

   void compute_backprojected_color_arrays();
   void backproject_videoframe_onto_pointcloud(
      int video_imagenumber,double s_weight=0.6);
   void backproject_videoframe_onto_pointcloud_with_shadowing(
      int video_imagenumber);
   void HSV_fuse(double Rcloud,double Gcloud,double Bcloud,
                 double saturation_weight,int& R,int& G,int& B);
   void HSV_fuse(double Rcloud,double Gcloud,double Bcloud,
                 int& R,int& G,int& B);
   void color_pointcloud_with_next_videoframe();
   void color_pointcloud_with_prev_videoframe();
   bool color_pointcloud(int video_imagenumber);
   void compute_and_save_shading_factors(int video_imagenumber);

// Video imagery scoring methods:

   int generate_gaussian_filters();
   void test_score();
   int get_backprojected_video_intensity(
      const camera* curr_camera_ptr,double X,double Y);
   double get_avg_backprojected_video_intensity(
      const camera* curr_camera_ptr,double X,double Y);
   double backprojected_video_intensity_Xderiv(
      const camera* curr_camera_ptr,double X,double Y,int deriv_order=1);
   double backprojected_video_intensity_Yderiv(
      const camera* curr_camera_ptr,double X,double Y,int deriv_order=1);
   void match_plaquettes(
      int curr_imagenumber,int next_imagenumber);
   double adjust_backprojected_feature_locations(
      int curr_imagenumber,int next_imagenumber);
   void construct_local_homography(
      const camera* curr_camera_ptr,double X,double Y,double dX,double dY,
      double& dU,double& dV,bool print_flag=false);

// Animation methods:

   void update_display();

// 2D rectangle backprojection into 3D member functions:
   
   polyhedron* backproject_2D_rectangle_into_3D();
   void project_ground_polyhedron_into_aerial_OBSFRUSTA();
   void fly_to_best_aerial_OBSFRUSTUM();
   void destroy_all_sub_aerial_OBSFRUSTA();

// WISP mask generation member functions:

   void generate_terrain_masks();
   void generate_terrain_masks(int photo_ID);

  protected:

  private:

   bool view_draped_video_flag,orthographic_projection_flag;
   unsigned int byte_counter;
   int start_imagenumber,stop_imagenumber;

// Note: As of Aug 2005, we do not know how to successfully read in
// files larger than 2 GByte in size using C++ ifstream calls.  So in
// order to accomodate very large color array files, we are forced to
// work with C-style FILE* pointers instead...

   FILE* fp_in;

   std::string color_arrays_filename;

   FeaturesGroup *FeaturesGroup_2D_ptr,*FeaturesGroup_3D_ptr;
   Movie* Movie_ptr;
   ObsFrustaGroup* ObsFrustaGroup_ptr;
   OBSFRUSTAGROUP *Ground_OBSFRUSTAGROUP_ptr,*sub_Ground_OBSFRUSTAGROUP_ptr;
   OBSFRUSTAGROUP *Aerial_OBSFRUSTAGROUP_ptr,*sub_Aerial_OBSFRUSTAGROUP_ptr;
   photogroup *ground_photogroup_ptr,*air_photogroup_ptr;
   PointCloudsGroup* PointCloudsGroup_ptr;
   PointCloud* PointCloud_ptr;
   PolyhedraGroup* PolyhedraGroup_ptr;
   PolyLinesGroup *PolyLinesGroup_2D_ptr,*PolyLinesGroup_3D_ptr;
   ray_tracer* ray_tracer_ptr;
   RectanglesGroup* RectanglesGroup_ptr;
   twoDarray* ztwoDarray_ptr;

   int filter_size;
   std::vector<double*> gaussian_filter;
   genmatrix* gaussian_2D_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const FusionGroup& f);

   int get_cloud_npoints();
   int get_color_array_size_in_bytes();

   void initialize_cloud_members();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void FusionGroup::set_orthographic_projection_flag(bool flag)
{
   orthographic_projection_flag=flag;
}

inline void FusionGroup::set_ObsFrustaGroup_ptr(ObsFrustaGroup* OFG_ptr)
{
   ObsFrustaGroup_ptr=OFG_ptr;
}

inline ObsFrustaGroup* FusionGroup::get_ObsFrustaGroup_ptr()
{
   return ObsFrustaGroup_ptr;
}

inline void FusionGroup::set_Ground_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OFG_ptr)
{
   Ground_OBSFRUSTAGROUP_ptr=OFG_ptr;
}

inline OBSFRUSTAGROUP* FusionGroup::get_Ground_OBSFRUSTAGROUP_ptr()
{
   return Ground_OBSFRUSTAGROUP_ptr;
}

inline void FusionGroup::set_sub_Ground_OBSFRUSTAGROUP_ptr(
   OBSFRUSTAGROUP* OFG_ptr)
{
   sub_Ground_OBSFRUSTAGROUP_ptr=OFG_ptr;
}

inline void FusionGroup::set_Aerial_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OFG_ptr)
{
   Aerial_OBSFRUSTAGROUP_ptr=OFG_ptr;
}

inline void FusionGroup::set_sub_Aerial_OBSFRUSTAGROUP_ptr(
   OBSFRUSTAGROUP* OFG_ptr)
{
   sub_Aerial_OBSFRUSTAGROUP_ptr=OFG_ptr;
}

inline OBSFRUSTAGROUP* FusionGroup::get_Aerial_OBSFRUSTAGROUP_ptr()
{
   return Aerial_OBSFRUSTAGROUP_ptr;
}

inline void FusionGroup::set_ground_photogroup_ptr(photogroup* pg_ptr)
{
   ground_photogroup_ptr=pg_ptr;
}

inline void FusionGroup::set_air_photogroup_ptr(photogroup* pg_ptr)
{
   air_photogroup_ptr=pg_ptr;
}

inline void FusionGroup::set_PolyhedraGroup_ptr(PolyhedraGroup* PHG_ptr)
{
   PolyhedraGroup_ptr=PHG_ptr;
}

inline void FusionGroup::set_raytracer_ptr(ray_tracer* rt_ptr)
{
   ray_tracer_ptr=rt_ptr;
}

inline void FusionGroup::set_RectanglesGroup_ptr(RectanglesGroup* RG_ptr)
{
   RectanglesGroup_ptr=RG_ptr;
}

inline int FusionGroup::get_cloud_npoints()
{
   return PointCloud_ptr->get_ntotal_leaf_vertices();
}

inline int FusionGroup::get_color_array_size_in_bytes()
{
   return 4*get_cloud_npoints();
}

#endif // FusionGroup.h



