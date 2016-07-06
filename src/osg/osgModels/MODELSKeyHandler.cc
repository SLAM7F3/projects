// ==========================================================================
// MODELSKeyHandler class member function definitions
// ==========================================================================
// Last modified on 9/18/09; 9/30/09; 5/10/10; 12/4/10
// ==========================================================================

#include <string>
#include <vector>
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osgModels/MODELSKeyHandler.h"

#include "osg/osgEarth/EarthRegionsGroup.h"
#include "track/movers_group.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void MODELSKeyHandler::allocate_member_objects()
{
}

void MODELSKeyHandler::initialize_member_objects()
{
   MODELSGROUP_ptr=NULL;
}

MODELSKeyHandler::MODELSKeyHandler(
   MODELSGROUP* MG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(MG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   MODELSGROUP_ptr=MG_ptr;
}

MODELSKeyHandler::~MODELSKeyHandler()
{
}

// ------------------------------------------------------
bool MODELSKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
//      cout << "inside MODELSKeyHandler::handle()" << endl;

// For Bluegrass demo, press 's' while in RUN_MOVIE mode to toggle
// on/off movable Constant Hawk airplane model with its translucent
// OBSFRUSTUM field-of-view:

      if (get_ModeController_ptr()->getState()==ModeController::RUN_MOVIE &&
          ea.getKey()=='s')
      {
         if (MODELSGROUP_ptr == NULL) return false;

         cout << "Toggling sensor view" << endl;
         for (unsigned int i=0; i<MODELSGROUP_ptr->get_n_OSGsubPATs(); i++)
         {
            MODELSGROUP_ptr->toggle_model_mask(i);
         }
      }
      else if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_MODEL)
      {

// Press "Delete" key to completely destroy a Model:

         if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
            MODELSGROUP_ptr->destroy_Graphical();
            return true;
         }

// Press ">" ["<"] key to increase [decrease] a Model's size:

         else if (ea.getKey()=='>')
         {
            MODELSGROUP_ptr->change_scale(1.5);
            return true;
         }
         else if (ea.getKey()=='<')
         {
            MODELSGROUP_ptr->change_scale(0.666);
            return true;
         }

// Press "Up arrow" or "Down arrow" to move a selected Model up or
// down in the world-z direction:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
            return (MODELSGROUP_ptr->move_z(1) != NULL);
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
            return (MODELSGROUP_ptr->move_z(-1) != NULL);
         }
         else if (ea.getKey()=='f')
         {
            MODELSGROUP_ptr->finish_waypoint_entry();
            return true;
         }
         else if (ea.getKey()=='m')
         {
            MODELSGROUP_ptr->unmask_next_model();
            return true;
         }
//         else if (ea.getKey()=='p')
//         {
//            MODELSGROUP_ptr->record_waypoint();
//            return true;
//         }
         else if (ea.getKey()=='r')
         {
            double center_longitude=69;
            double center_latitude=35;
            double orbit_radius=150*1000;   // meters
            int flightpath_sgn=1;
            MODELSGROUP_ptr->generate_circular_PolyLine_Path(
               center_longitude,center_latitude,orbit_radius,flightpath_sgn);
            return true;
         }
//         else if (ea.getKey()=='t')
//         {
//            for (int i=0; i<MODELSGROUP_ptr->get_n_OSGsubPATs(); i++)
//            {
//               MODELSGROUP_ptr->toggle_model_mask(i);
//            }
//            return true;
//         }
         else if (ea.getKey()=='x')
         {
            if (MODELSGROUP_ptr->get_Path_PolyLinesGroup_ptr() != NULL &&
                MODELSGROUP_ptr->get_Path_PolyLinePickHandler_ptr() != NULL)
            {
               MODELSGROUP_ptr->broadcast_sensor_and_target_statevectors();

// --------------------------------------------------------------------
// FAKE FAKE: Sun Aug 3 at 7 am For now, we simply fill ROI_IDs with a
// trivial ordering of ROI IDs.  This section should be commented out
// when we're connected to Luca's MATLAB codes:

               EarthRegion* EarthRegion_ptr=
                  MODELSGROUP_ptr->get_EarthRegionsGroup_ptr()->
                  get_ID_labeled_EarthRegion_ptr(0);
               movers_group* movers_group_ptr=EarthRegion_ptr->
                  get_movers_group_ptr();
               vector<int> ROI_IDs=movers_group_ptr->
                  get_particular_mover_IDs(mover::ROI);

               for (int j=0; j<int(ROI_IDs.size()); j++)
               {
                  cout << "j = " << j << " ROI_IDs[j] = " << ROI_IDs[j]
                       << endl;
               }

               int n_UAVs=MODELSGROUP_ptr->get_n_Graphicals();
               cout << "n_UAVs = " << n_UAVs << endl;
               int ROI_step=ROI_IDs.size()/n_UAVs;
               cout << "ROI_step = " << ROI_step << endl;
               vector<int> start_ROI_ID,stop_ROI_ID;

               start_ROI_ID.push_back(0);
               for (int iter=0; iter<n_UAVs-1; iter++)
               {
                  stop_ROI_ID.push_back((iter+1)*ROI_step);
                  int next_ROI_ID=stop_ROI_ID.back()+1;
                  next_ROI_ID=basic_math::min(next_ROI_ID,ROI_IDs.back());
                  start_ROI_ID.push_back(next_ROI_ID);
               }
               stop_ROI_ID.push_back(ROI_IDs.back());

               for (int i=0; i<int(start_ROI_ID.size()); i++)
               {
                  cout << "i = "<< i
                       << " start_ROI_ID = " << start_ROI_ID[i]
                       << " stop_ROI_ID = " << stop_ROI_ID[i] << endl;
               }

               vector<Messenger::Property> properties;

               for (int n=0; n < n_UAVs; n++)
               {
                  int UAV_ID=n;
//                  key="UAV_and_ROI_IDs";
                  string key="UAVID_"+stringfunc::number_to_string(UAV_ID);

//                  value=stringfunc::number_to_string(UAV_ID)+",";
                  string value;
                  for (int i=start_ROI_ID[n]; i<= stop_ROI_ID[n]; i++)
//                  for (int i=0; i<ROI_IDs.size(); i++)
                  {
//                     value += stringfunc::number_to_string(ROI_IDs[i]);
                     value += stringfunc::number_to_string(i);
                     if (i < stop_ROI_ID[n]) value += ",";
//                     if (i < ROI_IDs.size()-1) value += ",";
                  }
                  cout << "value = " << value << endl;
                  properties.push_back(Messenger::Property(key,value));
   
                  MODELSGROUP_ptr->get_Messenger_ptr()->broadcast_subpacket(
                     "START_PACKET");
                  MODELSGROUP_ptr->get_Messenger_ptr()->broadcast_subpacket(
                     "ASSIGN_TASK",properties);
                  MODELSGROUP_ptr->get_Messenger_ptr()->broadcast_subpacket(
                     "STOP_PACKET");
               } // loop over UAV IDs


// --------------------------------------------------------------------

               return true;
            }
         }
         
      } // mode = MANIPULATE_MODEL conditional
   } // key down conditional
   
   return false;
}


