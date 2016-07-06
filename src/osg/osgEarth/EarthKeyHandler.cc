// ==========================================================================
// EarthKeyHandler class member function definitions
// ==========================================================================
// Last modified on 2/16/07; 3/11/07; 8/15/07
// ==========================================================================

#include <string>
#include "osg/osgEarth/Earth.h"
#include "osg/osgEarth/EarthKeyHandler.h"
#include "osg/osgEarth/EarthManipulator.h"
#include "astro_geo/geopoint.h"
#include "general/inputfuncs.h"
#include "osg/ModeController.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void EarthKeyHandler::allocate_member_objects()
{
}

void EarthKeyHandler::initialize_member_objects()
{
   Earth_ptr=NULL;
   EarthManipulator_ptr=NULL;
}

EarthKeyHandler::EarthKeyHandler(
   Earth* E_ptr,osgGA::EarthManipulator* EM_ptr,
   ModeController* MC_ptr):
   GraphicalsKeyHandler(MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   Earth_ptr=E_ptr;
   EarthManipulator_ptr=EM_ptr;
}

EarthKeyHandler::~EarthKeyHandler()
{
}

// ------------------------------------------------------
bool EarthKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_EARTH)
      {

         switch ( ea.getKey() )
         {
//            case 'd' :
//               Earth_ptr->toggle_long_lat_lines_depth_buffering();
//               return true;
            case 'l' :
               Earth_ptr->toggle_long_lat_lines();
               return true;
            case 'k' :
               Earth_ptr->toggle_borders_display();
               return true;
            case 'j' :
               Earth_ptr->toggle_cities_display();
               return true;
            case 'f' :
               string query="Enter destination name:";
               string destination_name=inputfunc::enter_string(query);
//               string destination_name="Boston";
               cout << "destination name = " << destination_name << endl;
               geopoint destination;

               if (Earth_ptr->long_lat_for_specified_geosite(
                  destination_name,destination))
               {
                  bool write_to_file_flag=true;
                  EarthManipulator_ptr->flyto(
                     destination.get_longitude(),
                     destination.get_latitude(),
                     destination.get_altitude(),write_to_file_flag);
                  return true;
               }
               else
               {
                  cout << "Sorry: Destination has unknown geolocation"
                       << endl;
               }
               

         }
      } // mode = MANIPULATE_EARTH conditional         
   } // key down conditional
   
   return false;
}


