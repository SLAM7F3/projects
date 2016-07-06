// ==========================================================================
// Header file for COLORMAP class
// ==========================================================================
// Last modified on 11/19/11; 12/8/11; 12/25/11
// ==========================================================================

#ifndef COLORMAP_H
#define COLORMAP_H

#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <osg/Array>
#include <osg/NodeCallback>
#include <osg/observer_ptr>
#include "color/colorfuncs.h"
#include "math/fourvector.h"
#include "general/outputfuncs.h"

class ColorMap
{

  public:

// Initialization, constructor and destructor functions:

   ColorMap();
   ColorMap(std::string dir_name,int p_map=9,int p_dependent_var=2);
   ~ColorMap();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const ColorMap& C);

// Set & get member functions:

   void set_mapnumber(int m);
   int get_map_number() const;
   void set_cyclic_frac_offset(double f);
   double get_cyclic_frac_offset();
   int* get_map_number_ptr();
   int get_n_dependent_vars() const;
   void set_dependent_var(int d);
   int get_dependent_var() const;

   void set_max_value(int i,double value);
   double get_max_value(int i) const;
   void set_max_threshold(double threshold);
   void set_max_threshold(int i,double threshold);

   void set_min_value(int i,double value);
   double get_min_value(int i) const;
   void set_min_threshold(double threshold);
   void set_min_threshold(int i,double threshold);

   double get_max_threshold() const;
   double get_max_threshold(int i) const;
   double get_min_threshold() const;
   double get_min_threshold(int i) const;
   int get_n_map_colors() const;

// Color loading & retrieval member functions:

   void load(int m,std::vector<osg::Vec4>& colors);
   void change_dependent_coloring_var(int var_increment);

   void increment_mapnumber(int m);
   const osg::Vec4ub& retrieve_frac_color(double curr_frac) const;
   const osg::Vec4ub& retrieve_curr_color(double curr_value) const;
   const osg::Vec4ub& retrieve_curr_color(double curr_value,int depend_var) 
      const;

   const colorfunc::RGBA retrieve_curr_RGBA(double curr_value) const;
   const colorfunc::RGBA retrieve_curr_RGBA(double curr_value,int depend_var) 
      const;

// Member functions copied from Ross' Colormap class:

   unsigned long getCurrentUpdateIndex() const;
   void IncrementUpdateIndex();
   void setEnabled( osg::Node& n, bool enable );
   bool isEnabled( osg::Node& n );
   osg::NodeCallback* get_UpdateCallback_ptr();

  private:

   std::string directory_name;
   std::vector<std::string> colormap_name;
   int N_COLORMAPS,map,dependent_var;
   unsigned long _currentUpdateIndex;
   double cyclic_frac_offset;
   fourvector max_values,min_values;
   fourvector max_threshold,min_threshold;
   std::vector<osg::Vec4ubArray*> colorarray_ptrs;
        
   typedef std::list< osg::observer_ptr<osg::Node> > NodeObserverList;
   NodeObserverList _roots;
   osg::ref_ptr<osg::NodeCallback> updateCallback_refptr;

   osg::Vec4ub null_color;

   void initialize_member_objects();
   void allocate_member_objects();
   void docopy(const ColorMap& P);

   void load_all_maps();
   void load(int m,osg::Vec4ubArray* curr_colorarray_ptr);
   osg::Vec4ubArray* get_curr_colorarray_ptr();
   const osg::Vec4ubArray* get_curr_colorarray_ptr() const;

   void update();
   void updateState();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ColorMap::set_mapnumber(int m)
{
//   std::cout << "inside ColorMap::set_mapnumber()" << std::endl;
//   std::cout << "map = " << map << " m = " << m << std::endl;
   map=m;
   map=modulo(map,N_COLORMAPS);
   std::cout << "Current colormap = " << colormap_name[map] << std::endl;
}

inline int ColorMap::get_map_number() const
{
   return map;
}

inline int* ColorMap::get_map_number_ptr() 
{
   return &map;
}

inline void ColorMap::set_cyclic_frac_offset(double f)
{
   cyclic_frac_offset=f;
}

inline double ColorMap::get_cyclic_frac_offset()
{
   return cyclic_frac_offset;
}

inline int ColorMap::get_n_dependent_vars() const
{
   const int N_DEPENDENT_VARS=4;
   return N_DEPENDENT_VARS;
}

inline void ColorMap::set_dependent_var(int d)
{
   dependent_var=d;
}

inline int ColorMap::get_dependent_var() const
{
   return dependent_var;
}

inline void ColorMap::set_max_value(int i,double value)
{
   max_values.put(i,value);
}

inline double ColorMap::get_max_value(int i) const
{
   return max_values.get(i);
}

inline void ColorMap::set_max_threshold(double threshold)
{
   set_max_threshold(dependent_var,threshold);
}

inline void ColorMap::set_max_threshold(int i,double threshold)
{
//   std::cout << "inside ColorMap::set_max_threshold()" << std::endl;
   
   std::string dependent_var;
   switch (i)
   {
      case 0:
         dependent_var="X";
         break;
      case 1:
         dependent_var="Y";
         break;
      case 2:
         dependent_var="Z";
         break;
      case 3:
         dependent_var="P";
         break;
   }
//   std::cout << "Max threshold for " << dependent_var 
//             << " = " << threshold << std::endl;
   max_threshold.put(i,threshold);
}

inline void ColorMap::set_min_value(int i,double value) 
{
   min_values.put(i,value);
}

inline double ColorMap::get_min_value(int i) const
{
   return min_values.get(i);
}

inline void ColorMap::set_min_threshold(double threshold)
{
   set_min_threshold(dependent_var,threshold);
}

inline void ColorMap::set_min_threshold(int i,double threshold)
{
//   std::cout << "inside ColorMap::set_min_threshold" << std::endl;
   
   std::string dependent_var;
   switch (i)
   {
      case 0:
         dependent_var="X";
         break;
      case 1:
         dependent_var="Y";
         break;
      case 2:
         dependent_var="Z";
         break;
      case 3:
         dependent_var="P";
         break;
   }
//   std::cout << "Min threshold for " << dependent_var 
//             << " = " << threshold << std::endl;
   min_threshold.put(i,threshold);
//   outputfunc::enter_continue_char();
}

inline double ColorMap::get_max_threshold() const
{
   return get_max_threshold(dependent_var);
}

inline double ColorMap::get_max_threshold(int i) const
{
   return max_threshold.get(i);
}

inline double ColorMap::get_min_threshold() const
{
   return get_min_threshold(dependent_var);
}

inline double ColorMap::get_min_threshold(int i) const
{
   return min_threshold.get(i);
}

inline int ColorMap::get_n_map_colors() const
{
   return get_curr_colorarray_ptr()->size();
}

inline unsigned long ColorMap::getCurrentUpdateIndex() const 
{ 
   return _currentUpdateIndex; 
}

inline void ColorMap::IncrementUpdateIndex() 
{ 
//   std::cout << "inside ColorMap::IncrementUpdateIndex()" << std::endl;
   _currentUpdateIndex++; 
//   std::cout << "currUpdateIndex = " << _currentUpdateIndex << std::endl;
}

inline void ColorMap::increment_mapnumber(int m)
{
//   std::cout << "inside ColorMap::increment_mapnumber()" << std::endl;
//   std::cout << "map = " << map << " m = " << m << std::endl;
   map += m;
   map=modulo(map,N_COLORMAPS);
   std::cout << "Current colormap = " << colormap_name[map] << std::endl;
}


#endif // ColorMap.h



