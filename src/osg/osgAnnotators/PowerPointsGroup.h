// ==========================================================================
// Header file for POWERPOINTSGROUP class
// ==========================================================================
// Last modified on 8/24/07
// ==========================================================================

#ifndef POWERPOINTSGROUP_H
#define POWERPOINTSGROUP_H

#include <iostream>
#include "osg/osgAnnotators/AnnotatorsGroup.h"
#include "osg/osgAnnotators/PowerPoint.h"
#include "osg/osgGeometry/BoxesGroup.h"

class Clock;
class Ellipsoid_model;

class PowerPointsGroup : public BoxesGroup, public AnnotatorsGroup
{

  public:

// Initialization, constructor and destructor functions:

   PowerPointsGroup(Pass* PI_ptr,threevector* GO_ptr);
   PowerPointsGroup(Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
                    threevector* GO_ptr);
   virtual ~PowerPointsGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const PowerPointsGroup& A);

// Set & get member functions:

   PowerPoint* get_PowerPoint_ptr(int n) const;
   PowerPoint* get_ID_labeled_PowerPoint_ptr(int ID) const;

// PowerPoint creation and manipulation member functions:

   PowerPoint* generate_new_PowerPoint(int ID=-1);
   void generate_powerpoint_group(PowerPoint* powerpoint_ptr);
   int destroy_PowerPoint();

// Message handling member functions:

   bool issue_invocation_message();

  protected:

  private:

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PowerPointsGroup& A);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline PowerPoint* PowerPointsGroup::get_PowerPoint_ptr(int n) const
{
   return dynamic_cast<PowerPoint*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline PowerPoint* PowerPointsGroup::get_ID_labeled_PowerPoint_ptr(int ID) 
   const
{
   return dynamic_cast<PowerPoint*>(get_ID_labeled_Graphical_ptr(ID));
}



#endif // PowerPointsGroup.h



