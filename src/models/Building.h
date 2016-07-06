// ==========================================================================
// Header file for Building class
// ==========================================================================
// Last modified on 4/15/12; 4/16/12; 6/28/12
// ==========================================================================

#ifndef BUILDING_H
#define BUILDING_H

#include <iostream>
#include <map>
#include <vector>
#include "color/colorfuncs.h"
#include "math/lttwovector.h"
#include "datastructures/Quadruple.h"
#include "math/threevector.h"
#include "math/twovector.h"

class polygon;
class polyhedron;

class Building 
{

  public:

   typedef Quadruple<double,double,double,int> quadruple;

   typedef std::map<twovector,polygon*,lttwovector> RECTANGLE_SIDEFACES_MAP;

// Independent var for RECTANGLE_SIDEFACES_MAP holds rectangle and
// polyhedron IDs
// Dependent var holds pointer to rectangle side-face polygon

// Initialization, constructor and destructor functions:

   Building();

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~Building();

   friend std::ostream& operator<< 
      (std::ostream& outstream,Building& B);

// Set & get member functions:

   int get_ID() const;
   double get_ground_z() const;
   double get_roof_z() const;
   threevector& get_roof_COM();
   const threevector& get_roof_COM() const;
   
   std::vector<polyhedron*>& get_polyhedra_ptrs();
   const std::vector<polyhedron*>& get_polyhedra_ptrs() const;
   colorfunc::RGB get_primary_color() const;
   void set_primary_color(double r,double g,double b);
   RECTANGLE_SIDEFACES_MAP* get_rectangle_sidefaces_map_ptr();
   const RECTANGLE_SIDEFACES_MAP* get_rectangle_sidefaces_map_ptr() const;

// Manipulation member functions:

   void import_from_OFF_files(int ID,std::string OFF_subdir);
   void identify_occluded_rectangle_sidefaces();
//   bool point_inside(const threevector& V);

// Building coloring member functions

   void push_back_hsv(double h,double s,double v);
   bool compute_primary_color();

  private:
   
   int ID;
   double ground_z,roof_z;
   colorfunc::RGB primary_color;
   threevector roof_COM;
   std::vector<polyhedron*> polyhedra_ptrs;
   std::vector<quadruple*>* hsv_bin_ptr;
   std::vector<int> color_bin_frequency;
   RECTANGLE_SIDEFACES_MAP* rectangle_sidefaces_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Building& b);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int Building::get_ID() const
{
   return ID;
}

inline double Building::get_ground_z() const
{
   return ground_z;
}

inline double Building::get_roof_z() const
{
   return roof_z;
}

inline threevector& Building::get_roof_COM()
{
   return roof_COM;
}

inline const threevector& Building::get_roof_COM() const
{
   return roof_COM;
}

inline std::vector<polyhedron*>& Building::get_polyhedra_ptrs()
{
   return polyhedra_ptrs;
}

inline const std::vector<polyhedron*>& Building::get_polyhedra_ptrs() const
{
   return polyhedra_ptrs;
}

inline colorfunc::RGB Building::get_primary_color() const
{
   return primary_color;
}

inline void Building::set_primary_color(double r,double g,double b)
{
   primary_color.first=r;
   primary_color.second=g;
   primary_color.third=b;
}

inline Building::RECTANGLE_SIDEFACES_MAP* 
Building::get_rectangle_sidefaces_map_ptr()
{
   return rectangle_sidefaces_map_ptr;
}

inline const Building::RECTANGLE_SIDEFACES_MAP* 
Building::get_rectangle_sidefaces_map_ptr() const
{
   return rectangle_sidefaces_map_ptr;
}

#endif  // Building.h



