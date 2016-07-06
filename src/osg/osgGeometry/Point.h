// ==========================================================================
// Header file for POINT class
// ==========================================================================
// Last modified on 10/21/07; 2/2/09; 11/6/11
// ==========================================================================

#ifndef OSGPOINT_H
#define OSGPOINT_H

#include <iostream>
#include "osg/osgGeometry/Geometrical.h"

class AnimationController;

namespace osgGeometry
{
   
   class Point:public Geometrical
      {

        public:

// Initialization, constructor and destructor functions:

         Point(const int p_ndims,int id,AnimationController* AC_ptr=NULL);
         virtual ~Point();
         friend std::ostream& operator<< 
            (std::ostream& outstream,const Point& p);

// Set & get member functions:

         virtual void set_dim_dependent_colors();

// Drawing methods:

         osg::Geode* generate_drawable_geode(
            int passnumber,double crosshairs_size,double crosshairs_text_size,
            bool draw_text_flag=true,bool earth_feature_flag=false);

         void set_crosshairs_coords(double crosshairs_size);
         virtual void set_color(const osg::Vec4& color);
         void set_crosshairs_color(const osg::Vec4& color);
         void set_crosshairsnumber_text_posn(double text_size);
         void reset_text_label();
         void reset_text_label(std::string label);

// Point manipulation methods

         void set_posn(double curr_t,int pass_number,const threevector& V);

        protected:

         threevector Uhat,Vhat,What;

        private:

         void allocate_member_objects();
         void initialize_member_objects();
         void docopy(const Point& P);

         osg::Geometry* generate_drawable_geom(double crosshairs_size);
         osgText::Text* generate_crosshairsnumber_text(double text_size);
      };

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

} // osgGeometry namespace


#endif // OSGPoint.h



