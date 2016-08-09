// ==========================================================================
// Header file for MOVIESGROUP class
// ==========================================================================
// Last modified on 1/16/12; 2/4/12; 4/24/12; 8/9/16
// ==========================================================================

#ifndef MOVIESGROUP_H
#define MOVIESGROUP_H

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <osg/Group>
#include <osg/Node>
#include "osg/Custom3DManipulator.h"
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "osg/osg2D/Movie.h"
#include "video/photogroup.h"
#include "osg/osgTiles/ray_tracer.h"
#include "image/TwoDarray.h"

class AnimationController;
class homography;
class Pass;
class PassesGroup;
class PolyLinesGroup;

namespace osgGeometry
{
   class PointsGroup;
   class Polygon;
   class PolygonsGroup;
}

class MoviesGroup : public GraphicalsGroup
{

  public:

   typedef std::map<twovector,std::string,lttwovector > PHOTO_FILENAMES_MAP;

// Independent twovector holds Movie ID & framenumber integers
// Dependent string holds associated photo filename

// Initialization, constructor and destructor functions:

   MoviesGroup(const int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr,
               osgGA::Custom3DManipulator* CM_ptr=NULL);
   MoviesGroup(const int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr,
               photogroup* pg_ptr);
   MoviesGroup(const int p_ndims,Pass* PI_ptr,
               osgGeometry::PointsGroup* PG_ptr,
               osgGeometry::PolygonsGroup* PolyGrp_ptr,
               AnimationController* AC_ptr);
   virtual ~MoviesGroup();
   void clear_all_camera_ptrs();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const MoviesGroup& M);

// Set & get methods:

   void set_hide_backfaces_flag(bool flag);
   void set_extract_frames_flag(bool flag);
   void set_play_photos_as_video_flag(bool flag);
   void set_aerial_video_frame_flag(bool flag);
   void set_first_framenumber_to_extract(int framenumber);
   void set_last_framenumber_to_extract(int framenumber);
   void set_framenumber_skip(int skip);
   void set_raytracer_ptr(ray_tracer* rt_ptr);

   Movie* get_Movie_ptr(int n) const;
   Movie* get_ID_labeled_Movie_ptr(int ID) const;
   Movie* get_selected_Movie_ptr();
   void set_static_camera_posn_offset(const threevector& posn_offset);
   const threevector& get_static_camera_posn_offset() const;
   void set_photogroup_ptr(photogroup* pg_ptr);
   void set_DTED_ztwoDarray_ptr(twoDarray* twoDarray_ptr);
   twoDarray* get_DTED_ztwoDarray_ptr();
   const twoDarray* get_DTED_ztwoDarray_ptr() const;
   double get_reduced_DTED_scale_factor() const;
   twoDarray* get_reduced_DTED_ztwoDarray_ptr();
   const twoDarray* get_reduced_DTED_ztwoDarray_ptr() const;
   void set_PolyLinesGroup_ptr(PolyLinesGroup* PolyLinesGroup_ptr);

// Movie creation and destruction methods:

   Movie* generate_new_Movie(
      const PassesGroup& passes_group,double alpha=1.0,int ID=-1);
   Movie* generate_new_Movie(
      std::string movie_filename,double alpha=1.0,int ID=-1,
      bool force_empty_movie_construction_flag=false);
   texture_rectangle* generate_new_texture_rectangle(
      std::string movie_filename);
   Movie* generate_new_Movie(
      texture_rectangle* texture_rectangle_ptr,double alpha=1.0,int ID=-1);
   void convert_photos_to_movies();
   bool destroy_Movie(Movie*& Movie_ptr);
   void destroy_all_Movies();

// Movie manipulation methods:

   void reset_Uscale();
   void reset_Vscale();
   void update_display();
   void move_z(double delta_z,int Movie_ID=0);
   double get_total_altitude(int Movie_ID=0);
   void import_latest_photo();

// Movie output methods:

   void save_to_file();

// 2D region selection methods:

   osgGeometry::Polygon* generate_convexhull_poly();
   osgGeometry::Polygon* generate_poly();
   osgGeometry::Polygon* generate_Copley_photo_intersection_polys();
   void null_region_outside_poly();

// Raytracing member functions:

   void identify_sky_pixels();
   void identify_ocean_pixels();
   void compute_reduced_DTED_ztwoDarray();

// Photo handling member functions:

   bool read_future_photo(bool prev_number_flag);
   std::string get_future_photo_filename(
      std::string photo_filename,bool prev_number_flag);
   void insert_photo_filename_into_map(
      int Movie_ID,int frame_number,std::string photo_filename);
   void load_current_photos();

   bool project_PolyLines_into_selected_aerial_video_frame();

// 3D decal display member functions:

   void display_decals();

  protected:

  private:

   bool hide_backfaces_flag,extract_frames_flag,play_photos_as_video_flag;
   bool aerial_video_frame_flag;
   int reduced_DTED_scale_factor;
   int prev_extracted_framenumber,first_framenumber_to_extract,
      last_framenumber_to_extract,framenumber_skip;
   double prev_diag;
   threevector static_camera_posn_offset;
   osgGeometry::PointsGroup* PointsGroup_ptr;
   osgGeometry::PolygonsGroup* PolygonsGroup_ptr;
   PolyLinesGroup* PolyLinesGroup_ptr;
   photogroup* photogroup_ptr;
   ray_tracer* ray_tracer_ptr;
   twoDarray *DTED_ztwoDarray_ptr,*reduced_DTED_ztwoDarray_ptr;

   osg::ref_ptr<osgGA::Custom3DManipulator> CM_3D_refptr;
   PHOTO_FILENAMES_MAP* photo_filenames_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const MoviesGroup& f);
   void display_scale_factors(threevector& scale);
   void reset_scale_factors(const threevector& scale);

// Message handling member functions:

   virtual bool parse_next_message_in_queue(message& curr_message);



   void reset_photo(int ID);
   
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void MoviesGroup::set_hide_backfaces_flag(bool flag)
{
   hide_backfaces_flag=flag;
}

inline void MoviesGroup::set_extract_frames_flag(bool flag)
{
   extract_frames_flag=flag;
}

inline void MoviesGroup::set_play_photos_as_video_flag(bool flag)
{
   play_photos_as_video_flag=flag;
}

inline void MoviesGroup::set_aerial_video_frame_flag(bool flag)
{
   aerial_video_frame_flag=flag;
}

// --------------------------------------------------------------------------
inline void MoviesGroup::set_first_framenumber_to_extract(int framenumber)
{
   first_framenumber_to_extract=framenumber;
}

inline void MoviesGroup::set_last_framenumber_to_extract(int framenumber)
{
   last_framenumber_to_extract=framenumber;
}

inline void MoviesGroup::set_framenumber_skip(int skip)
{
   framenumber_skip=skip;
}

// --------------------------------------------------------------------------
inline Movie* MoviesGroup::get_Movie_ptr(int n) const
{
   return dynamic_cast<Movie*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline Movie* MoviesGroup::get_ID_labeled_Movie_ptr(int ID) const
{
   return dynamic_cast<Movie*>(get_ID_labeled_Graphical_ptr(ID));
}

// --------------------------------------------------------------------------
inline Movie* MoviesGroup::get_selected_Movie_ptr() 
{
   int g=get_selected_Graphical_ID();
   if (g==-1) g=0;
   return get_Movie_ptr(g);
}

// --------------------------------------------------------------------------
inline void MoviesGroup::set_static_camera_posn_offset(
   const threevector& posn_offset)
{
   static_camera_posn_offset=posn_offset;
}

inline const threevector& MoviesGroup::get_static_camera_posn_offset() const
{
   return static_camera_posn_offset;
}

// --------------------------------------------------------------------------
inline void MoviesGroup::set_photogroup_ptr(photogroup* pg_ptr)
{
   photogroup_ptr=pg_ptr;
}

// --------------------------------------------------------------------------
inline void MoviesGroup::set_DTED_ztwoDarray_ptr(twoDarray* twoDarray_ptr)
{
   DTED_ztwoDarray_ptr=twoDarray_ptr;
}

inline twoDarray* MoviesGroup::get_DTED_ztwoDarray_ptr()
{
   return DTED_ztwoDarray_ptr;
}

inline const twoDarray* MoviesGroup::get_DTED_ztwoDarray_ptr() const
{
   return DTED_ztwoDarray_ptr;
}

inline double MoviesGroup::get_reduced_DTED_scale_factor() const
{
   return reduced_DTED_scale_factor;
}

inline twoDarray* MoviesGroup::get_reduced_DTED_ztwoDarray_ptr()
{
   return reduced_DTED_ztwoDarray_ptr;
}

inline const twoDarray* MoviesGroup::get_reduced_DTED_ztwoDarray_ptr() const
{
   return reduced_DTED_ztwoDarray_ptr;
}

inline void MoviesGroup::set_PolyLinesGroup_ptr(
   PolyLinesGroup* PolyLinesGroup_ptr)
{
   this->PolyLinesGroup_ptr=PolyLinesGroup_ptr;
}

inline void MoviesGroup::set_raytracer_ptr(ray_tracer* rt_ptr)
{
   ray_tracer_ptr=rt_ptr;
}


#endif // MoviesGroup.h




