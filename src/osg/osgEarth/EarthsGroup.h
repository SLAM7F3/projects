// ==========================================================================
// Header file for EARTHSGROUP class
// ==========================================================================
// Last modified on 3/11/09; 5/22/09; 12/21/10
// ==========================================================================

#ifndef EARTHSGROUP_H
#define EARTHSGROUP_H

#include "osg/Custom3DManipulator.h"
#include "osg/osgEarth/Earth.h"
#include "osg/osgEarth/EarthManipulator.h"
#include "osg/osgGeometry/GeometricalsGroup.h"

class Clock;
class postgis_database;
class threevector;

class EarthsGroup : public GeometricalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   EarthsGroup(Pass* PI_ptr,Clock* clock_ptr,threevector* GO_ptr=NULL,
               bool flat_grid_flag=false);
   virtual ~EarthsGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const EarthsGroup& E);

// Set & get methods:

   Earth* get_Earth_ptr() const;
   void set_CM_3D_ptr(osgGA::Custom3DManipulator* CM_ptr);

// Earth creation methods:

   Earth* generate_new_Earth(postgis_database* pgdb_ptr=NULL);

// Lines of longitude & latitude display methods:

   void update_display();
   void update_altitude_dependent_masks();
   void redraw_long_lat_lines();

  protected:

  private:

   bool flat_grid_flag;
   double prev_log_eye_alt,log_eye_alt;
   threevector prev_camera_direction;
   Earth* Earth_ptr;
   osgGA::Custom3DManipulator* CM_3D_ptr;
   

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const EarthsGroup& E);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline Earth* EarthsGroup::get_Earth_ptr() const
{
   return dynamic_cast<Earth*>(get_Graphical_ptr(0));
}

inline void EarthsGroup::set_CM_3D_ptr(
   osgGA::Custom3DManipulator* CM_ptr)
{
   CM_3D_ptr=CM_ptr;
}


#endif // EarthsGroup.h



