// ==========================================================================
// Header file for PYRAMIDSGROUP class
// ==========================================================================
// Last modified on 7/20/07; 7/21/07; 7/22/07; 4/5/14
// ==========================================================================

#ifndef PYRAMIDSGROUP_H
#define PYRAMIDSGROUP_H

#include <iostream>
#include <string>
#include <vector>
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/Pyramid.h"

class AnimationController;
class pyramid;

class PyramidsGroup : public PolyhedraGroup
{

  public:

// Initialization, constructor and destructor functions:

   PyramidsGroup(Pass* PI_ptr,threevector* GO_ptr,
                 AnimationController* AC_ptr=NULL);
   virtual ~PyramidsGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const PyramidsGroup& P);

// Set & get methods:

   Pyramid* get_Pyramid_ptr(int n) const;
   Pyramid* get_last_Pyramid_ptr() const;
   Pyramid* get_ID_labeled_Pyramid_ptr(int ID) const;

// Pyramid creation and manipulation methods:

   Pyramid* generate_new_Pyramid(int ID=-1);
   Pyramid* generate_new_Pyramid(pyramid* p_ptr,int ID=-1);
   void generate_pyramid_geode(Pyramid* Pyramid_ptr);
   void update_display();
   

  protected:

  private:

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PyramidsGroup& P);

   void initialize_new_Pyramid(Pyramid* Pyramid_ptr,
                               unsigned int OSGsubPAT_number=0);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline Pyramid* PyramidsGroup::get_Pyramid_ptr(int n) const
{
   return dynamic_cast<Pyramid*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline Pyramid* PyramidsGroup::get_last_Pyramid_ptr() const
{
   return get_Pyramid_ptr(get_n_Graphicals()-1);
}

// --------------------------------------------------------------------------
inline Pyramid* PyramidsGroup::get_ID_labeled_Pyramid_ptr(int ID) const
{
   return dynamic_cast<Pyramid*>(get_ID_labeled_Graphical_ptr(ID));
}

#endif // PyramidsGroup.h



