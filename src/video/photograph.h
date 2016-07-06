// ==========================================================================
// Header file for photograph class
// ==========================================================================
// Last modified on 3/15/13; 12/27/13; 3/22/14; 6/7/14
// ==========================================================================

#ifndef PHOTOGRAPH_H
#define PHOTOGRAPH_H

#include <string>
#include <osg/Image>
#include "exif.hpp"

#include "video/camera.h"
#include "astro_geo/Clock.h"
#include "astro_geo/geopoint.h"
#include "graphs/node.h"
#include "math/rpy.h"
#include "math/twovector.h"
#include "image/TwoDarray.h"

class AnimationController;

class photograph : public node
{

  public:

   photograph(int id=-1);
   photograph(
      std::string filename,int id=-1,bool parse_exif_metadata_flag=false);
   photograph(std::string filename,int xdim,int ydim,int id=-1,
              bool parse_exif_metadata_flag=false);
   photograph(int id,std::string filename);

   photograph(const photograph& p);
   ~photograph();
   photograph& operator= (const photograph& p);
   friend std::ostream& operator<< 
      (std::ostream& outstream,photograph& p);

// Set and get methods:

   void set_filename(std::string filename);
   std::string get_filename() const;
   void set_URL(std::string url);
   std::string get_URL() const;

   void set_xdim(int xdim);
   unsigned int get_xdim() const;
   void set_ydim(int ydim);
   unsigned int get_ydim() const;

   double get_aspect_ratio() const;
   double get_minU() const;
   double get_maxU() const;
   double get_minV() const;
   double get_maxV() const;

   void set_UTM_zonenumber(int z);
   void set_n_matching_SIFT_features(int n);
   int get_n_matching_SIFT_features() const;
   void set_bundler_covariance_trace(double trace);
   double get_bundler_covariance_trace() const;

   double get_focal_length() const;
   void set_frustum_sidelength(double l);
   double get_frustum_sidelength() const;
   void set_movie_downrange_distance(double l);
   double get_movie_downrange_distance() const;

   std::string get_camera_make() const;
   std::string get_camera_model() const;
   twovector& get_focal_plane_array_size();
   const twovector& get_focal_plane_array_size() const;
   void set_geolocation(const geopoint& g);
   geopoint& get_geolocation();
   const geopoint& get_geolocation() const;
   
   Clock& get_clock();
   const Clock& get_clock() const;
   rpy& get_pointing();
   const rpy& get_pointing() const;
   camera* get_camera_ptr();
   const camera* get_camera_ptr() const;

   osg::Image* get_image_ptr();
   const osg::Image* get_image_ptr() const;

   void set_score(double s);
   double get_score() const;

// Data & metadata parsing methods:

   void read_image_from_file(std::string photo_filename);
   void read_image_from_file();
   void rescale_image_size(
      double horiz_scale_factor,double vert_scale_factor);
   bool parse_timestamp_exiftag();

// Camera parameter member functions:

   bool parse_Exif_metadata();
   void estimate_internal_camera_params(double FOV_u=NEGATIVEINFINITY);
   void export_camera_parameters(
      camera* input_camera_ptr,std::string filename_descriptor,
      std::string packages_subdir,double frustum_side_length=-1);
   bool reset_camera_parameters(
      double fu,double fv,double U0,double V0,
      double az,double el,double roll,const threevector& camera_posn);
   
  private: 
   
   unsigned int xdim,ydim;
   int n_matching_SIFT_features,UTM_zonenumber;
   double focal_length; 	// in millimeters
   double min_U,max_U,min_V,max_V;
   double frustum_sidelength,movie_downrange_distance;
   double bundler_covariance_trace;
   double score;
   std::string filename,URL;
   std::string camera_make,camera_model;
   twovector focal_plane_array_size;
   geopoint geolocation;
   Clock clock;
   rpy pointing;
   camera* camera_ptr;
   osg::ref_ptr<osg::Image> image_refptr;


   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const photograph& p);

   void parse_internal_params_metadata(Exiv2::ExifData& exifData);
   void set_focal_plane_array_size();
   void parse_geolocation_metadata(Exiv2::ExifData& exifData);
   void parse_timestamp_metadata(Exiv2::ExifData& exifData);
   void parse_pointing_metadata(Exiv2::ExifData& exifData);

   void compute_UV_bounds();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline unsigned int photograph::get_xdim() const
{
   return xdim;
}

inline void photograph::set_xdim(int xdim) 
{
   this->xdim=xdim;
}

inline unsigned int photograph::get_ydim() const
{
   return ydim;
}

inline void photograph::set_ydim(int ydim) 
{
   this->ydim=ydim;
}

inline double photograph::get_aspect_ratio() const
{
   return get_maxU();
}

inline double photograph::get_minU() const
{
   return min_U;
}

inline double photograph::get_maxU() const
{
   return max_U;
}

inline double photograph::get_minV() const
{
   return min_V;
}

inline double photograph::get_maxV() const
{
   return max_V;
}

inline void photograph::set_UTM_zonenumber(int z)
{
   UTM_zonenumber=z;
}

inline void photograph::set_n_matching_SIFT_features(int n)
{
   n_matching_SIFT_features=n;
}

inline int photograph::get_n_matching_SIFT_features() const
{
   return n_matching_SIFT_features;
}

inline void photograph::set_bundler_covariance_trace(double trace)
{
   bundler_covariance_trace=trace;
}

inline double photograph::get_bundler_covariance_trace() const
{
   return bundler_covariance_trace;
}

inline double photograph::get_focal_length() const
{
   return focal_length;
}

inline void photograph::set_frustum_sidelength(double l)
{
   frustum_sidelength=l;
}

inline double photograph::get_frustum_sidelength() const
{
   return frustum_sidelength;
}

inline void photograph::set_movie_downrange_distance(double l)
{
   movie_downrange_distance=l;
}

inline double photograph::get_movie_downrange_distance() const
{
   return movie_downrange_distance;
}

inline void photograph::set_filename(std::string filename)
{
   this->filename=filename;
}

inline std::string photograph::get_filename() const
{
   return filename;
}

inline void photograph::set_URL(std::string url)
{
   URL=url;
}

inline std::string photograph::get_URL() const
{
   return URL;
}

inline std::string photograph::get_camera_make() const
{
   return camera_make;
}

inline std::string photograph::get_camera_model() const
{
   return camera_model;
}

inline void photograph::set_geolocation(const geopoint& g)
{
   geolocation=g;
}

inline geopoint& photograph::get_geolocation() 
{
   return geolocation;
}

inline const geopoint& photograph::get_geolocation() const
{
   return geolocation;
}

inline Clock& photograph::get_clock() 
{
   return clock;
}

inline const Clock& photograph::get_clock() const
{
   return clock;
}

inline rpy& photograph::get_pointing() 
{
   return pointing;
}

inline const rpy& photograph::get_pointing() const
{
   return pointing;
}

inline camera* photograph::get_camera_ptr()
{
   return camera_ptr;
}

inline const camera* photograph::get_camera_ptr() const
{
   return camera_ptr;
}

inline osg::Image* photograph::get_image_ptr()
{
   return image_refptr.get();
}

inline const osg::Image* photograph::get_image_ptr() const
{
   return image_refptr.get();
}

inline void photograph::set_score(double s)
{
   score=s;
}

inline double photograph::get_score() const
{
   return score;
}


#endif  // photograph.h
