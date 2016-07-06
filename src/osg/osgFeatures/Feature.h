// ==========================================================================
// Header file for FEATURE class
// ==========================================================================
// Last modified on 10/21/07; 2/22/09; 1/21/13
// ==========================================================================

#ifndef FEATURE_H
#define FEATURE_H

#include <iostream>
#include <osg/Array>
#include <osgText/Text>
#include "osg/osgAnnotators/Annotator.h"
#include "osg/osgGeometry/Point.h"

class AnimationController;

class Feature:public osgGeometry::Point, public Annotator
{

  public:

// Initialization, constructor and destructor functions:

   Feature(const int p_ndims,int id,AnimationController* AC_ptr=NULL);
   virtual ~Feature();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Feature& f);

// Set & get member functions:

   virtual void set_dim_dependent_colors();
   
// Drawing methods:

   osg::Geode* generate_drawable_geode(
      int passnumber,double crosshairs_size,double crosshairs_text_size,
      bool earth_feature_flag=false);
   void set_nimages_appearance_text_posn(double text_size);


  protected:

  private:

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Feature& f);

   osgText::Text* generate_nimages_appearance_text(double text_size);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:



#endif // Feature.h



