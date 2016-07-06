// ==========================================================================
// Header file for EarthRegion class
// ==========================================================================
// Last updated on 3/10/09; 4/26/09; 5/4/09; 6/5/09; 1/22/16
// ==========================================================================

#ifndef EARTHREGION_H
#define EARTHREGION_H

#include <string>
#include <vector>
#include <osg/MatrixTransform>
#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "astro_geo/geopoint.h"
#include "osg/osgSceneGraph/DataGraph.h"
#include "osg/osgGeometry/Geometrical.h"
#include "osg/osgGrid/LatLongGrid.h"
#include "messenger/Messenger.h"
#include "track/movers_group.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgRegions/RegionPolyLinesGroup.h"
#include "osg/osgEarth/TextureSector.h"
#include "math/threevector.h"
#include "track/tracks_group.h"

class Cylinder;
class CylindersGroup;
class Earth;
class PointFinder;
class statevector;
class track;

class EarthRegion : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   EarthRegion(int id);
   EarthRegion(LatLongGrid* LLG_ptr,Earth* E_ptr=NULL,DataGraph* DG_ptr=NULL,
               int id=-1);
   virtual ~EarthRegion();
   friend std::ostream& operator<< (
      std::ostream& outstream,const EarthRegion& r);

// Set & get member functions:

   bounding_box* get_bbox_ptr(int n);
   const bounding_box* get_bbox_ptr(int n) const;
   tracks_group* get_dynamic_tracks_group_ptr();
   tracks_group* get_spatially_fixed_tracks_group_ptr();
   tracks_group* get_KOZ_tracks_group_ptr();

   movers_group* get_movers_group_ptr();
   PolyLinesGroup* get_roadlines_group_ptr();
   const PolyLinesGroup* get_roadlines_group_ptr() const;

   RegionPolyLinesGroup* get_ROILinesGroup_ptr();
   const RegionPolyLinesGroup* get_ROILinesGroup_ptr() const;
   RegionPolyLinesGroup* get_KOZLinesGroup_ptr();
   const RegionPolyLinesGroup* get_KOZLinesGroup_ptr() const;

   void set_robots_Messenger_ptr(Messenger* m_ptr);
   Messenger* get_robots_Messenger_ptr();
   const Messenger* get_robots_Messenger_ptr() const;

   void set_TextureSector_ptr(TextureSector* TS_ptr);
   TextureSector* get_TextureSector_ptr();
   const TextureSector* get_TextureSector_ptr() const;

   void set_northern_hemisphere_flag(bool flag);
   bool get_northern_hemisphere_flag() const;
   void set_UTM_zonenumber(int zone_number);
   int get_UTM_zonenumber() const;
   
   DataGraph* get_DataGraph_ptr();
   const DataGraph* get_DataGraph_ptr() const;
   LatLongGrid* get_LatLongGrid_ptr();
   const LatLongGrid* get_LatLongGrid_ptr() const;

   osg::MatrixTransform* get_SurfaceTransform_ptr();
   const osg::MatrixTransform* get_SurfaceTransform_ptr() const;
   osg::MatrixTransform* get_GridSurfaceTransform_ptr();
   const osg::MatrixTransform* get_GridSurfaceTransform_ptr() const;

   std::vector<geopoint>& get_GMTI_targets();
   std::vector<geopoint>& get_region_corners();
   std::vector<tracks_group*>& get_ROI_tracks_group_ptrs();
   const std::vector<tracks_group*>& get_ROI_tracks_group_ptrs() const;

// Matrix transform member functions:

   void place_cloud_onto_ellipsoid_surface(double altitude_offset=0);
   void place_latlonggrid_onto_ellipsoid_surface(double altitude_offset=0);
   osg::MatrixTransform* UTM_to_surface_transform(
      const threevector& UTM_translation,
      const threevector& origin_long_lat_alt,osg::Node* node_ptr=NULL);

// Coordinate transformation member functions:

   bool retrieve_lat_long_alt(
      const threevector& UTM_coords,geopoint* geopoint_ptr);

// GMTI member functions:

   void add_GMTI_target(const geopoint& curr_GMTI_target);

// Annotation member functions:
   
   Polyhedron* generate_bbox(
      PolyhedraGroup* PHG_ptr,int Polyhedra_subgroup,
      const geopoint& top_left_corner,const geopoint& bottom_right_corner,
      std::string bbox_color_str,std::string bbox_label,
      std::string bbox_label_color_str);
   PolyLinesGroup* generate_roadlines_group(Pass* pass_ptr);
   RegionPolyLinesGroup* generate_ROILinesGroup(
      Pass* pass_ptr, AnimationController* AC_ptr);
   RegionPolyLinesGroup* generate_KOZLinesGroup(
      Pass* pass_ptr, AnimationController* AC_ptr);

// Track member functions:

   void generate_track_colors(
      int n_cumulative_tracks,tracks_group* curr_tracks_group_ptr);
   void display_tracks_as_PolyLines(
      int n_cumulative_tracks,tracks_group* curr_tracks_group_ptr,
      PointFinder* pointfinder_ptr,PolyLinesGroup* PolyLinesGroup_ptr);
   void initialize_track_mover_Cylinder(
      double alpha,Cylinder* curr_Cylinder_ptr,track* curr_track_ptr);
   void propagate_all_tracks(CylindersGroup* CylindersGroup_ptr);


// Messenger member functions:

   void broadcast_dynamic_tracks();
   void broadcast_spatially_fixed_tracks();
   void broadcast_KOZ_tracks();

// Region of Interest (ROI) member functions:

   mover* generate_nominated_ROI(
      const std::vector<threevector>& vertices,
      colorfunc::Color ROI_PolyLine_color,std::string annotation_label="");
   std::vector<threevector> generate_ROI_circle_vertices(
      double center_longitude,double center_latitude,
      double center_altitude,double ROI_radius,int specified_UTM_zonenumber);
   tracks_group* generate_ROI_tracks_group();
   void purge_ROI_tracks_group_ptrs();
   void clear_all_ROIs();
   void check_Cylinder_ROI_intersections(CylindersGroup* CylindersGroup_ptr);
   void track_Cylinder_with_ROI_Polyhedron(
      CylindersGroup* CylindersGroup_ptr,PolyhedraGroup* PolyhedraGroup_ptr,
      bool bind_ROI_to_selected_Cylinder_flag);
   
  protected:

  private:

   bool northern_hemisphere_flag;
   int UTM_zonenumber;
   int prev_framenumber,prev_framenumber2;
   std::vector<bounding_box*> bbox_ptrs;
   Earth* Earth_ptr;
   DataGraph* DataGraph_ptr;
   LatLongGrid* LatLongGrid_ptr;
   TextureSector* TextureSector_ptr;
   tracks_group *dynamic_tracks_group_ptr,*spatially_fixed_tracks_group_ptr;
   tracks_group* KOZ_tracks_group_ptr;
   std::vector<tracks_group*> ROI_tracks_group_ptrs;
   movers_group* movers_group_ptr;
   PolyLinesGroup* roadlines_group_ptr;
   RegionPolyLinesGroup *ROILinesGroup_ptr, *KOZLinesGroup_ptr;
   Messenger* robots_Messenger_ptr;
   osg::MatrixTransform *SurfaceTransform_ptr,*GridSurfaceTransform_ptr;
   std::vector<geopoint> GMTI_targets;
   std::vector<geopoint> region_corners;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const EarthRegion& r);

   void display_track_as_PolyLine(
      PointFinder* pointfinder_ptr,PolyLinesGroup* PolyLinesGroup_ptr,
      track* curr_track_ptr,double distance_between_arrows);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline bounding_box* EarthRegion::get_bbox_ptr(int n)
{
   if (n >= 0 && n < int(bbox_ptrs.size()))
   {
      return bbox_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline const bounding_box* EarthRegion::get_bbox_ptr(int n) const
{
   if (n >= 0 && n < int(bbox_ptrs.size()))
   {
      return bbox_ptrs[n];
   }
   else
   {
      return NULL;
   }
}

inline tracks_group* EarthRegion::get_dynamic_tracks_group_ptr()
{
   return dynamic_tracks_group_ptr;
}

inline tracks_group* EarthRegion::get_spatially_fixed_tracks_group_ptr()
{
   return spatially_fixed_tracks_group_ptr;
}

inline tracks_group* EarthRegion::get_KOZ_tracks_group_ptr()
{
   return KOZ_tracks_group_ptr;
}

inline movers_group* EarthRegion::get_movers_group_ptr()
{
   return movers_group_ptr;
}

inline PolyLinesGroup* EarthRegion::get_roadlines_group_ptr()
{
   return roadlines_group_ptr;
}

inline const PolyLinesGroup* EarthRegion::get_roadlines_group_ptr() const
{
   return roadlines_group_ptr;
}

inline RegionPolyLinesGroup* EarthRegion::get_ROILinesGroup_ptr()
{
   return ROILinesGroup_ptr;
}

inline const RegionPolyLinesGroup* EarthRegion::get_ROILinesGroup_ptr() const
{
   return ROILinesGroup_ptr;
}

inline RegionPolyLinesGroup* EarthRegion::get_KOZLinesGroup_ptr()
{
   return KOZLinesGroup_ptr;
}

inline const RegionPolyLinesGroup* EarthRegion::get_KOZLinesGroup_ptr() const
{
   return KOZLinesGroup_ptr;
}

inline void EarthRegion::set_robots_Messenger_ptr(Messenger* m_ptr)
{
   robots_Messenger_ptr=m_ptr;
}

inline Messenger* EarthRegion::get_robots_Messenger_ptr()
{
   return robots_Messenger_ptr;
}

inline const Messenger* EarthRegion::get_robots_Messenger_ptr() const
{
   return robots_Messenger_ptr;
}

inline void EarthRegion::set_TextureSector_ptr(TextureSector* TS_ptr)
{
   TextureSector_ptr=TS_ptr;
}

inline TextureSector* EarthRegion::get_TextureSector_ptr()
{
   return TextureSector_ptr;
}

inline const TextureSector* EarthRegion::get_TextureSector_ptr() const
{
   return TextureSector_ptr;
}

inline void EarthRegion::set_northern_hemisphere_flag(bool flag)
{
   northern_hemisphere_flag=flag;
}

inline bool EarthRegion::get_northern_hemisphere_flag() const
{
   return northern_hemisphere_flag;
}

inline void EarthRegion::set_UTM_zonenumber(int zone_number)
{
   UTM_zonenumber=zone_number;
}

inline int EarthRegion::get_UTM_zonenumber() const
{
   return UTM_zonenumber;
}

inline DataGraph* EarthRegion::get_DataGraph_ptr() 
{
   return DataGraph_ptr;
}

inline const DataGraph* EarthRegion::get_DataGraph_ptr() const
{
   return DataGraph_ptr;
}

inline LatLongGrid* EarthRegion::get_LatLongGrid_ptr()
{
   return LatLongGrid_ptr;
}

inline const LatLongGrid* EarthRegion::get_LatLongGrid_ptr() const
{
   return LatLongGrid_ptr;
}

inline osg::MatrixTransform* EarthRegion::get_SurfaceTransform_ptr()
{
   return SurfaceTransform_ptr;
}

inline const osg::MatrixTransform* EarthRegion::get_SurfaceTransform_ptr() 
   const
{
   return SurfaceTransform_ptr;
}

inline osg::MatrixTransform* EarthRegion::get_GridSurfaceTransform_ptr()
{
   return GridSurfaceTransform_ptr;
}

inline const osg::MatrixTransform* 
EarthRegion::get_GridSurfaceTransform_ptr() const
{
   return GridSurfaceTransform_ptr;
}

inline std::vector<geopoint>& EarthRegion::get_GMTI_targets()
{
   return GMTI_targets;
}

inline std::vector<geopoint>& EarthRegion::get_region_corners()
{
   return region_corners;
}

inline std::vector<tracks_group*>& EarthRegion::get_ROI_tracks_group_ptrs()
{
   return ROI_tracks_group_ptrs;
}

inline const std::vector<tracks_group*>& EarthRegion::get_ROI_tracks_group_ptrs() const
{
   return ROI_tracks_group_ptrs;
}


#endif // EarthRegion.h



