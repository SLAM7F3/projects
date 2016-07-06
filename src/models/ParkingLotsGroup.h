// ==========================================================================
// Header file for ParkingLotsGroup class
// ==========================================================================
// Last modified on 4/21/12; 6/12/12; 4/5/14
// ==========================================================================

#ifndef PARKINGLOTSGROUP_H
#define PARKINGLOTSGROUP_H

#include <map>
#include <string>
#include <vector>
#include "math/lttwovector.h"
#include "models/ParkingLot.h"
#include "math/twovector.h"


class polyhedron;

class ParkingLotsGroup 
{

  public:

   typedef std::map<twovector,polyhedron*,lttwovector> PARKINGLOT_POLYHEDRON_MAP;

// Indep var: twovector holding parking_lot,polyhedron IDs
// Depend var: pointer to specified ParkingLot polyhedron

// Initialization, constructor and destructor functions:

   ParkingLotsGroup();

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~ParkingLotsGroup();

   int get_ID() const;
   int get_ParkingLot_ID(int polyhedron_ID) const;
   unsigned int get_n_ParkingLots() const;
   ParkingLot* get_ParkingLot_ptr(int ID);
   const ParkingLot* get_ParkingLot_ptr(int ID) const;

   PARKINGLOT_POLYHEDRON_MAP* get_ParkingLot_polyhedron_map_ptr();
   const PARKINGLOT_POLYHEDRON_MAP* get_ParkingLot_polyhedron_map_ptr() const;

// Manipulation member functions:

   void import_from_OFF_files(std::string OFF_subdir);

  private:
   
//    unsigned int n_ParkingLots;
   int ID;
   std::vector<ParkingLot*> ParkingLot_ptrs;
   PARKINGLOT_POLYHEDRON_MAP* ParkingLot_polyhedron_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ParkingLotsGroup& BG);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int ParkingLotsGroup::get_ID() const
{
   return ID;
}

inline unsigned int ParkingLotsGroup::get_n_ParkingLots() const
{
   return ParkingLot_ptrs.size();
}

inline ParkingLot* ParkingLotsGroup::get_ParkingLot_ptr(int ID)
{
   if (ID <0 || ID >= int(get_n_ParkingLots())) return NULL;
   return ParkingLot_ptrs[ID];
}

inline const ParkingLot* ParkingLotsGroup::get_ParkingLot_ptr(int ID) const
{
   return ParkingLot_ptrs[ID];
}

inline ParkingLotsGroup::PARKINGLOT_POLYHEDRON_MAP* 
ParkingLotsGroup::get_ParkingLot_polyhedron_map_ptr()
{
   return ParkingLot_polyhedron_map_ptr;
}

inline const ParkingLotsGroup:: PARKINGLOT_POLYHEDRON_MAP* 
ParkingLotsGroup::get_ParkingLot_polyhedron_map_ptr() const
{
   return ParkingLot_polyhedron_map_ptr;
}

#endif  // ParkingLotsGroup.h



