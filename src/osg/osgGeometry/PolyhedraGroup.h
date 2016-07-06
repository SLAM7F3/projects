// ==========================================================================
// Header file for POLYHEDRAGROUP class
// ==========================================================================
// Last modified on 1/23/12; 3/13/12; 4/21/12
// ==========================================================================

#ifndef POLYHEDRAGROUP_H
#define POLYHEDRAGROUP_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "osg/osgGeometry/Polyhedron.h"

class AnimationController;
class BuildingsGroup;
class ParkingLotsGroup;
class polyhedron;
class RoadsGroup;

class PolyhedraGroup : public GeometricalsGroup
{

  public:

   typedef std::map<polyhedron*,Polyhedron*> POLYHEDRON_MAP;

// Initialization, constructor and destructor functions:

   PolyhedraGroup(Pass* PI_ptr,threevector* GO_ptr,
                  AnimationController* AC_ptr=NULL);
   virtual ~PolyhedraGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const PolyhedraGroup& f);

// Set & get methods:

   Polyhedron* get_Polyhedron_ptr(int n) const;
   Polyhedron* get_ID_labeled_Polyhedron_ptr(int ID) const;
   Polyhedron* get_selected_Polyhedron_ptr() const;
   void set_bbox_sidelength(double length);
   void set_bbox_height(double height);
   void set_bbox_color_str(std::string color_str);
   void set_bbox_label_color_str(std::string label_color_str);
   void set_altitude_dependent_volume_alphas_flag(bool flag);
   void set_min_alpha_altitude(double min_alt);
   void set_max_alpha_altitude(double max_alt);
   void set_OFF_subdir(std::string subdir);
   POLYHEDRON_MAP* get_Polyhedron_map_ptr();
   const POLYHEDRON_MAP* get_Polyhedron_map_ptr() const;

// Polyhedron creation and manipulation methods:

   Polyhedron* generate_new_Polyhedron(int ID=-1,int OSGsubPAT_number=0);
   Polyhedron* generate_new_Polyhedron(
      polyhedron* p_ptr,int ID=-1,int OSGsubPAT_number=0);

   Polyhedron* generate_bbox(int Polyhedra_subgroup,double alpha=0.25);
   Polyhedron* generate_bbox(
      int Polyhedra_subgroup,colorfunc::Color& bbox_color,double alpha=0.25);
   Polyhedron* generate_bbox(
      double min_X,double max_X,double min_Y,double max_Y,
      int Polyhedra_subgroup,colorfunc::Color& bbox_color,double alpha=0.25);

// Polyhedron selection member functions:

   void display_selected_Polyhedron_vertex();
   void unselect_Polyhedra_vertices();
   void unselect_Polyhedra_edges();
   int increment_selected_Polyhedron();
   int decrement_selected_Polyhedron();

// Polyhedron importing member functions:

   void import_new_Polyhedra();
   Polyhedron* import_new_Polyhedron(std::string OFF_filename);
   void generate_Building_Polyhedra(BuildingsGroup* BuildingsGroup_ptr);
   void generate_ParkingLot_Polyhedra(ParkingLotsGroup* ParkingLotsGroup_ptr);
   void generate_Road_Polyhedra(RoadsGroup* RoadsGroup_ptr);
   double fit_constant_z_ground();

// Polyhedron destruction member function:

   void destroy_all_Polyhedra();
   bool destroy_Polyhedron();
   bool destroy_Polyhedron(int ID);
   bool destroy_Polyhedron(Polyhedron* curr_Polyhedron_ptr);
   
// Polyhedra display member functions:

   void adjust_Polyhedra_alphas();
   void set_altitude_dependent_Polyhedron_alpha(Polyhedron* Polyhedron_ptr);

// Message handling member functions

   void broadcast_bbox_corners();

// NYC demo member functions:

   void generate_skyscraper_bbox(
      polyhedron& bbox_3D,colorfunc::Color box_color,double alpha=0.2);
   void generate_skyscraper_bboxes();
   
  protected:

   bool altitude_dependent_volume_alphas_flag;
   std::string bbox_color_str;
   POLYHEDRON_MAP* Polyhedron_map_ptr;

  private:

   double bbox_sidelength,bbox_height;
   double min_alpha_altitude,max_alpha_altitude;
   std::string bbox_label_color_str,OFF_subdir;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PolyhedraGroup& P);

   void initialize_new_Polyhedron(
      Polyhedron* Polyhedron_ptr,int OSGsubPAT_number=0);

   virtual bool parse_next_message_in_queue(message& curr_message);
   void update_display();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline Polyhedron* PolyhedraGroup::get_Polyhedron_ptr(int n) const
{
   return dynamic_cast<Polyhedron*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline Polyhedron* PolyhedraGroup::get_ID_labeled_Polyhedron_ptr(
   int ID) const
{
   return dynamic_cast<Polyhedron*>(get_ID_labeled_Graphical_ptr(ID));
}

inline Polyhedron* PolyhedraGroup::get_selected_Polyhedron_ptr() const
{
   return dynamic_cast<Polyhedron*>(get_selected_Graphical_ptr());
}

inline void PolyhedraGroup::set_bbox_sidelength(double length)
{
   bbox_sidelength=length;
}

inline void PolyhedraGroup::set_bbox_height(double height)
{
   bbox_height=height;
}

inline void PolyhedraGroup::set_bbox_color_str(std::string color_str)
{
   bbox_color_str=color_str;
}

inline void PolyhedraGroup::set_bbox_label_color_str(
   std::string label_color_str)
{
   bbox_label_color_str=label_color_str;
}

inline void PolyhedraGroup::set_altitude_dependent_volume_alphas_flag(
   bool flag)
{
   altitude_dependent_volume_alphas_flag=flag;
}

inline void PolyhedraGroup::set_min_alpha_altitude(double min_alt)
{
   min_alpha_altitude=min_alt;
}

inline void PolyhedraGroup::set_max_alpha_altitude(double max_alt)
{
   max_alpha_altitude=max_alt;
}

inline void PolyhedraGroup::set_OFF_subdir(std::string subdir)
{
   OFF_subdir=subdir;
}

inline PolyhedraGroup::POLYHEDRON_MAP* PolyhedraGroup::get_Polyhedron_map_ptr()
{
   return Polyhedron_map_ptr;
}

inline const PolyhedraGroup::POLYHEDRON_MAP* 
PolyhedraGroup::get_Polyhedron_map_ptr() const
{
   return Polyhedron_map_ptr;
}


#endif // PolyhedraGroup.h



