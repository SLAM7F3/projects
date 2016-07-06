// ==========================================================================
// Header file for ARROWSGROUP class
// ==========================================================================
// Last modified on 9/17/09; 9/29/09; 11/16/10; 4/5/14
// ==========================================================================

#ifndef ARROWSGROUP_H
#define ARROWSGROUP_H

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <osg/AutoTransform>
#include <osg/Group>
#include <osg/Node>
#include "osg/osgGeometry/Arrow.h"
#include "color/colorfuncs.h"
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "image/TwoDarray.h"

class polyhedron;

class ArrowsGroup : public GeometricalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   ArrowsGroup(const int p_ndims,Pass* PI_ptr,threevector* GO_ptr);
   virtual ~ArrowsGroup();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const ArrowsGroup& a);

// Set & get methods:

   void set_altitude_dependent_size_flag(bool flag);
   void set_fixed_label_to_Arrow_ID(int ID,std::string fixed_label);
   Arrow* get_Arrow_ptr(int n) const;
   Arrow* get_ID_labeled_Arrow_ptr(int ID) const;
   void set_size(double size);
   void set_size(double size,double text_size);
   void set_max_text_width(double width);
   void set_colors(colorfunc::Color permanent_color,
                   colorfunc::Color selected_color);
   void update_colors();

// Arrow creation member functions:

   Arrow* generate_new_Arrow(int ID=-1,unsigned int OSGsubPAT_number=0);

// Arrow manipulation member functions:

   void edit_Arrow_label();
   bool assign_fixed_label(int curr_ID);
   int destroy_Arrow();
   bool destroy_Arrow(Arrow* curr_Arrow_ptr);
   void destroy_all_Arrows();
   bool erase_Arrow();
   bool unerase_Arrow();
   Arrow* move_z(int sgn);
   void update_display();

// Vector field generation member functions:

   void generate_flow_field(
      twoDarray* magnitude_twoDarray_ptr,twoDarray* phase_twoDarray_ptr,
      double Z_field,double arrowhead_size_prefactor);
   void display_polyhedron_surface_points(polyhedron* polyhedron_ptr);

  protected:

  private:

   bool altitude_dependent_size_flag;
   std::vector<std::pair<int,std::string> > ID_fixed_label_pairs;
   osg::AutoTransform* AutoTransform_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ArrowsGroup& f);

   void initialize_new_Arrow(Arrow* curr_Arrow_ptr,
                             unsigned int OSGsubPAT_number=0);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ArrowsGroup::set_altitude_dependent_size_flag(bool flag)
{
   altitude_dependent_size_flag=flag;
}

// --------------------------------------------------------------------------
inline Arrow* ArrowsGroup::get_Arrow_ptr(int n) const
{
   return dynamic_cast<Arrow*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline Arrow* ArrowsGroup::get_ID_labeled_Arrow_ptr(int ID) const
{
   return dynamic_cast<Arrow*>(get_ID_labeled_Graphical_ptr(ID));
}


#endif // ArrowsGroup.h



