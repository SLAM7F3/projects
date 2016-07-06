// ==========================================================================
// Header file for RAYTRACINGFUNCS namespace
// ==========================================================================
// Last modified on 7/1/11; 7/2/11; 7/9/11
// ==========================================================================

#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include "osg/osgTiles/TilesGroup.h"
#include "image/TwoDarray.h"

class ray_tracer
{

  public:

   ray_tracer();
   ~ray_tracer();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const ray_tracer& rt);

// Set & get member functions:

   bool get_ladar_height_data_flag() const;
   void set_max_zground(double z);
   void set_DTED_ptwoDarray_ptr(twoDarray* DTED_ptwoDarray_ptr);
   twoDarray* get_DTED_ptwoDarray_ptr();
   void set_TilesGroup_ptr(TilesGroup* TG_ptr);
   TilesGroup* get_TilesGroup_ptr();
   const TilesGroup* get_TilesGroup_ptr() const;
   
   int trace_individual_ray(
      int px,int py,const threevector& apex,double max_ground_Z,
      double max_raytrace_range,double ds,
      twoDarray* DTED_ztwoDarray_ptr,twoDarray* reduced_DTED_ztwoDarray_ptr);
   int trace_individual_ray(
      const threevector& apex,double curr_x,double curr_y,
      double max_ground_Z,double max_raytrace_range,double min_raytrace_range,
      double ds,twoDarray* DTED_ztwoDarray_ptr,
      twoDarray* reduced_DTED_ztwoDarray_ptr,threevector& occluded_ray_posn);

   int trace_individual_ray(
      const threevector& apex,double curr_x,double curr_y,
      double max_ground_Z,double max_raytrace_range,double min_raytrace_range,
      double ds,threevector& occluded_ray_posn);
   int trace_individual_ray(
      const threevector& apex,const threevector& rhat,
      double max_raytrace_range,double min_raytrace_range,
      double ds,threevector& occluded_ray_posn);
   int trace_individual_ray(
      const threevector& apex,const threevector& ground_point,
      double max_ground_Z,double max_raytrace_range,double min_raytrace_range,
      double ds,threevector& occluded_ray_posn);
   int ray_line_integral(
      const threevector& apex,double curr_x,double curr_y,
      double max_ground_Z,double ds);

// Region raytracing member functions:

   void raytrace_circle_around_ground_target(
      const threevector& apex,double max_radius,double ds,
      int& n_total_rays,int& n_occluded_rays);
   void ground_target_line_integrals(
      const threevector& apex,double max_radius,double ds);

  private:

   int prev_i_start,prev_i_stop;
   double interior_intensity_value,max_zground;
   twoDarray* DTED_ptwoDarray_ptr;   
   TilesGroup* TilesGroup_ptr;

   bool evaluate_segment_height(
      int i,int i_window,double curr_ray_x,double curr_ray_y,double curr_ray_z,
      double curr_max_ground_Z,twoDarray* DTED_ztwoDarray_ptr);
   bool evaluate_segment_height(
      double curr_ray_x,double curr_ray_y,double curr_ray_z,
      double curr_max_ground_Z);
   bool evaluate_segment_height(
      int i,int i_window,double curr_ray_x,double curr_ray_y,double curr_ray_z,
      double curr_max_ground_Z);

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline bool ray_tracer::get_ladar_height_data_flag() const
{
   return TilesGroup_ptr->get_ladar_height_data_flag();
}

inline void ray_tracer::set_max_zground(double z)
{
   max_zground=z;
}

inline void ray_tracer::set_DTED_ptwoDarray_ptr(twoDarray* DTED_ptwoDarray_ptr)
{
   this->DTED_ptwoDarray_ptr=DTED_ptwoDarray_ptr;
}

inline twoDarray* ray_tracer::get_DTED_ptwoDarray_ptr()
{
   return DTED_ptwoDarray_ptr;
}

inline void ray_tracer::set_TilesGroup_ptr(TilesGroup* TG_ptr)
{
   TilesGroup_ptr=TG_ptr;
}

inline TilesGroup* ray_tracer::get_TilesGroup_ptr()
{
   return TilesGroup_ptr;
}

inline const TilesGroup* ray_tracer::get_TilesGroup_ptr() const
{
   return TilesGroup_ptr;
}


#endif // ray_tracer.h



