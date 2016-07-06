// ==========================================================================
// LOSMODELSKeyHandler class member function definitions
// ==========================================================================
// Last modified on 5/22/11; 10/14/11; 10/15/11
// ==========================================================================

#include <string>
#include <vector>
#include "osg/ModeController.h"
#include "osg/osgModels/LOSMODELSGROUP.h"
#include "osg/osgModels/LOSMODELSKeyHandler.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void LOSMODELSKeyHandler::allocate_member_objects()
{
}

void LOSMODELSKeyHandler::initialize_member_objects()
{
   LOSMODELSGROUP_ptr=NULL;
   PointCloudsGroup_ptr=NULL;
}

LOSMODELSKeyHandler::LOSMODELSKeyHandler(
   LOSMODELSGROUP* MG_ptr,ModeController* MC_ptr):
   MODELSKeyHandler(MG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   LOSMODELSGROUP_ptr=MG_ptr;
}

LOSMODELSKeyHandler::~LOSMODELSKeyHandler()
{
}

// ------------------------------------------------------
bool LOSMODELSKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& ga)
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_MODEL)
      {
         if (ea.getKey()=='c')
         {
            LOSMODELSGROUP_ptr->toggle_between_earth_and_aircraft_reference_frames();
            return true;
         }
         else if (ea.getKey()=='d')
         {
            LOSMODELSGROUP_ptr->display_average_LOS_results();
            return true;
         }
         else if (ea.getKey()=='h')
         {
            LOSMODELSGROUP_ptr->raytrace_moving_ground_target();
            return true;
         }
         else if (ea.getKey()=='i')
         {
            LOSMODELSGROUP_ptr->export_moving_ground_target_visibility_rays();
            return true;
         }
         else if (ea.getKey()=='j')
         {
            LOSMODELSGROUP_ptr->import_moving_ground_target_visibility_rays();
            return true;
         }
         else if (ea.getKey()=='m')
         {
            LOSMODELSGROUP_ptr->mark_current_FOV_midpoint();
            return true;
         }
         else if (ea.getKey()=='o')
         {
            LOSMODELSGROUP_ptr->generate_target_visibility_omnimap();
            return true;
         }
         else if (ea.getKey()=='p')
         {
            string visibility_filename="simulated_visibility.omnimap";
            threevector transmitter_posn;
            LOSMODELSGROUP_ptr->import_visibility_omnimap(
               visibility_filename,transmitter_posn);
            return true;
         }
         else if (ea.getKey()=='r')
         {
//            string visibility_filename="simulated_visibility.omnimap";
            string visibility_filename="smeared_simulated_visibility.omnimap";
            threevector transmitter_posn;
            twoDarray* measured_twoDarray_ptr=
               LOSMODELSGROUP_ptr->import_visibility_omnimap(
                  visibility_filename,transmitter_posn);
            LOSMODELSGROUP_ptr->fit_ground_target_position(
               transmitter_posn,measured_twoDarray_ptr);
            return true;
         }
         if (ea.getKey()=='s')
         {
            string visibility_filename="simulated_visibility.omnimap";
            threevector transmitter_posn;
            twoDarray* measured_twoDarray_ptr=
               LOSMODELSGROUP_ptr->import_visibility_omnimap(
                  visibility_filename,transmitter_posn);
            twoDarray* smeared_twoDarray_ptr=LOSMODELSGROUP_ptr->
               smear_visibility_omnimap(measured_twoDarray_ptr);
            LOSMODELSGROUP_ptr->display_omni_occlusion(
               smeared_twoDarray_ptr);
            string smeared_visibility_filename="smeared_"+visibility_filename;
            LOSMODELSGROUP_ptr->export_visibility_omnimap(
               smeared_visibility_filename,transmitter_posn,
               smeared_twoDarray_ptr);
            return true;
         }
         else if (ea.getKey()=='t')
         {
            LOSMODELSGROUP_ptr->get_ArrowsGroup_ptr()->destroy_all_Arrows();
            return true;
         }
         else if (ea.getKey()=='u')
         {
            double SAM_range=6000;  // meters = 20 Kft

            cout << "Enter SAM range in meters:" << endl;
            cin >> SAM_range;

            double threatmap_longitude_lo,threatmap_longitude_hi;
            double threatmap_latitude_lo,threatmap_latitude_hi;
            cout << "Enter threatmap longitude lo:" << endl;
            cin >> threatmap_longitude_lo;
            cout << "Enter threatmap longitude hi:" << endl;
            cin >> threatmap_longitude_hi;
            cout << "Enter threatmap latitude lo:" << endl;
            cin >> threatmap_latitude_lo;
            cout << "Enter threatmap latitude hi:" << endl;
            cin >> threatmap_latitude_hi;

            LOSMODELSGROUP_ptr->visualize_SAM_threatmap(
               SAM_range,threatmap_longitude_lo,threatmap_longitude_hi,
               threatmap_latitude_lo,threatmap_latitude_hi);

            cout << "PointCloudsGroup_ptr = " << PointCloudsGroup_ptr
                 << endl;
            
            if (PointCloudsGroup_ptr != NULL)
            {
               PointCloudsGroup_ptr->set_curr_colorbar_index(2);
               PointCloudsGroup_ptr->update_ColorbarHUD();
            }
            
            return true;
         }
         
      } // mode = MANIPULATE_MODEL conditional
   } // key down conditional
   
   return false;
}

