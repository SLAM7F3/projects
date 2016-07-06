// ==========================================================================
// Header file for POINTSGROUP class
// ==========================================================================
// Last modified on 12/17/09; 12/30/10; 1/1/11; 4/5/14
// ==========================================================================

#ifndef POINTSGROUP_H
#define POINTSGROUP_H

#include <iostream>
#include <fstream>
#include <string>
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "osg/osgGeometry/Point.h"

class AnimationController;
class Clock;
class Ellipsoid_model;
class polygon;

namespace osgGeometry
{
   
   class PointsGroup : public GeometricalsGroup
      {

        public:

// Initialization, constructor and destructor functions:

         PointsGroup(const int p_ndims,Pass* PI_ptr,
                     AnimationController* AC_ptr=NULL);
         PointsGroup(const int p_ndims,Pass* PI_ptr,threevector* GO_ptr);
         PointsGroup(const int p_ndims,Pass* PI_ptr,Clock* clock_ptr,
                     Ellipsoid_model* EM_ptr,threevector* GO_ptr=NULL);
         PointsGroup(const int p_ndims,Pass* PI_ptr,
                     AnimationController* AC_ptr,threevector* GO_ptr);
         virtual ~PointsGroup();

         friend std::ostream& operator<< 
            (std::ostream& outstream,const PointsGroup& f);

// Set & get methods:

         void set_crosshairs_size(double size);
         double get_crosshairs_size() const;
         void set_crosshairs_text_size(double size);
         double get_crosshairs_text_size() const;
         Point* get_Point_ptr(int n) const;
         Point* get_ID_labeled_Point_ptr(int ID) const;
         Point* get_selected_Point_ptr() const;

// Point creation and manipulation methods:

         Point* generate_new_Point(
            bool draw_text_flag=true,bool earth_point_flag=false,
            int ID=-1,unsigned int OSGsubPAT_number=0);
         Point* generate_new_Point(
            const threevector& V,bool draw_text_flag=true,
            bool earth_point_flag=false,int ID=-1,
            unsigned int OSGsubPAT_number=0);

         void edit_label();
         bool erase_point();
         bool unerase_point();

// Point destruction member functions:

         void destroy_all_Points();
         int destroy_Point();
         bool destroy_Point(int Point_ID);


         double change_size(double factor);
         void move_z(int sgn);

         void refresh_crosshair_coords();

         void update_display();

// Region generation methods:

         polygon* generate_convexhull_poly(
            double U_to_pu_factor,double V_to_pv_factor);
         polygon* generate_poly();

// Ascii point file I/O methods:

         void save_point_info_to_file();
         void read_point_info_from_file(bool propagate_points_flag=false);
         int minimum_point_ID();
         int maximum_point_ID();

        protected:

        private:

         double crosshairs_size[4];
         double crosshairs_text_size[4];

         void allocate_member_objects();
         void initialize_member_objects();
         void docopy(const PointsGroup& f);

         void initialize_new_Point(
            const threevector& V,Point* Point_ptr,
            unsigned int OSGsubPAT_number=0);
         osg::Geode* generate_point_geode(
            Point* point_ptr,bool draw_text_flag=true,
            bool earth_point_flag=false);
      };

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

   inline void PointsGroup::set_crosshairs_size(double size)
      {
         crosshairs_size[get_ndims()]=size;
      }

   inline double PointsGroup::get_crosshairs_size() const
      {
         return crosshairs_size[get_ndims()];
      }

   inline void PointsGroup::set_crosshairs_text_size(double size)
      {
         crosshairs_text_size[get_ndims()]=size;
      }

   inline double PointsGroup::get_crosshairs_text_size() const
      {
         return crosshairs_text_size[get_ndims()];
      }

// --------------------------------------------------------------------------
   inline Point* PointsGroup::get_Point_ptr(int n) const
      {
         return dynamic_cast<Point*>(get_Graphical_ptr(n));
      }

// --------------------------------------------------------------------------
   inline Point* PointsGroup::get_ID_labeled_Point_ptr(int ID) const
      {
         return dynamic_cast<Point*>(get_ID_labeled_Graphical_ptr(ID));
      }

   inline Point* PointsGroup::get_selected_Point_ptr() const
      {
         return dynamic_cast<Point*>(get_selected_Graphical_ptr());
      }

} // osgGeometry namespace


#endif // PointsGroup.h



