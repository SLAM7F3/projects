// ==========================================================================
// Header file for PLANESGROUP class
// ==========================================================================
// Last modified on 11/24/06; 1/4/07; 1/9/07; 2/18/08
// ==========================================================================

#ifndef PLANESGROUP_H
#define PLANESGROUP_H

#include <vector>
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "osg/osgGeometry/Plane.h"
#include "math/threevector.h"

class FeaturesGroup;
class plane;

class PlanesGroup : public GraphicalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   PlanesGroup(Pass* PI_ptr);
   virtual ~PlanesGroup();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const PlanesGroup& p);

// Set & get methods:

   Plane* get_Plane_ptr(int n) const;
   Plane* get_ID_labeled_Plane_ptr(int ID) const;

// Plane generation & manipulation methods:

   Plane* generate_new_canonical_Plane(int ID=-1);
   Plane* generate_new_Plane(const plane& p,int ID=-1);
   void generate_planar_normal_segment(int ID);
   void generate_plane_from_features();
   void flip_planar_parity(int ID);

  protected:

  private:

   FeaturesGroup* FeaturesGroup_3D_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PlanesGroup& G);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline Plane* PlanesGroup::get_Plane_ptr(int n) const
{
   return dynamic_cast<Plane*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline Plane* PlanesGroup::get_ID_labeled_Plane_ptr(int ID) const
{
   return dynamic_cast<Plane*>(get_ID_labeled_Graphical_ptr(ID));
}

#endif // PlanesGroup.h



