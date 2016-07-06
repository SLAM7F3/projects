// ==========================================================================
// Header file for ObsFrustum class
// ==========================================================================
// Last updated on 6/20/07; 9/25/07; 10/13/07
// ==========================================================================

#ifndef ObsFrustum_H
#define ObsFrustum_H

#include <iostream>
#include <vector>
#include <osg/Group>
#include "osg/osgGeometry/Geometrical.h"
#include "osg/osg2D/Movie.h"
#include "passes/PassesGroup.h"
#include "geometry/polyhedron.h"

class AnimationController;
class LineSegmentsGroup;
class Model;
class ModelsGroup;
class polygon;

class ObsFrustum : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   ObsFrustum(
      Pass* PI_ptr,double az_extent,double el_extent,
      const threevector& grid_world_origin,
      AnimationController* AC_ptr,Movie* Movie_ptr=NULL,int id=-1);
   ObsFrustum(
      Pass* PI_ptr,const threevector& grid_world_origin,
      AnimationController* AC_ptr,Movie* Movie_ptr=NULL,int id=-1);

   virtual ~ObsFrustum();
   friend std::ostream& operator<< (
      std::ostream& outstream,const ObsFrustum& f);

// Set & get methods:

   void set_rectangular_movie_flag(bool flag);
   bool get_rectangular_movie_flag() const;
   void set_virtual_camera_flag(bool flag);
   bool get_virtual_camera_flag() const;
   void set_display_camera_model_flag(bool flag);

   threevector get_footprint_corner(int c);
   void set_Movie_ptr(Movie* Movie_ptr);
   Movie* get_Movie_ptr();
   LineSegmentsGroup* get_LineSegmentsGroup_ptr();
   const LineSegmentsGroup* get_LineSegmentsGroup_ptr() const;
   ModelsGroup* get_ModelsGroup_ptr();
   const ModelsGroup* get_ModelsGroup_ptr() const;
   polyhedron* get_polyhedron_ptr();
   const polyhedron* get_polyhedron_ptr() const;

// Frustum construction and manipulation methods:

   void compute_aerial_and_footprint_corners(
      double z_ground,const threevector& camera_posn,
      const std::vector<threevector>& UV_corner_dir,double max_lambda=10000);

   void build_current_frustum(
      double curr_t,int pass_number,const threevector& V,
      const threevector& n_hat,double z_offset);
   void build_current_frustum(
      double curr_t,int pass_number,double z_offset,
      const threevector& camera_posn,
      const std::vector<threevector>& UV_corner_dir,
      double max_lambda=10000);
   void build_frustum_with_movie(
      double curr_t,int pass_number,double z_offset,
      double movie_downrange_distance,double max_lambda=10000);
   virtual void set_color(const osg::Vec4& color);

// Translation & rotation methods:

   void absolute_position_and_orientation(
      double curr_t,int pass_number,
      const threevector& absolute_position,double az,double el,double roll,
      bool recompute_internal_params_flag=false);

// Footprint member functions:

   polygon GMTI_dwell_frustum_footprint(
      double curr_t,int pass_number,const threevector& V,const threevector& G,
      double range_extent,double crossrange_extent,double z_offset);
   polygon reconstruct_footprint(double curr_t,int pass_number);

// Polyhedron generation member functions:

   polyhedron* generate_curr_polyhedron(double curr_t,int pass_number);

  protected:

  private:

   bool rectangular_movie_flag,virtual_camera_flag;
   bool display_camera_model_flag;
   double az_extent,el_extent;
   threevector grid_world_origin;
   std::vector<threevector> corner_ray,footprint_corner,aerial_corner;
   polyhedron* polyhedron_ptr;
   Movie* Movie_ptr;
   LineSegmentsGroup* LineSegmentsGroup_ptr;
   ModelsGroup* ModelsGroup_ptr;
   Model* camera_model_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ObsFrustum& f);

   void initialize_linesegments();
   void rotate_about_camera_posn(
      double curr_t,int pass_number,double az,double el,double roll,
      bool recompute_internal_params_flag);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ObsFrustum::set_rectangular_movie_flag(bool flag)
{
   rectangular_movie_flag=flag;
}

inline bool ObsFrustum::get_rectangular_movie_flag() const
{
   return rectangular_movie_flag;
}

inline void ObsFrustum::set_virtual_camera_flag(bool flag)
{
   virtual_camera_flag=flag;
}

inline bool ObsFrustum::get_virtual_camera_flag() const
{
   return virtual_camera_flag;
}

inline void ObsFrustum::set_display_camera_model_flag(bool flag)
{
   display_camera_model_flag=flag;
}

inline threevector ObsFrustum::get_footprint_corner(int c)
{
   return footprint_corner[c];
}

// Note sure if we can get rid of next method...it's called by 
// ObsFrustaGroup::generate_HAFB_movie_ObsFrustum()

inline void ObsFrustum::set_Movie_ptr(Movie* Movie_ptr)
{
   this->Movie_ptr=Movie_ptr;
}

inline Movie* ObsFrustum::get_Movie_ptr()
{
   return Movie_ptr;
}

inline LineSegmentsGroup* ObsFrustum::get_LineSegmentsGroup_ptr()
{
   return LineSegmentsGroup_ptr;
}

inline const LineSegmentsGroup* ObsFrustum::get_LineSegmentsGroup_ptr() 
   const
{
   return LineSegmentsGroup_ptr;
}

inline ModelsGroup* ObsFrustum::get_ModelsGroup_ptr()
{
   return ModelsGroup_ptr;
}

inline const ModelsGroup* ObsFrustum::get_ModelsGroup_ptr() const
{
   return ModelsGroup_ptr;
}

inline polyhedron* ObsFrustum::get_polyhedron_ptr()
{
   return polyhedron_ptr;
}

inline const polyhedron* ObsFrustum::get_polyhedron_ptr() const
{
   return polyhedron_ptr;
}

#endif // ObsFrustum.h



