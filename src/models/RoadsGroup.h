// ==========================================================================
// Header file for RoadsGroup class
// ==========================================================================
// Last modified on 4/21/12; 4/5/14
// ==========================================================================

#ifndef ROADSGROUP_H
#define ROADSGROUP_H

#include <map>
#include <string>
#include <vector>
#include "math/lttwovector.h"
#include "math/twovector.h"

class polyhedron;
class Road;

class RoadsGroup 
{

  public:

   typedef std::map<twovector,polyhedron*,lttwovector> ROAD_POLYHEDRON_MAP;

// Indep var: twovector holding parking_lot,polyhedron IDs
// Depend var: pointer to specified Road polyhedron

// Initialization, constructor and destructor functions:

   RoadsGroup();

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~RoadsGroup();

   int get_ID() const;
   int get_Road_ID(int polyhedron_ID) const;
   unsigned int get_n_Roads() const;
   Road* get_Road_ptr(int ID);
   const Road* get_Road_ptr(int ID) const;

   ROAD_POLYHEDRON_MAP* get_Road_polyhedron_map_ptr();
   const ROAD_POLYHEDRON_MAP* get_Road_polyhedron_map_ptr() const;

// Manipulation member functions:

   void import_from_OFF_files(std::string OFF_subdir);

  private:
   
   int ID;
   std::vector<Road*> Road_ptrs;
   ROAD_POLYHEDRON_MAP* Road_polyhedron_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const RoadsGroup& BG);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int RoadsGroup::get_ID() const
{
   return ID;
}

inline unsigned int RoadsGroup::get_n_Roads() const
{
   return Road_ptrs.size();
}

inline Road* RoadsGroup::get_Road_ptr(int ID)
{
   if (ID <0 || ID >= int(get_n_Roads())) return NULL;
   return Road_ptrs[ID];
}

inline const Road* RoadsGroup::get_Road_ptr(int ID) const
{
   return Road_ptrs[ID];
}

inline RoadsGroup::ROAD_POLYHEDRON_MAP* 
RoadsGroup::get_Road_polyhedron_map_ptr()
{
   return Road_polyhedron_map_ptr;
}

inline const RoadsGroup:: ROAD_POLYHEDRON_MAP* 
RoadsGroup::get_Road_polyhedron_map_ptr() const
{
   return Road_polyhedron_map_ptr;
}

#endif  // RoadsGroup.h



