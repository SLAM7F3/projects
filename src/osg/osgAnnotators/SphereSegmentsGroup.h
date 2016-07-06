// ==========================================================================
// Header file for SPHERESEGMENTSGROUP class
// ==========================================================================
// Last modified on 8/4/11; 8/24/11; 12/2/11
// ==========================================================================

#ifndef SPHERESEGMENTSGROUP_H
#define SPHERESEGMENTSGROUP_H

#include <iostream>
#include "osg/osgAnnotators/AnnotatorsGroup.h"
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "osg/osgAnnotators/SphereSegment.h"

class Clock;
class Ellipsoid_model;
class threevector;

class SphereSegmentsGroup : public GeometricalsGroup, public AnnotatorsGroup
{

  public:

// Initialization, constructor and destructor functions:

   SphereSegmentsGroup(Pass* PI_ptr,threevector* GO_ptr,
      bool display_spokes_flag=false,bool include_blast_flag=true);
   SphereSegmentsGroup(
      Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
      threevector* GO_ptr,bool display_spokes_flag=false,
      bool include_blast_flag=true);

   virtual ~SphereSegmentsGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const SphereSegmentsGroup& f);

// Set & get methods:

   SphereSegment* get_SphereSegment_ptr(int n) const;
   SphereSegment* get_ID_labeled_SphereSegment_ptr(int ID) const;

// SphereSegment creation and manipulation methods:

   SphereSegment* generate_new_canonical_SphereSegment(
      double radius,double az_spread,double el_spread,int ID=-1);
   SphereSegment* generate_new_SphereSegment(
      double radius,double az_min,double az_max,double el_min,double el_max,
      int ID=-1);
   SphereSegment* generate_new_SphereSegment(
      double radius,threevector posn,double az_min,double az_max,
      double el_min,double el_max,int ID=-1);
   SphereSegment* generate_new_hemisphere(
      double radius,threevector& posn,int ID=-1);

//   void generate_spheresegment_group(
//      SphereSegment* spheresegment_ptr,
//      bool display_spokes_flag=false,bool include_blast_flag=true);

   void initialize_vertical_posn(SphereSegment* curr_SphereSegment_ptr);
   bool erase_SphereSegment();
   bool unerase_SphereSegment();

   void change_size(double factor);
//    void reset_colors();

// SphereSegment animation methods:

   void update_display();

// Ascii feature file I/O methods

   void save_info_to_file();
   bool read_info_from_file();

// RF direction finding member functions:

   void generate_RF_direction_segments(std::string rf_dir_filename);

  private:

   bool display_spokes_flag,include_blast_flag;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const SphereSegmentsGroup& f);

   void initialize_new_SphereSegment(
      SphereSegment* curr_SphereSegment_ptr,int OSGsubPAT_number=0);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline SphereSegment* SphereSegmentsGroup::get_SphereSegment_ptr(int n) const
{
   return dynamic_cast<SphereSegment*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline SphereSegment* SphereSegmentsGroup::get_ID_labeled_SphereSegment_ptr(
   int ID) const
{
   return dynamic_cast<SphereSegment*>(get_ID_labeled_Graphical_ptr(ID));
}


#endif // SphereSegmentsGroup.h



