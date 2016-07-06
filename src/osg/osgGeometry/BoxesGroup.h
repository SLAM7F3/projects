// ==========================================================================
// Header file for BOXESGROUP class
// ==========================================================================
// Last modified on 1/9/07; 1/26/07; 2/7/07; 12/2/11
// ==========================================================================

#ifndef BOXESGROUP_H
#define BOXESGROUP_H

#include <iostream>
#include <string>
#include <osg/Group>
#include "osg/osgGeometry/Box.h"
#include "osg/osgGeometry/GeometricalsGroup.h"

class AnimationController;

class BoxesGroup : public GeometricalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   BoxesGroup(Pass* PI_ptr,threevector* GO_ptr);
   BoxesGroup(Pass* PI_ptr,threevector* GO_ptr,AnimationController* AC_ptr);
   BoxesGroup(Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
              threevector* GO_ptr);
   virtual ~BoxesGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const BoxesGroup& f);

// Set & get methods:

   void set_wlh(double w,double l,double h);
   void set_wlhd(double w,double l,double h,double d);
   double get_width() const;
   double get_length() const;
   double get_height() const;
   double get_size() const;
   Box* get_Box_ptr(int n) const;
   Box* get_ID_labeled_Box_ptr(int ID) const;

// Box creation and manipulation methods:

   Box* generate_new_Box(const threevector& V,int ID=-1);
   Box* generate_new_canonical_Box(int ID=-1);
//   void initialize_posn(Box* curr_Box_ptr);
//   bool erase_Box();
//   bool unerase_Box();

   void change_size(double factor);
   void change_color();
   void deselect_all_faces();
   void move_z(int sgn);
   void update_mybox(Box* Box_ptr);

   void update_display();

// Ascii feature file I/O methods:

   void save_info_to_file();
//   void read_info_from_file(
//      std::string segments_filename,std::vector<double>& curr_time,
//      std::vector<int>& pass_number,std::vector<threevector>& V1,
//      std::vector<threevector>& V2,std::vector<colorfunc::Color>& color);

   osg::Group* createBoxLight(const threevector& light_posn);

  protected:

   double width,length,height,selected_face_displacement;

  private:

   double size[4];
   
   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const BoxesGroup& f);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void BoxesGroup::set_wlh(double w,double l,double h) 
{
   width=w;
   length=l;
   height=h;
}

inline void BoxesGroup::set_wlhd(double w,double l,double h,double d) 
{
   width=w;
   length=l;
   height=h;
   selected_face_displacement=d;
}

inline double BoxesGroup::get_width() const
{
   return width;
}

inline double BoxesGroup::get_length() const
{
   return length;
}

inline double BoxesGroup::get_height() const
{
   return height;
}

// --------------------------------------------------------------------------
inline double BoxesGroup::get_size() const
{
   return size[get_ndims()];
}

// --------------------------------------------------------------------------
inline Box* BoxesGroup::get_Box_ptr(int n) const
{
   return dynamic_cast<Box*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline Box* BoxesGroup::get_ID_labeled_Box_ptr(int ID) const
{
   return dynamic_cast<Box*>(get_ID_labeled_Graphical_ptr(ID));
}


#endif // BoxesGroup.h



