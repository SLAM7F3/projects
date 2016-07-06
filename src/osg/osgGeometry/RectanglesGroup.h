// ==========================================================================
// Header file for RECTANGLESGROUP class
// ==========================================================================
// Last modified on 2/17/09; 7/9/11; 7/10/11; 4/5/14
// ==========================================================================

#ifndef RECTANGLESGROUP_H
#define RECTANGLESGROUP_H

#include <iostream>
#include <osg/Group>
#include <osg/Node>
#include "osg/osgGeometry/Rectangle.h"
#include "osg/osgGeometry/GeometricalsGroup.h"

class RectanglesGroup : public GeometricalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   RectanglesGroup(const int p_ndims,Pass* PI_ptr);
   virtual ~RectanglesGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const RectanglesGroup& f);

// Set & get methods:

   double get_size() const;
   Rectangle* get_Rectangle_ptr(int n) const;
   Rectangle* get_ID_labeled_Rectangle_ptr(int ID) const;
   Rectangle* get_selected_Rectangle_ptr() const;

// Rectangle creation and manipulation methods:

   Rectangle* generate_new_Rectangle(
      int ID=-1,unsigned int OSGsubPAT_number=0);
   Rectangle* generate_new_Rectangle(
      const threevector& V,int ID=-1,unsigned int OSGsubPAT_number=0);
   bool erase_Rectangle();
   bool unerase_Rectangle();
   void reset_colors();

// Ascii file I/O methods

   void save_info_to_file();
   void read_info_from_file();

  protected:

  private:

   double size[4];
   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const RectanglesGroup& f);

   void initialize_new_Rectangle(
      const threevector& V,Rectangle* curr_Rectangle_ptr,
      unsigned int OSGsubPAT_number=0);

   void update_display();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline double RectanglesGroup::get_size() const
{
   return size[get_ndims()];
}

// --------------------------------------------------------------------------
inline Rectangle* RectanglesGroup::get_Rectangle_ptr(int n) const
{
   return dynamic_cast<Rectangle*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline Rectangle* RectanglesGroup::get_ID_labeled_Rectangle_ptr(int ID) const
{
   return dynamic_cast<Rectangle*>(get_ID_labeled_Graphical_ptr(ID));
}

// --------------------------------------------------------------------------
inline Rectangle* RectanglesGroup::get_selected_Rectangle_ptr() const
{
   int selected_ID=get_selected_Graphical_ID();
   return dynamic_cast<Rectangle*>(get_ID_labeled_Graphical_ptr(
      selected_ID));
}


#endif // RectanglesGroup.h



