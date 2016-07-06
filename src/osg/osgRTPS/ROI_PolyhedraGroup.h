// ==========================================================================
// Header file for ROI_POLYHEDRAGROUP class
// ==========================================================================
// Last modified on 12/21/09
// ==========================================================================

#ifndef ROI_POLYHEDRAGROUP_H
#define ROI_POLYHEDRAGROUP_H

#include <iostream>
#include <string>
#include <vector>
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgRTPS/ROI_Polyhedron.h"
//#include "Qt/rtps/RTPSMessenger.h"

class AnimationController;
class polyhedron;
// class RTPSMessenger;

class ROI_PolyhedraGroup : public PolyhedraGroup
{

  public:

// Initialization, constructor and destructor functions:

   ROI_PolyhedraGroup(Pass* PI_ptr,threevector* GO_ptr,
                      AnimationController* AC_ptr=NULL);
   virtual ~ROI_PolyhedraGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const ROI_PolyhedraGroup& f);

// Set & get methods:

   ROI_Polyhedron* get_ROI_Polyhedron_ptr(int n) const;
   ROI_Polyhedron* get_ID_labeled_ROI_Polyhedron_ptr(int ID) const;

// ROI_Polyhedron creation and manipulation methods:

   ROI_Polyhedron* generate_new_ROI_Polyhedron(
      polyhedron* p_ptr,int ID=-1,int OSGsubPAT_number=0);
   ROI_Polyhedron* generate_bbox(int Polyhedra_subgroup,double alpha=0.25);

// Polyhedron destruction member function:

   void destroy_all_ROI_Polyhedra();
   bool destroy_ROI_Polyhedron();
   bool destroy_ROI_Polyhedron(int ID);
   bool destroy_ROI_Polyhedron(ROI_Polyhedron* curr_ROI_Polyhedron_ptr);

/*
// RTPS message handling member functions:

   void pushback_RTPSMessenger_ptr(RTPSMessenger* M_ptr);
   RTPSMessenger* get_RTPSMessenger_ptr();
   const RTPSMessenger* get_RTPSMessenger_ptr() const;
   int get_n_RTPSMessenger_ptrs() const;
   RTPSMessenger* get_RTPSMessenger_ptr(int i);
   const RTPSMessenger* get_RTPSMessenger_ptr(int i) const;
*/

//   void broadcast_bbox_corners();
   
  protected:

  private:

//    std::vector<RTPSMessenger*> RTPSMessenger_ptrs;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ROI_PolyhedraGroup& P);

   void initialize_new_ROI_Polyhedron(
      ROI_Polyhedron* ROI_Polyhedron_ptr,int OSGsubPAT_number=0);

   virtual bool parse_next_message_in_queue(message& curr_message);
   void update_display();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline ROI_Polyhedron* ROI_PolyhedraGroup::get_ROI_Polyhedron_ptr(int n) const
{
   return dynamic_cast<ROI_Polyhedron*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline ROI_Polyhedron* ROI_PolyhedraGroup::get_ID_labeled_ROI_Polyhedron_ptr(
   int ID) const
{
   return dynamic_cast<ROI_Polyhedron*>(get_ID_labeled_Graphical_ptr(ID));
}

#endif // ROI_ROI_PolyhedraGroup.h



