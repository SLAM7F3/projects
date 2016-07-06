// ==========================================================================
// Header file for ROI_Polyhedron class
// ==========================================================================
// Last updated on 12/21/09
// ==========================================================================

#ifndef ROI_Polyhedron_H
#define ROI_Polyhedron_H

#include <iostream>
#include <vector>
#include "osg/osgGeometry/Polyhedron.h"


class ROI_Polyhedron : public Polyhedron
{

  public:
    
// Initialization, constructor and destructor functions:

   ROI_Polyhedron(Pass* PI_ptr,const threevector& grid_world_origin,
                  osgText::Font* f_ptr,int id,
                  AnimationController* AC_ptr=NULL);
   ROI_Polyhedron(Pass* PI_ptr,const threevector& grid_world_origin,
                  polyhedron* p_ptr,osgText::Font* f_ptr,int id,
                  AnimationController* AC_ptr=NULL);
   virtual ~ROI_Polyhedron();
   friend std::ostream& operator<< (
      std::ostream& outstream,const ROI_Polyhedron& P);

// Set & get methods:

// ROI_Polyhedron generation methods:

   void build_current_polyhedron(double curr_t,int pass_number);
   
  protected:

  private:

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ROI_Polyhedron& f);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

/*
inline LineSegmentsGroup* ROI_Polyhedron::get_LineSegmentsGroup_ptr()
{
   return LineSegmentsGroup_ptr;
}
*/


#endif // ROI_Polyhedron.h



