// ==========================================================================
// Header file for EARTHREGIONSGROUP class
// ==========================================================================
// Last modified on 5/7/09; 5/25/09; 6/5/09; 8/14/09
// ==========================================================================

#ifndef EARTHREGIONSGROUP_H
#define EARTHREGIONSGROUP_H

#include <osg/MatrixTransform>
#include <osg/Node>
#include "color/colorfuncs.h"
#include "osg/osgGeometry/CylindersGroup.h"
#include "osg/osgEarth/EarthRegion.h"
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "passes/PassesGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "track/tracks_group.h"

class Earth;
class geopoint;
class LatLongGridsGroup;
class Movie;
class MoviesGroup;
class PassInfo;
class PointCloudsGroup;
class polyline;
class PolyLinesGroup;
class RegionPolyLinesGroup;
class texture_rectangle;
class TextureSectorsGroup;

class PointFinder;
class PointCloud;

class EarthRegionsGroup : public GeometricalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   EarthRegionsGroup(Pass* PI_ptr,Earth* E_ptr=NULL);
   EarthRegionsGroup(
      Pass* PI_ptr,LatLongGridsGroup* LLGG_ptr,Earth* E_ptr=NULL);
   EarthRegionsGroup(
      Pass* PI_ptr,PointCloudsGroup* PCG_ptr,LatLongGridsGroup* LLGG_ptr,
      Earth* E_ptr=NULL);
   EarthRegionsGroup(
      Pass* PI_ptr,LatLongGridsGroup* LLGG_ptr,MoviesGroup* MG_ptr,
      Earth* E_ptr=NULL);
   EarthRegionsGroup(
      Pass* PI_ptr,PointCloudsGroup* PCG_ptr,LatLongGridsGroup* LLGG_ptr,
      MoviesGroup* MG_ptr,Earth* E_ptr=NULL);
   virtual ~EarthRegionsGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const EarthRegionsGroup& R);

// Set & get methods:

   void set_northern_hemisphere_flag(bool flag);
   const bool& get_northern_hemisphere_flag() const;
   void set_specified_UTM_zonenumber(int zonenumber);
   const int& get_specified_UTM_zonenumber() const;
   void set_propagate_all_tracks_flag(bool flag);
   void set_check_Cylinder_ROI_intersections_flag(bool flag);
   void set_t_start(double t);
   void set_t_stop(double t);
   void set_movie_alpha(double alpha);
   void set_ROI_color(colorfunc::Color roi_color);
   colorfunc::Color get_ROI_color() const;
   void set_KOZ_color(colorfunc::Color koz_color);
   colorfunc::Color get_KOZ_color() const;
   void set_SKS_DataServer_URL(std::string URL);
   EarthRegion* get_curr_region_ptr();

   EarthRegion* get_EarthRegion_ptr(int r);
   const EarthRegion* get_EarthRegion_ptr(int r) const;
   EarthRegion* get_ID_labeled_EarthRegion_ptr(int ID);
   const EarthRegion* get_ID_labeled_EarthRegion_ptr(int ID) const;
   PointCloudsGroup* get_PointCloudsGroup_ptr();
   const PointCloudsGroup* get_PointCloudsGroup_ptr() const;

   void set_CylindersGroup_ptr(CylindersGroup* CylindersGroup_ptr);
   void set_PolyhedraGroup_ptr(PolyhedraGroup* PolyhedraGroup_ptr);
   
   TextureSectorsGroup* get_TextureSectorsGroup_ptr();
   const TextureSectorsGroup* get_TextureSectorsGroup_ptr() const;
   threevector get_TextureSector_posn(int ID);

   Messenger* get_robots_Messenger_ptr();
   const Messenger* get_robots_Messenger_ptr() const;
   Messenger* get_GoogleEarth_Messenger_ptr();
   const Messenger* get_GoogleEarth_Messenger_ptr() const;
   Messenger* get_aircraft_Messenger_ptr();
   const Messenger* get_aircraft_Messenger_ptr() const;
   Messenger* get_blueforce_car_Messenger_ptr();
   const Messenger* get_blueforce_car_Messenger_ptr() const;
   void assign_EarthRegionsGroup_Messenger_ptrs();

// EarthRegion creation member functions:

   void generate_regions(PassesGroup& passes_group,
                         bool place_onto_bluemarble_flag=true,
                         bool generate_pointcloud_LatLongGrid_flag=true,
                         bool display_SurfaceTexture_LatLongGrid_flag=false);
   void generate_empty_EarthRegion(Pass* pass_ptr);
   void generate_PointCloud_EarthRegion(
      Pass* pass_ptr,bool place_onto_bluemarble_flag=true,
      bool generate_pointcloud_LatLongGrid_flag=true);
   void generate_SurfaceTexture_EarthRegion(
      Pass* pass_ptr,bool generate_LatLongGrid_flag);
   void generate_SurfaceTexture_EarthRegion(
      std::string Movie_filename,
      std::vector<threevector>& video_corner_vertices);

   EarthRegion* generate_new_EarthRegion(
      LatLongGrid* latlonggrid_ptr,Earth* Earth_ptr,
      DataGraph* DataGraph_ptr=NULL,int ID=-1,int OSGsubPAT_number=0);

//   void insert_into_cloud_transform(int r,osg::Node* node_ptr);
   osg::MatrixTransform* UTM_to_surface_transform(
      unsigned int r,const threevector& UTM_translation,
      double origin_longitude,double origin_latitude,double origin_altitude,
      osg::Node* node_ptr);

// Animation member functions:

   Movie* generate_EarthRegion_video_chip(int r,double alpha=0);
   Movie* get_EarthRegion_video_chip(int r);
   void destroy_EarthRegion_video_chip(int r,Movie* Movie_ptr);
   texture_rectangle* generate_EarthRegion_texture_rectangle(
      int r,std::string texture_filename);
   void update_display();

// Annotation member functions:
   
   void generate_annotation_bboxes(int r);
   PolyLinesGroup* generate_roadlines_group(int r);
   RegionPolyLinesGroup* generate_ROIlines_group(int r);
   RegionPolyLinesGroup* generate_KOZlines_group(int r);

// Dataserver track retrieval & display member functions:

   void initialize_track_mover_Cylinders(
      int r,double cylinder_radius,double cylinder_height,
      double cylinder_alpha,bool associate_tracks_with_movers_flag=false);
   std::string form_SKS_query_for_tracks_intersecting_polyline(
      PolyLinesGroup* PolyLinesGroup_ptr,polyline*& ROI_polyline_ptr);
   std::string form_SKS_query_for_speed_ROIs(
      double minSpeed,double maxSpeed,double minDuration,double radius,
      const std::vector<tracks_group*>& vehicle_tracks_group_ptrs);

// Real time persistent surveillance specific member functions

   void initialize_RTPS_mover_Cylinders(
      int n_cylinders,int starting_ID,bool associate_tracks_with_movers_flag);
   void update_blueforce_car_posn(
      int blueforce_track_ID,double elapsed_secs,
      double longitude,double latitude,double altitude);

  protected:

  private:

   bool northern_hemisphere_flag;
   bool propagate_all_tracks_flag,check_Cylinder_ROI_intersections_flag;
   int curr_region_ID,specified_UTM_zonenumber;
   double t_start,t_stop;
   double movie_alpha;
   colorfunc::Color ROI_color,KOZ_color;
   PointCloudsGroup* clouds_group_ptr;
   TextureSectorsGroup* TextureSectorsGroup_ptr;
   LatLongGridsGroup* latlonggrids_group_ptr;
   CylindersGroup* CylindersGroup_ptr;
   PolyhedraGroup* PolyhedraGroup_ptr;
   Earth* Earth_ptr;
   MoviesGroup* MoviesGroup_ptr;
   Messenger *urban_network_Messenger_ptr,*robots_Messenger_ptr,
      *GoogleEarth_Messenger_ptr,*aircraft_Messenger_ptr,
      *blueforce_car_Messenger_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const EarthRegionsGroup& R);

   void initialize_new_EarthRegion(
      EarthRegion* EarthRegion_ptr,int OSGsubPAT_number=0);

   virtual bool parse_next_message_in_queue(message& curr_message);
   bool generate_ROI(const geopoint& ROI_center);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void EarthRegionsGroup::set_northern_hemisphere_flag(bool flag)
{
   northern_hemisphere_flag=flag;
}

inline const bool& EarthRegionsGroup::get_northern_hemisphere_flag() const
{
   return northern_hemisphere_flag;
}

inline void EarthRegionsGroup::set_specified_UTM_zonenumber(int zonenumber)
{
   specified_UTM_zonenumber=zonenumber;
}

inline const int& EarthRegionsGroup::get_specified_UTM_zonenumber() const
{
   return specified_UTM_zonenumber;
}

inline void EarthRegionsGroup::set_propagate_all_tracks_flag(bool flag)
{
   propagate_all_tracks_flag=flag;
}

inline void EarthRegionsGroup::set_check_Cylinder_ROI_intersections_flag(
   bool flag)
{
   check_Cylinder_ROI_intersections_flag=flag;
}

inline void EarthRegionsGroup::set_t_start(double t)
{
   t_start=t;
}

inline void EarthRegionsGroup::set_t_stop(double t)
{
   t_stop=t;
}

inline void EarthRegionsGroup::set_movie_alpha(double alpha)
{
   movie_alpha=alpha;
}

inline void EarthRegionsGroup::set_ROI_color(colorfunc::Color roi_color)
{
   ROI_color=roi_color;
}

inline colorfunc::Color EarthRegionsGroup::get_ROI_color() const
{
   return ROI_color;
}

inline void EarthRegionsGroup::set_KOZ_color(colorfunc::Color koz_color)
{
   KOZ_color=koz_color;
}

inline colorfunc::Color EarthRegionsGroup::get_KOZ_color() const
{
   return KOZ_color;
}

inline EarthRegion* EarthRegionsGroup::get_curr_region_ptr() 
{
   return get_EarthRegion_ptr(curr_region_ID);
}

inline EarthRegion* EarthRegionsGroup::get_EarthRegion_ptr(int r) 
{
   return dynamic_cast<EarthRegion*>(get_Graphical_ptr(r));
}

inline const EarthRegion* EarthRegionsGroup::get_EarthRegion_ptr(int r) const
{
   return dynamic_cast<EarthRegion*>(get_Graphical_ptr(r));
}

inline EarthRegion* EarthRegionsGroup::get_ID_labeled_EarthRegion_ptr(int ID) 
{
   return dynamic_cast<EarthRegion*>(get_ID_labeled_Graphical_ptr(ID));
}

inline const EarthRegion* EarthRegionsGroup::get_ID_labeled_EarthRegion_ptr(
   int ID) const
{
   return dynamic_cast<EarthRegion*>(get_ID_labeled_Graphical_ptr(ID));
}

inline PointCloudsGroup* EarthRegionsGroup::get_PointCloudsGroup_ptr() 
{
   return clouds_group_ptr;
}

inline const PointCloudsGroup* EarthRegionsGroup::get_PointCloudsGroup_ptr() 
   const
{
   return clouds_group_ptr;
}

inline void EarthRegionsGroup::set_CylindersGroup_ptr(
   CylindersGroup* CylindersGroup_ptr)
{
   this->CylindersGroup_ptr=CylindersGroup_ptr;
}

inline void EarthRegionsGroup::set_PolyhedraGroup_ptr(
   PolyhedraGroup* PolyhedraGroup_ptr)
{
   this->PolyhedraGroup_ptr=PolyhedraGroup_ptr;
}

inline TextureSectorsGroup* EarthRegionsGroup::get_TextureSectorsGroup_ptr()
{
   return TextureSectorsGroup_ptr;
}

inline const TextureSectorsGroup* EarthRegionsGroup::get_TextureSectorsGroup_ptr() const
{
   return TextureSectorsGroup_ptr;
}

inline Messenger* EarthRegionsGroup::get_robots_Messenger_ptr()
{
   return robots_Messenger_ptr;
}

inline const Messenger* EarthRegionsGroup::get_robots_Messenger_ptr() const
{
   return robots_Messenger_ptr;
}

inline Messenger* EarthRegionsGroup::get_GoogleEarth_Messenger_ptr()
{
   return GoogleEarth_Messenger_ptr;
}

inline const Messenger* EarthRegionsGroup::get_GoogleEarth_Messenger_ptr() const
{
   return GoogleEarth_Messenger_ptr;
}

inline Messenger* EarthRegionsGroup::get_aircraft_Messenger_ptr()
{
   return aircraft_Messenger_ptr;
}

inline const Messenger* EarthRegionsGroup::get_aircraft_Messenger_ptr() const
{
   return aircraft_Messenger_ptr;
}

inline Messenger* EarthRegionsGroup::get_blueforce_car_Messenger_ptr()
{
   return blueforce_car_Messenger_ptr;
}

#endif // EarthRegionsGroup.h





