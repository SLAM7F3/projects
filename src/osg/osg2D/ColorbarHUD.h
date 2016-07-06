// ==========================================================================
// Header file for ColorbarHUD class which displays side Colorbar.
// ==========================================================================
// Last modified on 5/31/09; 6/1/09; 6/17/09; 9/28/09
// ==========================================================================

#ifndef ColorbarHUD_H
#define ColorbarHUD_H

#include <string>
#include <osgSim/ScalarBar>
#include "osg/GenericHUD.h"

class ColorbarHUD : public GenericHUD
{
  public:

   ColorbarHUD(double hue_start,double hue_stop,
               double scalar_value_start,double scalar_value_stop,
               std::string title);

// Set & get member functions:

   void set_truncate_value_flag(bool flag);
   void set_nodemask(int i);
   void set_colorbar_index(unsigned int i);

   void pushback_hue_start(double hue);
   void pushback_hue_stop(double hue);
   void pushback_scalar_value_start(double scalar);
   void pushback_scalar_value_stop(double scalar);
   void pushback_title(std::string title);

  protected:

  private:

   bool truncate_value_flag;
   int numLabels,numColors,colorbar_index;
   std::vector<double> hue_start,hue_stop;
   std::vector<double> scalar_value_start,scalar_value_stop;
   std::vector<std::string> title;
   
   osg::ref_ptr<osgSim::ScalarBar> scalarbar_geode_refptr;
   osg::ref_ptr<osgSim::ColorRange> colorrange_refptr;
   osg::ref_ptr<osg::Geode> title_geode_refptr;
   osg::ref_ptr<osg::Geode> labels_geode_refptr;
   osg::ref_ptr<osg::MatrixTransform> title_transform_refptr;
   osg::ref_ptr<osg::MatrixTransform> scalarbar_transform_refptr;
   osg::ref_ptr<osg::MatrixTransform> labels_transform_refptr;

   void allocate_member_objects();
   void initialize_member_objects();

   void initialize_title_transform();
   void initialize_scalarbar_transform();
   void initialize_labels_transform();
   void reset_title();
   void reset_scalarbar();
   void reset_labels();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ColorbarHUD::set_truncate_value_flag(bool flag)
{
   truncate_value_flag=flag;
   reset_labels();
}



#endif 
