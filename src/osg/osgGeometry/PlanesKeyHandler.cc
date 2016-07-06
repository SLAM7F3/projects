// ==========================================================================
// PlanesKeyHandler class member function definitions
// ==========================================================================
// Last modified on 3/14/06; 10/30/06; 12/26/06
// ==========================================================================

#include "osg/osgGeometry/PlanesKeyHandler.h"

using std::cin;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PlanesKeyHandler::allocate_member_objects()
{
}

void PlanesKeyHandler::initialize_member_objects()
{
   ModeController_ptr=NULL;
   PlanesGroup_ptr=NULL;
}

PlanesKeyHandler::PlanesKeyHandler(PlanesGroup* PG_ptr,ModeController* MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   PlanesGroup_ptr=PG_ptr;
   ModeController_ptr=MC_ptr;
}

PlanesKeyHandler::~PlanesKeyHandler()
{
}

// ------------------------------------------------------
bool PlanesKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (ModeController_ptr->getState()==ModeController::MANIPULATE_PLANE)
      {
         if (ea.getKey()=='p')
         {
            PlanesGroup_ptr->generate_plane_from_features();
            return true;
         }
         else if (ea.getKey()=='f')
         {
            int ID;
            cout << "Enter plane ID:" << endl;
            cin >> ID;
            PlanesGroup_ptr->flip_planar_parity(ID);
            return true;
         }
/*
         else if (ea.getKey()=='r')
         {
            int ID;
            cout << "Enter plane ID:" << endl;
            cin >> ID;
            PlanesGroup_ptr->replace_points_near_feature_rectangle(ID);
//            PlanesGroup_ptr->replace_points_near_plane(ID);
            return true;
         }
*/
/*
         else if (ea.getKey()=='d')
         {
            int ID;
            cout << "Enter plane ID:" << endl;
            cin >> ID;
            PlanesGroup_ptr->points_near_feature_rectangle(ID);
//            PlanesGroup_ptr->points_near_plane(ID);
            return true;
         }
*/

      } // Mode conditional
   } // key down conditional
   
   return false;
}


