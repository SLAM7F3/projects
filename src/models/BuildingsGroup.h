// ==========================================================================
// Header file for BuildingsGroup class
// ==========================================================================
// Last modified on 3/13/12; 4/24/12; 4/5/14
// ==========================================================================

#ifndef BUILDINGSGROUP_H
#define BUILDINGSGROUP_H

#include <map>
#include <string>
#include <vector>
#include "math/lttwovector.h"
#include "math/twovector.h"

class polyhedron;
class Building;

class BuildingsGroup 
{

  public:

   typedef std::map<twovector,polyhedron*,lttwovector> BUILDING_POLYHEDRON_MAP;

// Indep var: twovector holding building,polyhedron IDs
// Depend var: pointer to specified building polyhedron

   typedef std::map<int,fourvector> 
      CLIPPEDPOLYGON_BUILDING_POLYHEDRON_RECTANGLE_MAP;

// Indep var: clipped Polygon ID
// Depend var: fourvector holding building, polyhedron and rectangle IDs plus
// dotproduct weight

// Initialization, constructor and destructor functions:

   BuildingsGroup();

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~BuildingsGroup();

   int get_ID() const;
   int get_Building_ID(int polyhedron_ID) const;
   unsigned int get_n_Buildings() const;
   Building* get_Building_ptr(int ID);
   const Building* get_Building_ptr(int ID) const;

   BUILDING_POLYHEDRON_MAP* get_Building_polyhedron_map_ptr();
   const BUILDING_POLYHEDRON_MAP* get_Building_polyhedron_map_ptr() const;

   CLIPPEDPOLYGON_BUILDING_POLYHEDRON_RECTANGLE_MAP* 
      get_ClippedPolygon_building_polyhedron_rectangle_map_ptr();
   const CLIPPEDPOLYGON_BUILDING_POLYHEDRON_RECTANGLE_MAP* 
      get_ClippedPolygon_building_polyhedron_rectangle_map_ptr() const;

// Manipulation member functions:

   void import_from_OFF_files(std::string OFF_subdir);
   void compute_primary_Building_colors();

  private:
   
   int ID;
   std::vector<Building*> Building_ptrs;
   BUILDING_POLYHEDRON_MAP* Building_polyhedron_map_ptr;

   CLIPPEDPOLYGON_BUILDING_POLYHEDRON_RECTANGLE_MAP* 
      ClippedPolygon_building_polyhedron_rectangle_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const BuildingsGroup& BG);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int BuildingsGroup::get_ID() const
{
   return ID;
}

inline unsigned int BuildingsGroup::get_n_Buildings() const
{
   return Building_ptrs.size();
}

inline Building* BuildingsGroup::get_Building_ptr(int ID)
{
   if (ID <0 || ID >= int(get_n_Buildings())) return NULL;
   return Building_ptrs[ID];
}

inline const Building* BuildingsGroup::get_Building_ptr(int ID) const
{
   return Building_ptrs[ID];
}

inline BuildingsGroup::BUILDING_POLYHEDRON_MAP* 
BuildingsGroup::get_Building_polyhedron_map_ptr()
{
   return Building_polyhedron_map_ptr;
}

inline const BuildingsGroup:: BUILDING_POLYHEDRON_MAP* 
BuildingsGroup::get_Building_polyhedron_map_ptr() const
{
   return Building_polyhedron_map_ptr;
}

inline BuildingsGroup::CLIPPEDPOLYGON_BUILDING_POLYHEDRON_RECTANGLE_MAP* 
BuildingsGroup::get_ClippedPolygon_building_polyhedron_rectangle_map_ptr()
{
   return ClippedPolygon_building_polyhedron_rectangle_map_ptr;
}

inline const BuildingsGroup::CLIPPEDPOLYGON_BUILDING_POLYHEDRON_RECTANGLE_MAP* 
BuildingsGroup::get_ClippedPolygon_building_polyhedron_rectangle_map_ptr() const
{
   return ClippedPolygon_building_polyhedron_rectangle_map_ptr;
}



#endif  // BuildingsGroup.h



