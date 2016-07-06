// =========================================================================
// Header file for SATELLITEPASS class
// =========================================================================
// Last modified on 7/20/06; 7/28/06; 7/31/06; 8/22/06; 11/2/06; 10/8/11
// =========================================================================

#ifndef SATELLITEPASS_H
#define SATELLITEPASS_H

#include <list>
#include <set>
#include <string>
#include "datastructures/Linkedlist.h"
#include "space/motionfuncs.h"
#include "space/satelliteimage.h"
#include "space/satelliteorbit.h"
#include "math/statevector.h"
#include "math/threevector.h"

class AnimationController;
class geopoint;
class imagecdf;
class ground_radar;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

class satellitepass
{

  public:

// Initialization, constructor and destructor methods:

   satellitepass(std::string sat_name);
   satellitepass(const satellitepass& m);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~satellitepass();

   satellitepass& operator= (const satellitepass& m);

// Set and get member functions:

   bool get_left_handed_pass() const;
   satelliteorbit& get_orbit();
   std::vector<satelliteimage*>& get_satimage_ptrs();
   ground_radar* get_ground_radar_ptr();
   imagecdf* get_imagecdf_ptr();
   const imagecdf* get_imagecdf_ptr() const;
   void set_AnimationController_ptr(AnimationController* AC_ptr);

// Pass initialization member functions:

   bool initialize_pass(
      std::string filename,bool regularize_images,
      bool subsample_oversampled_images=false,bool clip_huge_images=false);
   void simplify_raw_images(
      bool regularize_images,bool subsampled_oversampled_images);
   void regularize_imagery_data();
   void downsample_oversampled_images(double min_deltax,double min_deltay);
   void allocate_and_initialize_working_twoDarrays();

   void compute_imagery_overlap();
   double average_time_interval_for_image(int i);
   void compute_physics_params_for_each_image();
   
// Basic astro and ECI coordinate system member functions:

   threevector compute_sun_direction(double currtime);
   double sidereal_angle(double currtime);

// Target & ground_radar state vector computation member functions:

   void compute_average_orbit_parameters();
   void determine_pass_handedness();
   statevector compute_ground_radar_statevector(
      double currtime,const geopoint& ground_radar_geoposn);

// Target elevation member functions:

   void plot_target_orbit_geometry_wrt_sensor();
   std::list<std::pair<double,threevector> > compute_positive_elevations(
      const threevector& init_ground_radar_posn,
      const threevector& init_target_posn);
   std::list<std::pair<double,threevector> > 
      generate_actual_image_az_el_range_list() const;

   void generate_actual_image_ranges_list(
      linkedlist& actual_images_list) const;
   void generate_actual_image_azimuths_list(
      linkedlist& actual_images_list) const;
   void generate_actual_image_elevations_list(
      linkedlist& actual_images_list) const;
   void plot_range_vs_time(linkedlist& full_range_list) const;
   void plot_azimuth_vs_time(linkedlist& full_azimuth_list) const;
   void plot_elevation_vs_time(linkedlist& full_elevation_list) const;
   void plot_elevation_vs_time(
      int imagenumber,linkedlist& full_elevation_list) const;

// Image frame computation member functions:

   void compute_imageplane_params();
   void adjust_crossrange_scale_for_sunsync_target();
   void compute_qnom_mags_and_earthstable_to_sunsync_ratios();
   void compute_earthstable_to_sunsync_scalefactors();
   void project_statevectors_into_imageplane();

// Pass output member functions:

   void write_videofile(bool delete_grey_video_flag=true,
                        bool equalize_intensity_histogram_flag=false);
   void write_imagecdf(bool equalize_intensity_histogram_flag=false);

  protected:

   bool left_handed_pass,nominal_pass;
   int pass_date; // Pass date.  Measured in integer number of seconds since 
		  // 1970-01-01 (Julian date = 2440587.5) till midnight of 
		  // pass collection day.
   int n_total_pixels;  // total # of pixels summed over all images in pass
   double crossover_elevation;      // Target crossover elevation in degs
   double t_rising,t_setting,time_target_above_horizon;	// secs

   imagecdf* imagecdf_ptr;
   ground_radar* ground_radar_ptr;
   motionfunc::Motion_type nominal_spacecraft_motion_type;
   satelliteorbit orbit;

// Satellitepass objects contain an STL vector of pointers to
// "working" satelliteimages which generally always exist after an
// imagecdf file is read in:
   
   std::vector<satelliteimage*> currimage_ptr;

  private:
   
   AnimationController* AnimationController_ptr;
   
   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const satellitepass& m);
};

// ==========================================================================
// Inlined methods
// ==========================================================================

// Set and get member functions

inline bool satellitepass::get_left_handed_pass() const
{
   return left_handed_pass;
}

inline satelliteorbit& satellitepass::get_orbit() 
{
   return orbit;
}

inline ground_radar* satellitepass::get_ground_radar_ptr() 
{
   return ground_radar_ptr;
}

inline std::vector<satelliteimage*>& satellitepass::get_satimage_ptrs()
{
   return currimage_ptr;
}

inline imagecdf* satellitepass::get_imagecdf_ptr()
{
   return imagecdf_ptr;
}

inline const imagecdf* satellitepass::get_imagecdf_ptr() const
{
   return imagecdf_ptr;
}

inline void satellitepass::set_AnimationController_ptr(
   AnimationController* AC_ptr)
{
   AnimationController_ptr=AC_ptr;
}


#endif // satellitepass.h

