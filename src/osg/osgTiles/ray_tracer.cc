// ==========================================================================
// RAY_TRACER class member function definitions
// ==========================================================================
// Last modified on 7/2/11; 7/3/11; 7/9/11
// ==========================================================================

#include <iostream>
#include "osg/osgTiles/ray_tracer.h"

using std::cout;
using std::endl;
using std::flush;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:

void ray_tracer::allocate_member_objects()
{
}

void ray_tracer::initialize_member_objects()
{
   prev_i_start=prev_i_stop=-1;
   interior_intensity_value=-0.5;
   max_zground=NEGATIVEINFINITY;
   DTED_ptwoDarray_ptr=NULL;
   TilesGroup_ptr=NULL;
}

ray_tracer::ray_tracer()
{
   allocate_member_objects();
   initialize_member_objects();
}

ray_tracer::~ray_tracer()
{
}
   
// ---------------------------------------------------------------------
// Member function trace_individual_ray() takes in the sensor's and
// ground target's location in apex and (px,py) in UTM coordinates.
// If the sensor's view of the ground point is occluded by some part
// of the terrain in *DTED_ztwoDarray_ptr, this method returns 0.  If
// the ground point is visible to the sensor, this method returns 1.
// If the ground point lies outside the sensor's field-of-view, this
// method returns -1.

int ray_tracer::trace_individual_ray(
   int px,int py,const threevector& apex,double max_ground_Z,
   double max_raytrace_range,double ds,
   twoDarray* DTED_ztwoDarray_ptr,twoDarray* reduced_DTED_ztwoDarray_ptr)
{
//   cout << "inside ray_tracer::trace_individual_ray()" << endl;
//   cout << "ds = " << ds << endl;

   get_DTED_ptwoDarray_ptr()->put(px,py,1);

// Form ray from ground_point to camera location.  Then check whether
// it exceeds max_raytrace_range:

   threevector ground_point;
   DTED_ztwoDarray_ptr->pixel_to_threevector(px,py,ground_point);

   double length=(apex-ground_point).magnitude();
   if (length > max_raytrace_range) return -1;

// Check whether ray is occluded:

   threevector e_hat( (apex-ground_point).unitvector() );

   double d_length = ds/sqrt(1-sqr(e_hat.get(2)));

   threevector d_length_e_hat(d_length*e_hat);
   int n_steps=static_cast<int>(length/d_length);

// If the previous call to trace_individual_ray() found that a ground
// obstacle occluded the ray somewhere along its path, there is a
// reasonable probability that the current ray will also be occluded
// in same vicinity.  So start ray tracing at the location of the
// prior ray's occlusion:

//   int i_min=2;
//   int i_min=3;
   int i_min=4;
//   int i_window=4;
   int i_window=5;
   const double SMALL_POSITIVE=0.001;
   double x,y,z;
   threevector curr_ray_posn;

   if (prev_i_start >= i_min+i_window)
   {
      curr_ray_posn=ground_point+prev_i_start*d_length_e_hat;
      x=curr_ray_posn.get(0);
      y=curr_ray_posn.get(1);
      z=curr_ray_posn.get(2);
      
      for (int i=prev_i_start; i<=basic_math::min(n_steps-1,prev_i_stop); i++)
      {
         if (evaluate_segment_height(
                i,i_window,x,y,z,max_ground_Z,DTED_ztwoDarray_ptr))
         {
            get_DTED_ptwoDarray_ptr()->put(px,py,SMALL_POSITIVE);
            return 0;
         }

         x += d_length_e_hat.get(0);
         y += d_length_e_hat.get(1);
         z += d_length_e_hat.get(2);
      } // loop over index i labeling steps along ray
   } // prev_i_start >= i_min+i_window conditional

// Start ray trace not at exact ground point location but rather a few
// steps along the ray in the air...

   double min_dist_from_ground_point=3;		// meters	
   if (i_min*d_length < min_dist_from_ground_point)
   {
      i_min=min_dist_from_ground_point/d_length;
   }

   curr_ray_posn=ground_point+i_min*d_length_e_hat;
   x=curr_ray_posn.get(0);
   y=curr_ray_posn.get(1);
   z=curr_ray_posn.get(2);
//   cout << "z = " << z << endl;

// Take large steps along ray to determine if it's occluded anywhere.
// If not, don't waste time evaluating ray occlusion along finer steps:

   int scale_factor=TilesGroup_ptr->get_terrain_reduction_scale_factor();
   threevector scaled_d_length_e_hat(scale_factor*d_length_e_hat);

   bool ray_occluded_flag=false;
   for (int i=scale_factor+i_min; i<n_steps && !ray_occluded_flag; 
        i += scale_factor)
   {
      ray_occluded_flag=evaluate_segment_height(
         i,i_window,x,y,z,max_ground_Z,reduced_DTED_ztwoDarray_ptr);
      if (ray_occluded_flag) break;

      x += scaled_d_length_e_hat.get(0);
      y += scaled_d_length_e_hat.get(1);
      z += scaled_d_length_e_hat.get(2);
   } // loop over index i labeling steps along ray

   if (!ray_occluded_flag) 
   {
//      cout << "Ray not occluded px = " << px << " py = " << py << endl;
      return 1;
   }

   curr_ray_posn=threevector(x,y,z);
   int i_start=basic_math::round(
      (curr_ray_posn-ground_point).magnitude()/d_length);

   for (int i=i_start; i<n_steps; i++)
   {
      if (evaluate_segment_height(
             i,i_window,x,y,z,max_ground_Z,DTED_ztwoDarray_ptr))
      {
         get_DTED_ptwoDarray_ptr()->put(px,py,SMALL_POSITIVE);
         return 0;
      }

      x += d_length_e_hat.get(0);
      y += d_length_e_hat.get(1);
      z += d_length_e_hat.get(2);

   } // loop over index i labeling steps along ray

   return 1;
}

// ---------------------------------------------------------------------
// This overloaded version of trace_individual_ray() is intended for
// use with individual ground target points which are already known to
// lie inside the OBSFRUSTUM's z-plane trapezoid:

int ray_tracer::trace_individual_ray(
   const threevector& apex,double curr_x,double curr_y,
   double max_ground_Z,double max_raytrace_range,double min_raytrace_range,
   double ds,twoDarray* DTED_ztwoDarray_ptr,
   twoDarray* reduced_DTED_ztwoDarray_ptr,threevector& occluded_ray_posn)
{
//   cout << "inside OBSFRUSTUM::trace_individual_ray()" << endl;
//   cout << "max_ground_z = " << max_ground_Z 
//        << " ds = " << ds << endl;
//   cout << "max_raytrace_range = " << max_raytrace_range
//        << " min_raytrace_range = " << min_raytrace_range << endl;

   unsigned int gx,gy;
   if (!DTED_ztwoDarray_ptr->point_to_pixel(curr_x,curr_y,gx,gy)) 
      return -1;

// Form ray from ground_point to camera location.  Then check whether
// it is occluded:

   double curr_z=DTED_ztwoDarray_ptr->get(gx,gy);

   double z_offset=2;	// meters
   if (get_ladar_height_data_flag())
   {
      z_offset=1.5;	// meters
   }

   threevector ground_point(curr_x,curr_y,curr_z+z_offset);
//   cout << "ground_point = " << ground_point << endl;

// Check ground target's visibility to aerial sensor:

   double length=(apex-ground_point).magnitude();
//   cout << "length = " << length << endl;
   if (length > max_raytrace_range || length < min_raytrace_range)
   {
      return -1;
   }

   threevector e_hat( (apex-ground_point).unitvector() );
   double d_length = ds/sqrt(1-sqr(e_hat.get(2)));
   threevector d_length_e_hat(d_length*e_hat);
   int n_steps=static_cast<int>(length/d_length);
//   cout << "n_steps = " << n_steps << endl;

// Start ray trace not at exact ground point location but rather a few
// steps along the ray in the air...

   int i_min=4;
   if (get_ladar_height_data_flag())
   {
      i_min=3;
   }

   threevector curr_ray_posn(ground_point+i_min*d_length_e_hat);
   double x=curr_ray_posn.get(0);
   double y=curr_ray_posn.get(1);
   double z=curr_ray_posn.get(2);
   double curr_max_ground_Z=max_ground_Z;

// Take large steps along ray to determine if it's occluded anywhere.
// If not, don't waste time evaluating ray occlusion along finer steps:

   int i_window=5; // unimportant param for this method

   int scale_factor=TilesGroup_ptr->get_terrain_reduction_scale_factor();
   threevector scaled_d_length_e_hat(scale_factor*d_length_e_hat);

   bool ray_occluded_flag=false;
   for (int i=scale_factor+i_min; i<n_steps && !ray_occluded_flag; 
        i += scale_factor)
   {
      ray_occluded_flag=evaluate_segment_height(
         i,i_window,x,y,z,curr_max_ground_Z,reduced_DTED_ztwoDarray_ptr);
      if (ray_occluded_flag) break;

      x += scaled_d_length_e_hat.get(0);
      y += scaled_d_length_e_hat.get(1);
      z += scaled_d_length_e_hat.get(2);
   } // loop over index i labeling steps along ray

   if (!ray_occluded_flag) 
   {
//      cout << "Ray not occluded" << endl;
      return 1;
   }

   curr_ray_posn=threevector(x,y,z);
   int i_start=basic_math::round(
      (curr_ray_posn-ground_point).magnitude()/d_length);
//   cout << "i_start = " << i_start << endl;

   bool occluded_ray_flag=false;
   for (int i=i_start; i<n_steps; i++)
   {
//      curr_ray_posn=threevector(x,y,z);

      if (evaluate_segment_height(
         i,i_window,x,y,z,curr_max_ground_Z,DTED_ztwoDarray_ptr))
      {
         occluded_ray_posn=threevector(x,y,z);
         occluded_ray_flag=true;
      }

      x += d_length_e_hat.get(0);
      y += d_length_e_hat.get(1);
      z += d_length_e_hat.get(2);

   } // loop over index i labeling steps along ray

   if (occluded_ray_flag)
   {
      return 0;
   }
   else
   {
      return 1;
   }
}

// ---------------------------------------------------------------------
// This overloaded version of trace_individual_ray() is intended for
// tracing (moving) ground targets within an ALIRT ladar map. It calls
// TilesGroup::get_ladar_z_given_xy() and does NOT use
// DTED_ztwoDarray_ptr.  We wrote this version for the red actor path
// problem of TOC11 as well as general point-to-point raytracing for
// the HAFB minimap.

int ray_tracer::trace_individual_ray(
   const threevector& apex,double curr_x,double curr_y,
   double max_ground_Z,double max_raytrace_range,double min_raytrace_range,
   double ds,threevector& occluded_ray_posn)
{

// Reconstruct ground point using height information from ladar data:

   if (nearly_equal(curr_x,0) || nearly_equal(curr_y,0))
   {
      cout << "inside ray_tracer::trace_individual_ray() #3" << endl;
      cout << "max_ground_z = " << max_ground_Z 
           << " ds = " << ds << endl;
      cout << "max_raytrace_range = " << max_raytrace_range
           << " min_raytrace_range = " << min_raytrace_range << endl;
      cout << "apex = " << apex << endl;
      cout << "curr_x = " << curr_x << " curr_y = " << curr_y << endl;
      outputfunc::enter_continue_char();
   }

   double curr_z;
   TilesGroup_ptr->get_ladar_z_given_xy(curr_x,curr_y,curr_z);
//   cout << "curr_x = " << curr_x << " curr_y = " << curr_y
//        << " curr_z = " << curr_z << endl;

   double z_offset=2;	// meters
   if (get_ladar_height_data_flag())
   {
      z_offset=1.5;	// meters
   }
   threevector ground_point(curr_x,curr_y,curr_z+z_offset);
//   cout << "ground_point = " << ground_point << endl;

   return trace_individual_ray(
      apex,ground_point,max_ground_Z,max_raytrace_range,min_raytrace_range,
      ds,occluded_ray_posn);
}

// ---------------------------------------------------------------------
// This next overloaded version of trace_individual_ray() is
// specifically intended for tracing rays from WISP's location to
// ground points around HAFB.  We therefore intentionally call the
// next version of trace_individual_ray() with ground_point playing
// the role of apex and vice-versa.

int ray_tracer::trace_individual_ray(
   const threevector& apex,const threevector& rhat,
   double max_raytrace_range,double min_raytrace_range,
   double ds,threevector& occluded_ray_posn)
{
//   cout << "inside ray_tracer::trace_individual_ray() #4" << endl;
//   cout << "apex = " << apex << " rhat = " << rhat << endl;
   threevector ground_point=apex+0.9*max_raytrace_range*rhat;
//   cout << "Ground_point = " << ground_point << endl;
   return trace_individual_ray(
      ground_point,apex,
      max_zground,max_raytrace_range,min_raytrace_range,
      ds,occluded_ray_posn);
}

// ---------------------------------------------------------------------
int ray_tracer::trace_individual_ray(
   const threevector& apex,const threevector& ground_point,
   double max_ground_Z,double max_raytrace_range,double min_raytrace_range,
   double ds,threevector& occluded_ray_posn)
{
//   cout << "inside ray_tracer::trace_individual_ray() #5" << endl;
//   cout << "ground_point = " << ground_point << endl;

// Check ground target's visibility to sensor:

   double length=(apex-ground_point).magnitude();
//   cout << "length = " << length << endl;
   if (length > max_raytrace_range || length < min_raytrace_range)
   {
      return -1;
   }

   threevector e_hat( (apex-ground_point).unitvector() );
   double d_length = ds/sqrt(1-sqr(e_hat.get(2)));
   threevector d_length_e_hat(d_length*e_hat);
   int n_steps=static_cast<int>(length/d_length);
//   cout << "n_steps = " << n_steps << endl;

// Start ray trace not at exact ground point location but rather a few
// steps along the ray in the air...

// FAKE FAKE:  Saturday, April 23, 2011 at 4:09 pm
// Experiment with i_min=3 rather than i_min=4 for OMNI raytracing...

   int i_min=4;
   if (get_ladar_height_data_flag())
   {
      i_min=2;
//      i_min=3;
   }

   threevector curr_ray_posn(ground_point+i_min*d_length_e_hat);
   double x=curr_ray_posn.get(0);
   double y=curr_ray_posn.get(1);
   double z=curr_ray_posn.get(2);

   bool occluded_ray_flag=false;
   for (int i=i_min; i<n_steps && !occluded_ray_flag; i++)
   {
//      cout << "i = " << i << " n_steps = " << n_steps << endl;
      if (z > max_ground_Z) break;
      
      if (evaluate_segment_height(x,y,z,max_ground_Z))
      {
         occluded_ray_posn=threevector(x,y,z);
         occluded_ray_flag=true;
      }

      x += d_length_e_hat.get(0);
      y += d_length_e_hat.get(1);
      z += d_length_e_hat.get(2);
   } // loop over index i labeling steps along ray

//   cout << "occluded_ray_flag = " << occluded_ray_flag << endl;

   if (occluded_ray_flag)
   {
      return 0;
   }
   else
   {
      return 1;
   }
}

// ---------------------------------------------------------------------
// Member function ray_line_integral() 

int ray_tracer::ray_line_integral(
   const threevector& apex,double curr_x,double curr_y,
   double max_ground_Z,double ds)
{
//   cout << "inside ray_tracer::ray_line_integral()" << endl;
//   cout << "max_ground_z = " << max_ground_Z 
//        << " ds = " << ds << endl;

// Form ray from ground_point to sensor location.  Then check whether
// it is occluded:

   double curr_z;
   TilesGroup_ptr->get_ladar_z_given_xy(curr_x,curr_y,curr_z);
   double z_offset=2;	// meters
   if (get_ladar_height_data_flag())
   {
      z_offset=1.5;	// meters
   }
   threevector ground_point(curr_x,curr_y,curr_z+z_offset);
//   cout << "ground_point = " << ground_point << endl;

   double length=(apex-ground_point).magnitude();
//   cout << "length = " << length << endl;
   threevector e_hat( (apex-ground_point).unitvector() );
   double d_length = ds/sqrt(1-sqr(e_hat.get(2)));
   threevector d_length_e_hat(d_length*e_hat);
   int n_steps=static_cast<int>(length/d_length);
//   cout << "n_steps = " << n_steps << endl;

// Start ray trace not at exact ground point location but rather a few
// steps along the ray in the air...

   int i_min=4;
   if (get_ladar_height_data_flag())
   {
      i_min=2;
   }

   threevector curr_ray_posn(ground_point+i_min*d_length_e_hat);
   double x=curr_ray_posn.get(0);
   double y=curr_ray_posn.get(1);
   double z=curr_ray_posn.get(2);

   int n_occluded_samples=0;
   for (int i=i_min; i<n_steps; i++)
   {
//      cout << "i = " << i << " n_steps = " << n_steps << endl;
      if (z > max_ground_Z) break;
      
      if (evaluate_segment_height(x,y,z,max_ground_Z))
      {
         n_occluded_samples++;
      }

      x += d_length_e_hat.get(0);
      y += d_length_e_hat.get(1);
      z += d_length_e_hat.get(2);

   } // loop over index i labeling steps along ray

   return n_occluded_samples;
}

// ---------------------------------------------------------------------
// Member function evaluate_segment_height() checks whether the curr
// ray segment lies above the maximal ground height.  If so, this
// boolean method returns false (indicating that the ray is not
// occluded by the ground terrain) and sets
// prev_i_start=prev_i_stop=-1 (indicating that the next ray to be
// traced has no a priori likely location where it will be occluded).
// Otherwise, this method checks whether the curr ray segment lies
// above the local ground height.  If so, it returns true indicating
// that the ray is occluded by the ground terrain.

bool ray_tracer::evaluate_segment_height(
   int i,int i_window,double curr_ray_x,double curr_ray_y,double curr_ray_z,
   double curr_max_ground_Z,twoDarray* DTED_ztwoDarray_ptr)
{
//   cout << "inside ray_tracer::evaluate_segment_height()" << endl;

   if (curr_ray_z > curr_max_ground_Z)
   {
      prev_i_start=prev_i_stop=-1;
      return false;
   }

   unsigned int px,py;
   if (!DTED_ztwoDarray_ptr->point_to_pixel(curr_ray_x,curr_ray_y,px,py))
   {
      return false;
   }

   double curr_ground_z=DTED_ztwoDarray_ptr->get(px,py);
//   double curr_ground_z=DTED_ztwoDarray_ptr->fast_XY_to_Z(
//      curr_ray_x,curr_ray_y);
//   cout << "curr_ground_z = " << curr_ground_z << endl;
//   cout << "curr_ray_z = " << curr_ray_z << endl;

   if (curr_ray_z < curr_ground_z)
   {
      prev_i_start=i-i_window;
      prev_i_stop=i+i_window;
      return true;
   } // curr_ray_z < curr_ground_z conditional

   return false;
}

// ---------------------------------------------------------------------
// This overloaded version of evaluate_segment_height() checks whether
// the curr ray segment lies above the maximal ground height. 
// It calls TilesGroup::get_ladar_z_given_xy() and does NOT use
// DTED_ztwoDarray_ptr.  We wrote this version for the red actor path
// problem of TOC11.

bool ray_tracer::evaluate_segment_height(
   double curr_ray_x,double curr_ray_y,double curr_ray_z,
   double curr_max_ground_Z)
{
//   cout << "inside ray_tracer::evaluate_segment_height() #2" << endl;

//   if (curr_ray_z > curr_max_ground_Z) return false;

   if (nearly_equal(curr_ray_x,0) || nearly_equal(curr_ray_y,0))
   {
      cout << "inside ray_tracer::evaluate_segment_height() #2()" << endl;
      cout << "curr_ray_x = " << curr_ray_x 
           << " curr_ray_y = " << curr_ray_y << endl;
      outputfunc::enter_continue_char();
   }

   double curr_ground_z;
   if (!TilesGroup_ptr->get_ladar_z_given_xy(
      curr_ray_x,curr_ray_y,curr_ground_z))
   {
      return false;
   }

//   cout << "curr_ground_z = " << curr_ground_z << endl;
//   cout << "curr_ray_z = " << curr_ray_z << endl;

   return (curr_ray_z < curr_ground_z);
}

// ---------------------------------------------------------------------
// Member function raytrace_circle_around_ground_target() takes in
// some sensor's location within input threevector apex along with a
// maximum 2D radius around the sensor to be raytraced.  It
// instantiates *DTED_ptwoDarray_ptr to hold -1, 0 or 1 values
// indicating ground locations lying outside maximum radius, occluded
// ground location, or ground location with clear line-of-sight to sensor.
// This method is designed to be called with ALIRT ladar data whose
// height information is NOT read in from *DTED_ztwoDarray_ptr.

void ray_tracer::raytrace_circle_around_ground_target(
   const threevector& apex,double max_radius,double ds,
   int& n_total_rays,int& n_occluded_rays)
{
   cout << "inside ray_tracer::raytrace_circle_around_ground_target()" << endl;

   double xmin=apex.get(0)-max_radius;
   double xmax=apex.get(0)+max_radius;
   double ymin=apex.get(1)-max_radius;
   double ymax=apex.get(1)+max_radius;
   
   int mdim=(xmax-xmin)/ds+1;
   int ndim=(ymax-ymin)/ds+1;
   xmax=xmin+(mdim-1)*ds;
   ymax=ymin+(ndim-1)*ds;

//   cout << "xmin = " << xmin << " xmax = " << xmax << endl;
//   cout << "ymin = " << ymin << " ymax = " << ymax << endl;
   cout << "mdim = " << mdim << " ndim = " << ndim << endl;
   double sqr_max_radius=sqr(max_radius);

   delete get_DTED_ptwoDarray_ptr();
   DTED_ptwoDarray_ptr=new twoDarray(mdim,ndim);
   DTED_ptwoDarray_ptr->init_coord_system(xmin,xmax,ymin,ymax);
   get_DTED_ptwoDarray_ptr()->initialize_values(-1);

//   threevector behind_Abldg(313585.394439 , 4703303.01661, 32.0586128235);
//   threevector near_flagpole(313569.961824 , 4703212.66104, 39.9953193665);

   threevector occluded_ray_posn(Zero_vector);
   for (int px=0; px<mdim; px++)
   {
      if (px%100==0) cout << px << " " << flush;
      double x=xmin+px*ds;
      
      for (int py=0; py<ndim; py++)
      {
         double y=ymin+py*ds;
         double rho_sqr=sqr(x-apex.get(0))+sqr(y-apex.get(1));
         if (rho_sqr > sqr_max_radius) continue;

         int tracing_result=trace_individual_ray(
            apex,x,y,max_zground,max_radius,0,ds,occluded_ray_posn);

         unsigned int qx,qy;
         get_DTED_ptwoDarray_ptr()->point_to_pixel(x,y,qx,qy);
         get_DTED_ptwoDarray_ptr()->put(qx,qy,tracing_result);

         n_total_rays++;
         if (tracing_result==0)
         {
            n_occluded_rays++;
         }
         else if (tracing_result==1)
         {
         }
      } // loop over py index
   } // loop over px index
   cout << endl;

   cout << "n_total_rays = " << n_total_rays << endl;
   cout << "n_occluded_rays = " << n_occluded_rays << endl;
}

// ---------------------------------------------------------------------
// Member function ground_target_line_integrals() takes in
// some sensor's location within input threevector apex along with a
// maximum 2D radius around the sensor to be raytraced.  It
// instantiates *DTED_ptwoDarray_ptr to hold -1 indicating ground
// locations lying outside a maximum radius or relative occlusion
// frequency values ranging over [0,1].  We wrote this specialized
// method at the request of G103 staff member Marko.  He wanted to
// generlize line-of-sight maps away from just binary visible/occluded
// plots.  This method is designed to be called with ALIRT ladar data
// whose height information is NOT read in from *DTED_ztwoDarray_ptr.

void ray_tracer::ground_target_line_integrals(
   const threevector& apex,double max_radius,double ds)
{
   cout << "inside ray_tracer::ground_target_line_integrals()" << endl;

   double xmin=apex.get(0)-max_radius;
   double xmax=apex.get(0)+max_radius;
   double ymin=apex.get(1)-max_radius;
   double ymax=apex.get(1)+max_radius;
   
   int mdim=(xmax-xmin)/ds+1;
   int ndim=(ymax-ymin)/ds+1;
   xmax=xmin+(mdim-1)*ds;
   ymax=ymin+(ndim-1)*ds;

//   cout << "xmin = " << xmin << " xmax = " << xmax << endl;
//   cout << "ymin = " << ymin << " ymax = " << ymax << endl;
   cout << "mdim = " << mdim << " ndim = " << ndim << endl;
   double sqr_max_radius=sqr(max_radius);

   delete get_DTED_ptwoDarray_ptr();
   DTED_ptwoDarray_ptr=new twoDarray(mdim,ndim);
   DTED_ptwoDarray_ptr->init_coord_system(xmin,xmax,ymin,ymax);
   get_DTED_ptwoDarray_ptr()->initialize_values(-1);

   for (int px=0; px<mdim; px++)
   {
      if (px%100==0) cout << px << " " << flush;
      double x=xmin+px*ds;
      
      for (int py=0; py<ndim; py++)
      {
         double y=ymin+py*ds;
         double rho_sqr=sqr(x-apex.get(0))+sqr(y-apex.get(1));
         if (rho_sqr > sqr_max_radius) continue;

         int n_occluded_samples=ray_line_integral(apex,x,y,max_zground,ds);

         unsigned int qx,qy;
         if (get_DTED_ptwoDarray_ptr()->point_to_pixel(x,y,qx,qy))
            get_DTED_ptwoDarray_ptr()->put(qx,qy,n_occluded_samples);

      } // loop over py index
   } // loop over px index
   cout << endl;

// If no obstructions are encountered along a line-of-sight, the intercepted
// ground point is colored green.  On the other hand, if more than
// max_n_occluded_samples occlusions are encountered along the
// line-of-sight, the intercept ground point is colored red.  Ground
// points with intermediate numbers of occlusions along their lines of
// sight are colored shades of orange and yellow.

   const double max_n_occluded_samples=40;  // red
//   double max_n_occluded_samples=get_DTED_ptwoDarray_ptr()->
//      maximum_value();
//   cout << "max_n_occluded_samples = " << max_n_occluded_samples << endl;

   for (int px=0; px<mdim; px++)
   {
      for (int py=0; py<ndim; py++)
      {
         double n_occluded_samples=get_DTED_ptwoDarray_ptr()->get(px,py);
         if (n_occluded_samples < 0) continue;
         double relative_occlusion_frequency=
            n_occluded_samples/max_n_occluded_samples;
         relative_occlusion_frequency=basic_math::min(
            relative_occlusion_frequency,1.0);
//         double relative_occlusion_frequency=n_occluded_samples/
//            max_n_occluded_samples;
         double relative_visibility_frequency=1-relative_occlusion_frequency;
         get_DTED_ptwoDarray_ptr()->put(px,py,relative_visibility_frequency);
      } // loop over py index
   } // loop over px index

}


