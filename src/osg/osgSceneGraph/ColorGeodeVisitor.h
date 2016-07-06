// ========================================================================
// ColorGeodeVisitor header file
// ========================================================================
// Last updated on 5/28/09; 2/5/11; 11/27/11
// ========================================================================

#ifndef COLORGEODEVISITOR_H
#define COLORGEODEVISITOR_H

#include <iostream>
#include <map>
#include <osg/NodeCallback>
#include "math/lttwovector.h"
#include "osg/osgSceneGraph/MyNodeVisitor.h"
#include "image/TwoDarray.h"
#include "math/twovector.h"

class ColorMap;
// class osg::Geometry;

class ColorGeodeVisitor : public MyNodeVisitor
{
  public: 

   ColorGeodeVisitor();
   ColorGeodeVisitor(ColorMap* height_CM_ptr,ColorMap* prob_CM_ptr=NULL);
   virtual ~ColorGeodeVisitor(); 

// Set & get methods:

   void set_fixed_to_mutable_colors_flag(bool flag);
   bool get_fixed_to_mutable_colors_flag() const;
   void set_mutable_to_fixed_colors_flag(bool flag);
   bool get_mutable_to_fixed_colors_flag() const;
   void set_probabilities_magnification(double factor);
   
   void set_height_ColorMap_ptr(ColorMap* height_CM_ptr);
   ColorMap* get_height_ColorMap_ptr();
   void set_prob_ColorMap_ptr(ColorMap* prob_CM_ptr);
   ColorMap* get_prob_ColorMap_ptr();

   void set_ptwoDarray_ptr(twoDarray* input_ptwoDarray_ptr);
   unsigned int get_n_ptwoDarray_ptrs() const;
   void push_back_ptwoDarray_ptr(twoDarray* input_ptwoDarray_ptr);
   void push_back_ptwoDarray_ptrs(
      const std::vector<twoDarray*> input_ptwoDarray_ptrs);
   void clear_ptwoDarray_ptrs();

   virtual void apply(osg::Geode& currGeode);

   void color_geometry_vertices(osg::Geometry* curr_Geometry_ptr);

// LONG_LAT_ID_map member functions:

   typedef std::map<twovector,int,lttwovector > LONGLATID_MAP;
   LONGLATID_MAP* long_lat_ID_map_ptr;

   LONGLATID_MAP* get_long_lat_ID_map_ptr();
   const LONGLATID_MAP* get_long_lat_ID_map_ptr() const;
   int get_ptwoDarray_ID(const twovector& longlat);
   void insert_long_lat_ID_map_entry(
      const twovector& longlat,int ptwoDarray_ID);
   void clear_long_lat_ID_map();
   int get_long_lat_ID_map_size() const;
   void print_long_lat_ID_map_contents() const;

  protected:

  private:
   
   bool fixed_to_mutable_colors_flag,mutable_to_fixed_colors_flag;
   bool magnify_probabilities_flag;
   int p_ID_start,p_ID_stop,p_counter;
   double p_magnification_factor;
   ColorMap *height_ColorMap_ptr,*prob_ColorMap_ptr;
   std::vector<twoDarray*> ptwoDarray_ptrs;

   void allocate_member_objects();
   void initialize_member_objects();

   void magnify_p(double& p);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline ColorGeodeVisitor::LONGLATID_MAP* 
ColorGeodeVisitor::get_long_lat_ID_map_ptr()
{
   return long_lat_ID_map_ptr;
}

inline const ColorGeodeVisitor::LONGLATID_MAP* 
ColorGeodeVisitor::get_long_lat_ID_map_ptr() const
{
   return long_lat_ID_map_ptr;
}

inline void ColorGeodeVisitor::set_fixed_to_mutable_colors_flag(bool flag)
{
   fixed_to_mutable_colors_flag=flag;
   mutable_to_fixed_colors_flag=!flag;

//   std::cout << "inside CGV::set_fixed_to_mut_colors" << std::endl;
//   std::cout << "fixed_to_mutable_colors_flag = " 
//             << fixed_to_mutable_colors_flag << std::endl;
//   std::cout << "mutable_to_fixed_colors_flag = "
//             << mutable_to_fixed_colors_flag << std::endl;
}

inline bool ColorGeodeVisitor::get_fixed_to_mutable_colors_flag() const
{
   return fixed_to_mutable_colors_flag;
}

inline void ColorGeodeVisitor::set_mutable_to_fixed_colors_flag(bool flag)
{
   mutable_to_fixed_colors_flag=flag;
   fixed_to_mutable_colors_flag=!flag;

//   std::cout << "inside CGV::set_mutable_to_fixed_colors" << std::endl;
//   std::cout << "fixed_to_mutable_colors_flag = " 
//             << fixed_to_mutable_colors_flag << std::endl;
//   std::cout << "mutable_to_fixed_colors_flag = "
//             << mutable_to_fixed_colors_flag << std::endl;
}

inline bool ColorGeodeVisitor::get_mutable_to_fixed_colors_flag() const
{
   return mutable_to_fixed_colors_flag;
}

inline void ColorGeodeVisitor::set_probabilities_magnification(double factor)
{
   p_magnification_factor=factor;
}

inline void ColorGeodeVisitor::magnify_p(double& p)
{
   p *= p_magnification_factor;
   if (p > 1) p=1;
}



#endif 
