// ==========================================================================
// Header file for CompassHUD class 
// ==========================================================================
// Last modified on 6/6/09; 6/7/09; 8/25/09; 2/3/10
// ==========================================================================

#ifndef CompassHUD_H
#define CompassHUD_H

#include <osg/Depth>
#include <osg/PositionAttitudeTransform>
#include <osg/ShapeDrawable>
#include "color/colorfuncs.h"
#include "osg/GenericHUD.h"

class CompassHUD : public GenericHUD
{
  public:

   CompassHUD(colorfunc::Color compass_color);

// Set & get member functions:

   void set_nadir_oriented_compass_flag(bool flag);
   bool get_nadir_oriented_compass_flag() const;
//   void set_color(colorfunc::Color compass_color);
   void set_north_az_offset(double offset);
   void set_nodemask(int mask_value);

   void rotate_compass(double az);

  protected:

  private:

   bool nadir_oriented_compass_flag;
   int n_cones;
   double north_az_offset;
   colorfunc::Color compass_color;

   osg::Geode *N_text_geode_ptr,*E_text_geode_ptr;
   osg::ref_ptr<osg::PositionAttitudeTransform> N_PAT_refptr,E_PAT_refptr;
   osg::ref_ptr<osg::MatrixTransform> compass_transform_refptr;

   osg::ref_ptr<osg::Depth> depth_off_refptr;
   

   void allocate_member_objects();
   void initialize_member_objects();

   void initialize_compass_transform();
   void update_compass();

   osg::MatrixTransform* generate_arrow_geode();
   osg::Geode* generate_text_label_geode(std::string label);
   void generate_compass_drawables();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void CompassHUD::set_nadir_oriented_compass_flag(bool flag)
{
   nadir_oriented_compass_flag=flag;
}

inline bool CompassHUD::get_nadir_oriented_compass_flag() const
{
   return nadir_oriented_compass_flag;
}

inline void CompassHUD::set_north_az_offset(double offset)
{
   north_az_offset=offset;
}

/*
inline void CompassHUD::set_color(colorfunc::Color compass_color)
{
   this->compass_color=compass_color;
}
*/


#endif 
