// ==========================================================================
// Header file for REGIONPOLYLINESGROUP class
// ==========================================================================
// Last modified on 12/15/08; 5/3/09; 5/4/09; 1/22/16
// ==========================================================================

#ifndef REGIONPOLYLINESGROUP_H
#define REGIONPOLYLINESGROUP_H

#include <iostream>
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgRegions/RegionPolyLine.h"

class RegionPolyLinesGroup : public PolyLinesGroup
{

  public:

// Initialization, constructor and destructor functions:

// Note added at noontime, Mon Feb 18, 2008: Should eventually
// eliminate first constructor in favor of more general 2nd
// constructor...

   RegionPolyLinesGroup(
      const int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr,
      threevector* GO_ptr=NULL);
   RegionPolyLinesGroup(
      const int p_ndims,Pass* PI_ptr,osgGeometry::PolygonsGroup* PG_ptr,
      AnimationController* AC_ptr, threevector* GO_ptr=NULL);
   RegionPolyLinesGroup(
      const int p_ndims,Pass* PI_ptr,osgGeometry::PolygonsGroup* PG_ptr,
      PolyhedraGroup* PHG_ptr, AnimationController* AC_ptr, 
      threevector* GO_ptr=NULL);
   RegionPolyLinesGroup(
      const int p_ndims,Pass* PI_ptr,postgis_database* bgdb_ptr,
      threevector* GO_ptr=NULL);

   virtual ~RegionPolyLinesGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const RegionPolyLinesGroup& P);

// Set & get member functions:

//   RegionPolyLine* get_RegionPolyLine_ptr(int n) const;
//   RegionPolyLine* get_ID_labeled_RegionPolyLine_ptr(int ID) const;
   void set_ROI_PolyLinesGroup_flag(bool flag);
   bool get_ROI_PolyLinesGroup_flag() const;
   void set_KOZ_PolyLinesGroup_flag(bool flag);
   bool get_KOZ_PolyLinesGroup_flag() const;

// Update member functions:

   void recolor_encountered_ROI_PolyLines();
   void update_display();

  protected:

  private:

   bool ROI_PolyLinesGroup_flag,KOZ_PolyLinesGroup_flag;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const RegionPolyLinesGroup& f);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

/*
inline RegionPolyLine* RegionPolyLinesGroup::get_RegionPolyLine_ptr(int n) 
   const
{
   std::cout << "inside RPLG::get_RP_ptr()" << std::endl;
   std::cout << "n = " << n 
             << " get_Graphical_ptr(n) = " << get_Graphical_ptr(n)
             << std::endl;
   std::cout << "dynamic_cast<> = "
             << dynamic_cast<RegionPolyLine*>(get_Graphical_ptr(n))
             << std:: endl;
                                              
   return dynamic_cast<RegionPolyLine*>(get_Graphical_ptr(n));
}

inline RegionPolyLine* 
RegionPolyLinesGroup::get_ID_labeled_RegionPolyLine_ptr(int ID) const
{
   return dynamic_cast<RegionPolyLine*>(get_ID_labeled_Graphical_ptr(ID));
}
*/

inline void RegionPolyLinesGroup::set_ROI_PolyLinesGroup_flag(bool flag)
{
   ROI_PolyLinesGroup_flag=flag;
}

inline bool RegionPolyLinesGroup::get_ROI_PolyLinesGroup_flag() const
{
   return ROI_PolyLinesGroup_flag;
}

inline void RegionPolyLinesGroup::set_KOZ_PolyLinesGroup_flag(bool flag)
{
   KOZ_PolyLinesGroup_flag=flag;
}

inline bool RegionPolyLinesGroup::get_KOZ_PolyLinesGroup_flag() const
{
   return KOZ_PolyLinesGroup_flag;
}

#endif // RegionPolyLinesGroup.h



