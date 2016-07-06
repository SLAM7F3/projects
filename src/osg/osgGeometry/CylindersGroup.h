// ==========================================================================
// Header file for CYLINDERSGROUP class
// ==========================================================================
// Last modified on 5/1/09; 5/3/09; 6/3/09; 6/4/09; 4/5/14
// ==========================================================================

#ifndef CYLINDERSGROUP_H
#define CYLINDERSGROUP_H

#include <iostream>
#include <string>
#include <osg/Group>
#include "color/colorfuncs.h"
#include "osg/osgGeometry/Cylinder.h"
#include "track/movers_group.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgGeometry/GeometricalsGroup.h"

// class osg::Quat;
class AnimationController;
class Clock;
class Ellipsoid_model;

class CylindersGroup : public GeometricalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   CylindersGroup(
      Pass* PI_ptr,AnimationController* AC_ptr,threevector* GO_ptr=NULL);
   CylindersGroup(
      Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
      threevector* GO_ptr=NULL);
   CylindersGroup(
      Pass* PI_ptr,AnimationController* AC_ptr,
      osgGA::Terrain_Manipulator* CM_3D_ptr,threevector* GO_ptr=NULL);
   virtual ~CylindersGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const CylindersGroup& c);

// Set & get member functions:

   void set_initial_jump_flag(bool flag);
   void set_tall_RTPS_size_flag(bool flag);
   void set_rh(double r,double h);
   void set_radius(double radius);
   double get_radius() const;
   void set_height(double height);
   double get_height() const;
   double get_size() const;
   Cylinder* get_Cylinder_ptr(int n) const;
   Cylinder* get_ID_labeled_Cylinder_ptr(int ID) const;
   void set_movers_group_ptr(movers_group* mg_ptr);
   movers_group* get_movers_group_ptr() const;

// Cylinder creation and manipulation member functions:

   Cylinder* generate_new_Cylinder(
      const threevector& center,const osg::Quat& q,
      colorfunc::Color& permanent_color,
      int n_text_messages=0,double text_displacement=0,double text_size=1,
      bool text_screen_axis_alignment_flag=true,int ID=-1,
      unsigned int OSGsubPAT_number=0);
   Cylinder* generate_new_Cylinder(
      const threevector& center,const osg::Quat& q,
      colorfunc::Color& permanent_color,
      const threevector& text_displacement,int n_text_messages=0,
      double text_size=1,bool text_screen_axis_alignment_flag=true,int ID=-1,
      unsigned int OSGsubPAT_number=0);
   void orient_cylinder_with_ellipsoid_radial_dir(Cylinder* cylinder_ptr);

   void scale_rotate_and_then_translate_cylinder(
      Cylinder* cylinder_ptr,double theta,double phi,const threevector& scale,
      const threevector& trans);
//   bool erase_Cylinder();
//   bool unerase_Cylinder();

   void change_color();
   osg::Group* createCylinderLight(const threevector& light_posn);

// Update member functions:

   void recolor_encountered_vehicle_Cylinders();
   void update_display();

// Message handling member functions:


// Cylinder tracking member functions:

   void follow_selected_cylinder(double min_height_above_cylinder=200);
   int find_Cylinder_ID_given_track_label(int track_label);

// Real-time persistent surveillance specific member functions:

   void set_tall_Cylinder_size();
   void set_short_Cylinder_size();
   void update_track_posn(
      int track_ID,double time,double X,double Y,double Z);
   void update_blueforce_car_posn(
      int blueforce_track_ID,double elapsed_secs,
      double longitude,double latitude,double altitude,
      int specified_UTM_zonenumber);

  protected:

  private:
   
   bool initial_jump_flag,tall_RTPS_size_flag;
   double radius,height;
   double size[4];
   double theta_U,theta_V;
   movers_group* movers_group_ptr;

//    osg::ref_ptr<osgGA::Terrain_Manipulator> CM_3D_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const CylindersGroup& C);

   void initialize_new_Cylinder(
      Cylinder* curr_Cylinder_ptr,bool text_screen_axis_alignment_flag,
      unsigned int OSGsubPAT_number=0);

   bool parse_next_message_in_queue(message& curr_message);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void CylindersGroup::set_initial_jump_flag(bool flag)
{
   initial_jump_flag=flag;
}

inline void CylindersGroup::set_tall_RTPS_size_flag(bool flag)
{
   tall_RTPS_size_flag=flag;
}

inline void CylindersGroup::set_rh(double r,double h) 
{
   radius=r;
   height=h;
}

inline void CylindersGroup::set_radius(double r)
{
   radius=r;
}

inline double CylindersGroup::get_radius() const
{
   return radius;
}

inline void CylindersGroup::set_height(double h)
{
   height=h;
}

inline double CylindersGroup::get_height() const
{
   return height;
}

// --------------------------------------------------------------------------
inline double CylindersGroup::get_size() const
{
   return size[get_ndims()];
}

// --------------------------------------------------------------------------
inline Cylinder* CylindersGroup::get_Cylinder_ptr(int n) const
{
   return dynamic_cast<Cylinder*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline Cylinder* CylindersGroup::get_ID_labeled_Cylinder_ptr(int ID) const
{
   return dynamic_cast<Cylinder*>(get_ID_labeled_Graphical_ptr(ID));
}

inline void CylindersGroup::set_movers_group_ptr(movers_group* mg_ptr)
{
   movers_group_ptr=mg_ptr;
}

inline movers_group* CylindersGroup::get_movers_group_ptr() const
{
   return movers_group_ptr;
}

#endif // CylindersGroup.h



