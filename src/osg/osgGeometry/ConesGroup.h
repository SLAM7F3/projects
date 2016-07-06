// ==========================================================================
// Header file for CONESGROUP class
// ==========================================================================
// Last modified on 5/6/07; 9/7/09; 9/8/09; 4/5/14
// ==========================================================================

#ifndef CONESGROUP_H
#define CONESGROUP_H

#include <iostream>
#include <string>
#include <osg/Group>
#include "osg/osgGeometry/Cone.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgGeometry/GeometricalsGroup.h"

class ConesGroup : public GeometricalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   ConesGroup(Pass* PI_ptr,threevector* GO_ptr);
   ConesGroup(Pass* PI_ptr,osgGA::Custom3DManipulator* CM_ptr,
              threevector* GO_ptr);
   virtual ~ConesGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const ConesGroup& c);

// Set & get methods:

   void set_rh(double r,double h);
   double get_radius() const;
   double get_height() const;
   double get_size() const;
   Cone* get_Cone_ptr(int n) const;
   Cone* get_ID_labeled_Cone_ptr(int ID) const;

// Cone creation and manipulation methods:

   Cone* generate_new_Cone(
      const threevector& tip,const threevector& base,double radius,
      int ID=-1,unsigned int OSGsubPAT_number=0);
   Cone* generate_new_Cone(int ID=-1,unsigned int OSGsubPAT_number=0);
   void transform_cone(
      Cone* Cone_ptr,const threevector& tip,const threevector& base,
      double radius);
//   bool erase_Cone();
//   bool unerase_Cone();

   void change_size(double factor);
   void set_altitude_dependent_size(
      double prefactor,double max_size,double min_size,
      double z_min,double z_max);

   void reset_colors();
   void change_color();
   void update_display();
   
   osg::Group* createConeLight(const threevector& light_posn);

// View frustum cone methods:

   void draw_FOV_cone();

  protected:

   double radius,height;

  private:

   double size[4];
   osg::ref_ptr<osgGA::Custom3DManipulator> CM_3D_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ConesGroup& f);

   void initialize_new_Cone(Cone* curr_Cone_ptr,unsigned int OSGsubPAT_number);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ConesGroup::set_rh(double r,double h) 
{
   radius=r;
   height=h;
   size[3]=radius;
}

inline double ConesGroup::get_radius() const
{
   return radius;
}

inline double ConesGroup::get_height() const
{
   return height;
}

// --------------------------------------------------------------------------
inline double ConesGroup::get_size() const
{
   return size[get_ndims()];
}

// --------------------------------------------------------------------------
inline Cone* ConesGroup::get_Cone_ptr(int n) const
{
   return dynamic_cast<Cone*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline Cone* ConesGroup::get_ID_labeled_Cone_ptr(int ID) const
{
   return dynamic_cast<Cone*>(get_ID_labeled_Graphical_ptr(ID));
}


#endif // ConesGroup.h



