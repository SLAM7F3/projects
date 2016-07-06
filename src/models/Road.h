// ==========================================================================
// Header file for Road class
// ==========================================================================
// Last modified on 4/21/12; 6/12/12
// ==========================================================================

#ifndef RAOD_H
#define ROAD_H

#include <iostream>
#include <vector>

class polyhedron;

class Road 
{

  public:

// Initialization, constructor and destructor functions:

   Road();

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~Road();

   friend std::ostream& operator<< 
      (std::ostream& outstream,Road& PL);

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
   void docopy(const Road& PL);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int Road::get_ID() const
{
   return ID;
}

inline double Road::get_ground_z() const
{
   return ground_z;
}

inline std::vector<polyhedron*>& Road::get_polyhedra_ptrs()
{
   return polyhedra_ptrs;
}

inline const std::vector<polyhedron*>& Road::get_polyhedra_ptrs() const
{
   return polyhedra_ptrs;
}

#endif  // Road.h



