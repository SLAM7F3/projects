// ==========================================================================
// Header file for groundspace class
// ==========================================================================
// Last modified on 2/14/08; 2/21/08; 2/27/08
// ==========================================================================

#ifndef GROUNDSPACE_H
#define GROUNDSPACE_H

#include "math/threevector.h"
#include "osg/osgGeometry/PolygonsGroup.h"

class groundspace
{

  public:

// Initialization, constructor and destructor functions:

   groundspace();
   groundspace(const groundspace& g);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~groundspace();
   groundspace& operator= (const groundspace& g);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const groundspace& g);

// Set and get member functions:

   void set_origin(const threevector& origin);
   threevector& get_origin();
   const threevector& get_origin() const;
   
   void set_UTM_zonenumber(int UTM_zonenumber);
   int get_UTM_zonenumber() const;
   void set_northern_hemisphere_flag(bool flag);
   bool get_northern_hemisphere_flag() const;
   void set_max_robot_dist_from_origin(double d);
   double get_max_robot_dist_from_origin() const;

   void set_KOZ_PolygonsGroup_ptr(osgGeometry::PolygonsGroup* PG_ptr);

// Cost function member functions:

   double potential_energy(double r,const threevector& x);

  private: 

   bool northern_hemisphere_flag;
   int UTM_zonenumber;
   threevector origin;
   double max_robot_dist_from_origin;
   osgGeometry::PolygonsGroup* KOZ_PolygonsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const groundspace& g);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void groundspace::set_origin(const threevector& origin)
{
   this->origin=origin;
}

inline threevector& groundspace::get_origin()
{
   return origin;
}

inline const threevector& groundspace::get_origin() const
{
   return origin;
}

inline void groundspace::set_UTM_zonenumber(int UTM_zonenumber)
{
   this->UTM_zonenumber=UTM_zonenumber;
}

inline int groundspace::get_UTM_zonenumber() const
{
   return UTM_zonenumber;
}

inline void groundspace::set_northern_hemisphere_flag(bool flag)
{
   northern_hemisphere_flag=flag;
}

inline bool groundspace::get_northern_hemisphere_flag() const
{
   return northern_hemisphere_flag;
}

inline void groundspace::set_max_robot_dist_from_origin(double d)
{
   max_robot_dist_from_origin=d;
}

inline double groundspace::get_max_robot_dist_from_origin() const
{
   return max_robot_dist_from_origin;
}

inline void groundspace::set_KOZ_PolygonsGroup_ptr(
   osgGeometry::PolygonsGroup* PG_ptr)
{
   KOZ_PolygonsGroup_ptr=PG_ptr;
}


#endif  // groundspace.h



