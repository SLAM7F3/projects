// ==========================================================================
// EARTHSGROUP class member function definitions
// ==========================================================================
// Last modified on 5/22/09; 12/4/10; 12/21/10
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "osg/osgEarth/EarthsGroup.h"
#include "osg/osgWindow/MyViewerEventHandler.h"
#include "osg/osgGIS/postgis_database.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void EarthsGroup::allocate_member_objects()
{
}		       

void EarthsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="EarthsGroup";
   prev_log_eye_alt=NEGATIVEINFINITY;
   prev_camera_direction=threevector(0,0,0);
   CM_3D_ptr=NULL;

   if (flat_grid_flag)
   {
      get_OSGgroup_ptr()->setUpdateCallback( 
         new AbstractOSGCallback<EarthsGroup>(
            this, &EarthsGroup::update_altitude_dependent_masks));
   }
   else
   {
      get_OSGgroup_ptr()->setUpdateCallback( 
         new AbstractOSGCallback<EarthsGroup>(
            this, &EarthsGroup::update_display));
   }
}		       

EarthsGroup::EarthsGroup(
   Pass* PI_ptr,Clock* clock_ptr,threevector* GO_ptr,
   bool flat_grid_flag):
   GeometricalsGroup(3,PI_ptr,clock_ptr,NULL,GO_ptr)
{	
   this->flat_grid_flag=flat_grid_flag;
   initialize_member_objects();
   allocate_member_objects();
}		       

EarthsGroup::~EarthsGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const EarthsGroup& EG)
{
   outstream << "Earth = " << EG.get_Earth_ptr() << endl;
   return(outstream);
}

// ==========================================================================
// Earth creation and manipulation methods
// ==========================================================================

// Member function generate_new_Earth returns a dynamically
// instantiated Earth object.

Earth* EarthsGroup::generate_new_Earth(postgis_database* pgdb_ptr)
{
//   cout << "inside EarthsGroup::generate_new_Earth()" << endl;

   Earth_ptr=new Earth(pass_ptr,Clock_ptr,pgdb_ptr);
   GraphicalsGroup::insert_Graphical_into_list(Earth_ptr);
   insert_graphical_PAT_into_OSGsubPAT(Earth_ptr,0);

   initialize_Graphical(Earth_ptr);

   Earth_ptr->get_PAT_ptr()->addChild(Earth_ptr->generate_drawable_group());
   set_Ellipsoid_model_ptr(Earth_ptr->get_Ellipsoid_model_ptr());

   return Earth_ptr;
}

// ==========================================================================
// Lines of longitude & latitude display methods
// ==========================================================================

// Member function update_display() redraws lines of longitude and
// latitude if the camera's radial and/or angular position has changed
// appreciably from the last time these lines were drawn.

void EarthsGroup::update_display()
{   
//   cout << "inside EarthsGroup::update_display()" << endl;
   
   bool redraw_grid_lines=false;

   Earth_ptr->compute_camera_posn_and_Zhat_in_ECI_coords();
   Earth_ptr->compute_camera_to_screen_center_distance();

// Update display if camera's orientation has changed appreciably
// compared to current angular spacing between longitude and latitude
// grid lines:

   double dtheta=basic_math::min(
      0.25,0.2*Earth_ptr->get_LatLongGrid_ptr()->get_delta_longitude(),
      0.025*Earth_ptr->get_LatLongGrid_ptr()->get_delta_latitude()); // degs
   double dotproduct=cos(dtheta*PI/180);

   if (prev_camera_direction.dot(-Earth_ptr->get_camera_Zhat()) < dotproduct)
   {
      prev_camera_direction=-Earth_ptr->get_camera_Zhat();
      redraw_grid_lines=true;
   }
   
// Update display if camera's altitude has changed appreciably:

   log_eye_alt=Earth_ptr->get_Ellipsoid_model_ptr()->
      log_eye_altitude(Earth_ptr->get_camera_ECI_posn());
//   cout << "log_eye_alt = " << log_eye_alt << endl;
   update_altitude_dependent_masks();

   if (fabs(log_eye_alt - prev_log_eye_alt) > 0.003)
   {
      prev_log_eye_alt=log_eye_alt;
      redraw_grid_lines=true;
   }

// Update display if long/lat mask has recently been enabled:

   if (Earth_ptr->get_refresh_longlat_lines_flag())
   {
      redraw_grid_lines=true;
      Earth_ptr->set_refresh_longlat_lines_flag(false);
   }

   if (redraw_grid_lines && Earth_ptr->get_display_long_lat_lines())
   {
      redraw_long_lat_lines();
   } 

   GraphicalsGroup::update_display();
}

// --------------------------------------------------------------------------
// Member function update_altitude_dependent_masks determines whether
// and how display should be updated as a function of camera's
// altitude.

void EarthsGroup::update_altitude_dependent_masks()
{   
//   cout << "inside EarthsGroup::update_altitude_dependent_masks()" << endl;

// Check if a CustomAnimationPathManipulator has been instantiated.
// If so, use it rather than manually operated terrain_manipulator to
// determine camera's height above grid.  We generalized this logic in
// Dec 2010 in order to enable altitude-dependent masks to work when
// making movies...

   osgGA::CustomAnimationPathManipulator* apm_ptr=NULL;
   if (CM_3D_ptr != NULL)
   {
      WindowManager* WM_ptr=CM_3D_ptr->get_WindowManager_ptr();
//      cout << "WM_ptr = " << WM_ptr << endl;
      ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(WM_ptr);
      apm_ptr=ViewerManager_ptr->get_MyViewerEventHandler_ptr()->
         get_CustomAnimationPathManipulator_ptr();
//      cout << "apm_ptr = " << apm_ptr << endl;
   }

// For dynamically modifying Afghanistan map, use camera height
// extracted from *CM_3D_ptr:

   if (flat_grid_flag && CM_3D_ptr != NULL)
   {
      threevector camera_posn;
      if (apm_ptr==NULL)
      {
         camera_posn=CM_3D_ptr->get_eye_world_posn();
      }
      else
      {
         camera_posn=apm_ptr->get_eye_world_posn();
      }
//      cout << "camera_posn = " << camera_posn << endl;
      log_eye_alt=log10(camera_posn.get(2)-CM_3D_ptr->get_grid_origin_ptr()->
		        get(2));
//           << CM_3D_ptr->get_camera_height_above_grid() << endl;

//      cout << "log_eye_alt = " << log_eye_alt << endl;
   }

   const double max_log_eye_alt=7.7;
   const double min_log_eye_alt=0.0;
   const double min_longlat_lines_log_alt=0.0;
//   const double min_longlat_lines_log_alt=2.0;

   const double max_borders_log_alt=7.4;
   const double min_borders_log_alt=5.5;
//   const double min_borders_log_alt=4.7;
   const double max_cities_log_alt=6.4;
   const double min_cities_log_alt=5.4;

   const double max_minor_cities_log_alt=5.8;
   const double min_minor_cities_log_alt=4.8;

   if (log_eye_alt > max_minor_cities_log_alt ||
       log_eye_alt < min_minor_cities_log_alt)
   {
      Earth_ptr->set_minor_cities_mask(0);
   }
   else
   {
      if (Earth_ptr->get_display_cities())
      {
         Earth_ptr->set_minor_cities_mask(1);
      }
   }

   if (log_eye_alt > max_cities_log_alt ||
       log_eye_alt < min_cities_log_alt)
   {
      Earth_ptr->set_cities_mask(0);
      Earth_ptr->set_countries_mask(0);
      Earth_ptr->set_borders_mask(0);
   }
   else
   {
      if (Earth_ptr->get_display_cities())
      {
         Earth_ptr->set_cities_mask(1);
//         Earth_ptr->set_altitude_dependent_border_width();
      }
   }

   if (log_eye_alt > max_borders_log_alt || log_eye_alt <= max_cities_log_alt)
   {
      Earth_ptr->set_countries_mask(0);
   }
   else if (log_eye_alt > max_cities_log_alt)
   {
      if (Earth_ptr->get_display_borders())
      {
         Earth_ptr->set_countries_mask(1);
      }
   }

   if (log_eye_alt > max_borders_log_alt || log_eye_alt < min_borders_log_alt)
   {
      Earth_ptr->set_borders_mask(0);
   }
   else
   {
      if (Earth_ptr->get_display_borders())
      {
         Earth_ptr->set_borders_mask(1);
         Earth_ptr->set_altitude_dependent_border_width();
      }
   }

// Shut off lines of longitude & latitude if camera is too far away
// from earth or too close to surface:

   if (log_eye_alt > max_log_eye_alt || 
       log_eye_alt < min_longlat_lines_log_alt)
   {
      Earth_ptr->set_longlat_lines_mask(0);
      return;
   }
   else
   {
      if (Earth_ptr->get_display_long_lat_lines())
         Earth_ptr->set_longlat_lines_mask(1);
   }

// Since national boundaries layer is not very accurate, we should
// turn it off if the camera is zoomed in very close to the earth's
// surface:

   if (log_eye_alt < min_borders_log_alt)
   {
      Earth_ptr->set_countries_mask(0);
      Earth_ptr->set_borders_mask(0);
   }

   log_eye_alt=basic_math::min(log_eye_alt,max_log_eye_alt);
   log_eye_alt=basic_math::max(log_eye_alt,min_log_eye_alt);

//   cout << "At end of EarthsGroup::update_alt_dep_masks, borders_mask=" 
//        << Earth_ptr->get_borders_mask() << endl;
}

// --------------------------------------------------------------------------
// Member function redraw_long_lat_lines

void EarthsGroup::redraw_long_lat_lines()
{   
//   cout << "inside EarthsGroup::redraw_long_lat_lines()" << endl;
//   cout << "log_eye_alt = " << log_eye_alt << endl;

// Make sure that camera_ECI_posn != (0,0,0):
   
   if (Earth_ptr->get_camera_ECI_posn().magnitude() < 1) return;

   Earth_ptr->get_LatLongGrid_ptr()->redraw_long_lat_lines(
      log_eye_alt,Earth_ptr->get_Clock_ptr());
}
