// ==========================================================================
// Header file for ParkingLot class
// ==========================================================================
// Last modified on 4/21/12; 6/12/12
// ==========================================================================

#ifndef PARKINGLOT_H
#define PARKINGLOT_H

#include <iostream>
#include <vector>

class polyhedron;

class ParkingLot 
{

  public:

// Initialization, constructor and destructor functions:

   ParkingLot();

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~ParkingLot();

   friend std::ostream& operator<< 
      (std::ostream& outstream,ParkingLot& PL);

// Set & get member functions:

   int get_ID() const;
   double get_ground_z() const;
   std::vector<polyhedron*>& get_polyhedra_ptrs();
   const std::vector<polyhedron*>& get_polyhedra_ptrs() const;

// Manipulation member functions:

   void import_from_OFF_files(int ID,std::string OFF_subdir);

  private:
   
   int ID;
   double ground_z;
   std::vector<polyhedron*> polyhedra_ptrs;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ParkingLot& PL);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int ParkingLot::get_ID() const
{
   return ID;
}

inline double ParkingLot::get_ground_z() const
{
   return ground_z;
}

inline std::vector<polyhedron*>& ParkingLot::get_polyhedra_ptrs()
{
   return polyhedra_ptrs;
}

inline const std::vector<polyhedron*>& ParkingLot::get_polyhedra_ptrs() const
{
   return polyhedra_ptrs;
}

#endif  // ParkingLot.h



