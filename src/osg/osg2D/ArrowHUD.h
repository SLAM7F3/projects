// ==========================================================================
// Header file for ArrowHUD class 
// ==========================================================================
// Last modified on 9/11/11
// ==========================================================================

#ifndef ArrowHUD_H
#define ArrowHUD_H

#include <osg/Depth>
#include <osg/PositionAttitudeTransform>
#include <osg/ShapeDrawable>
#include "color/colorfuncs.h"
#include "osg/GenericHUD.h"

class ArrowHUD : public GenericHUD
{
  public:

   ArrowHUD(colorfunc::Color arrow_color);

// Set & get member functions:

   void set_N_nodemask(int mask_value);
   void set_E_nodemask(int mask_value);
   void set_S_nodemask(int mask_value);
   void set_W_nodemask(int mask_value);
   void set_arrow_directions();

  protected:

  private:

   int n_cones;
   colorfunc::Color arrow_color;

   osg::ref_ptr<osg::PositionAttitudeTransform> 
      N_PAT_refptr,E_PAT_refptr,S_PAT_refptr,W_PAT_refptr;
   osg::ref_ptr<osg::MatrixTransform> 
      N_arrow_transform_refptr,E_arrow_transform_refptr,
      S_arrow_transform_refptr,W_arrow_transform_refptr;

   void allocate_member_objects();
   void initialize_member_objects();

   void initialize_arrow_transforms();
   void update_arrow();

   osg::MatrixTransform* generate_arrow_geode();
   void generate_arrow_drawables();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:


#endif 
